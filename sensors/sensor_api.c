#include "sensor_api.h"

// Current sensor constants
#define ADC_SEQB 1
#define ADC_SEQC 2
#define ADC_PRI 0
#define ADC_STEP 0
#define ADC_V_REF 3.3 // Page 1862 says ADC ref voltage 3.3V
#define ADC_RESISTANCE 0.007 // FAQ
#define ADC_GAIN 10.0 // FAQ
#define ADC_RESOLUTION 4095.0 // Page 1861 says resolution 12 bits

#define TMP_RESOLUTION 0.015625
#define CURR_CHECK_TICKS 250

// Data window size constants
#define windowLight 5
#define windowTemp 3
#define windowCurrent 5
#define windowAcceleration 5

// Data collectors (before filtering)
float lightBuffer[windowLight];
float boardTempBuffer[windowTemp];
float motorTempBuffer[windowTemp];
float currentBuffer[windowCurrent];
float powerBuffer[windowCurrent];
uint8_t accelerationBuffer[windowAcceleration];

// Current thresholds that trigger eStop
uint16_t glThresholdTemp = 30;
float glThresholdCurrent = 1.0;
uint16_t countCurrentTicks = 0;

// Setup handles
UART_Handle uartTemp;
char motorTempAddr;
char boardTempAddr;
Char taskLightStack[512];
Char taskTempStack[512];
Semaphore_Struct semLightStruct;
Semaphore_Handle semLightHandle;
Semaphore_Struct semTempStruct;
Semaphore_Handle semTempHandle;

// Recurring SWI stuff
Clock_Params clkSensorParams;
Clock_Struct clockLightStruct;
Clock_Struct clockTempStruct;
Clock_Struct clockCurrentStruct;
Clock_Struct clockAccelerationStruct;

///////////**************??????????????
// Implementations of SWI functions  //
///////////**************??????????????

// Read and filter light over I2C
void swiLight() {
    // On sensor booster pack
    Semaphore_post(semLightHandle);
}

void taskLight() {
    while(1) {
        Semaphore_pend(semLightHandle, BIOS_WAIT_FOREVER);

        // Variables for the ring buffer (not quite a ring buffer though)
        static uint8_t light_head = 0;
        uint16_t rawData = 0;
        float converted;

        if (sensorOpt3001Read(&rawData)) {
            sensorOpt3001Convert(rawData, &converted);

            lightBuffer[light_head++] = converted;
            light_head %= windowLight;

            //System_printf("Lux: %f\n", converted);
            //System_flush();
        }
    }
}

// Read and filter board and motor temperature (TMP107) sensors daisy chained over UART
void swiTemp() {
    // On sensor booster pack
    Semaphore_post(semTempHandle);
}

void taskTemp() {
    // Build temperature read command packets
    char tx_size = 3;
    char rx_size = 4;
    char tx[3];
    char rx[4]; // Both TMP107s reply with two packets

    // Setup transmission to make both TMP107s reply with temp
    tx[0] = 0x55; // Calibration Byte
    // The daisy chain returns data starting from the address specified in the command or
    // address phase, and ending with the address of the first device in the daisy chain.
    tx[1] = TMP107_Global_bit | TMP107_Read_bit | motorTempAddr;
    tx[2] = TMP107_Temp_reg;

    while(1) {
        Semaphore_pend(semTempHandle, BIOS_WAIT_FOREVER);

        // Transmit global temperature read command
        TMP107_Transmit(uartTemp, tx, tx_size);
        // Master cannot transmit again until after we've received
        // the echo of our transmit and given the TMP107 adequate
        // time to reply. thus, we wait.
        // Copy the response from TMP107 into user variable
        TMP107_WaitForEcho(uartTemp, tx_size, rx, rx_size);

        // Convert two bytes received from TMP107 into degrees C
        float boardTemp = TMP107_DecodeTemperatureResult(rx[3], rx[2]);
        float motorTemp = TMP107_DecodeTemperatureResult(rx[1], rx[0]);

        // Variables for the ring buffer (not quite a ring buffer though)
        static uint8_t tempHead = 0;
        boardTempBuffer[tempHead] = boardTemp;
        motorTempBuffer[tempHead++] = motorTemp;
        tempHead %= windowTemp;

        // Check if temperature threshold exceeded
        if (motorTemp > glThresholdTemp) {
            eStopMotor();
            eStopGUI();
        }

        //System_printf("BTemp: %f\n", boardTemp);
        //System_printf("MTemp: %f\n", motorTemp);
        //System_flush();
    }
}

// Read and filter two motor phase currents via analogue signals
void swiCurrent() {
    uint32_t ADC1ValueB[1], ADC1ValueC[1];
    float V_OutB = 0;
    float V_OutC = 0;

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQB);
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQC);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC1_BASE, ADC_SEQB, false));
    while(!ADCIntStatus(ADC1_BASE, ADC_SEQC, false));

    // Clear ADC Interrupt
    ADCIntClear(ADC1_BASE, ADC_SEQB);
    ADCIntClear(ADC1_BASE, ADC_SEQC);

    // Read ADC Value.
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQB, ADC1ValueB);
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQC, ADC1ValueC);

    // CONVERT TO DIGITAL THAN CURRENT
    // Analogue voltage = ADC reading * system voltage / ADC resolution
    // Current = (Vref/2 - Vsox) / Gcsa x Rsense

    // Convert digital value
    V_OutB = ((float)ADC1ValueB[0] * ADC_V_REF) / ADC_RESOLUTION;
    float currentSensorB = (ADC_V_REF/2.0 - V_OutB) / (ADC_GAIN * ADC_RESISTANCE);

    // Convert digital value
    V_OutC = ((float)ADC1ValueC[0] * ADC_V_REF) / ADC_RESOLUTION;
    float currentSensorC = (ADC_V_REF/2.0 - V_OutC) / (ADC_GAIN * ADC_RESISTANCE);

    // Variables for the ring buffer (not quite a ring buffer though)
    static uint8_t currentHead = 0;
    float current = (currentSensorB + currentSensorC) * 3 / 2;
    currentBuffer[currentHead++] = current;
    currentHead %= windowCurrent;

    // Save power for graphing
    static uint8_t powerHead = 0;
    // P = V * I     using avg of the voltages
    float power = current * ((V_OutB + V_OutC) / 2);
    powerBuffer[powerHead++] = power;
    powerHead %= windowCurrent; // windowCurrent deliberate

    // Check if filtered current exceeds threshold
    // Don't check every cycle, too expensive
    countCurrentTicks++;
    if (countCurrentTicks > 250) {
        countCurrentTicks = 0;

        // Check if current limit exceeded
        float sum = 0;
        uint8_t i;
        for (i = 0; i < windowCurrent; i++) {
            sum += currentBuffer[i];
        }
        float avgCurrent = sum / windowCurrent;

        //System_printf("C: %f\n", avgCurrent);
        //System_printf("V: %f %f\n", V_OutB, V_OutC);
        //System_flush();

        if (avgCurrent > glThresholdCurrent) {
            eStopMotor();
            eStopGUI();
        }
    }
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// NOTE: THIS IS AC CELERATION OF THE BOARD, NOT THE MOTOR
void swiAcceleration() {
    // BMI160 Inertial Measurement Sensor
    // TODO Get acceleration readings on three axes
    // ABS is calculated from those three in a getter
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool initLight() {
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    lighti2c = I2C_open(Board_I2C2, &i2cParams);
    if (!lighti2c) {
        System_printf("Light I2C did not open\n");
    }
    bool worked = sensorOpt3001Test();
    sensorOpt3001Enable(true);

    // Create task that reads light sensor
    // This retarded elaborate: swi - sem - task setup
    // is needed cause the read function doesn't work in a swi
    // yet still need the stupid recurringness a clock swi gives.
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semLightStruct, 0, &semParams);
    semLightHandle = Semaphore_handle(&semLightStruct);

    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = 512;
    taskParams.priority = 11;
    taskParams.stack = &taskLightStack;
    Task_Handle lightTask = Task_create((Task_FuncPtr)taskLight, &taskParams, NULL);
    if (lightTask == NULL) {
        System_printf("Task - LIGHT FAILED SETUP");
    }

    // Create a recurring 2Hz SWI swi_light to post semaphore
    clkSensorParams.period = 500;
    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swiLight, 1, &clkSensorParams);

    System_printf("Light setup\n");
    System_flush();
    return true;
}

// TEMPERATURE SETUP
bool initTemp(uint16_t thresholdTemp) {
    // Initialise UART
    uartTemp = TMP107_InitUart();

    // Initialise daisy chain of two TMP107s
    boardTempAddr = TMP107_Init(uartTemp);
    motorTempAddr = TMP107_LastDevicePoll(uartTemp); // motor_addr var will be a backwards 5 bit
    int device_count = TMP107_Decode5bitAddress(motorTempAddr);

    // Tell TMP107s to update in 500ms intervals
    TMP107_Set_Config(uartTemp, boardTempAddr);
    TMP107_Set_Config(uartTemp, motorTempAddr);

    // Create task that reads temp sensor
    // This retarded elaborate: swi - sem - task setup
    // is needed cause the read function doesn't work in a swi
    // yet still need the stupid recurringness a clock swi gives.
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semTempStruct, 0, &semParams);
    semTempHandle = Semaphore_handle(&semTempStruct);

    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = 512;
    taskParams.priority = 11;
    taskParams.stack = &taskTempStack;
    Task_Handle tempTask = Task_create((Task_FuncPtr)taskTemp, &taskParams, NULL);
    if (tempTask == NULL) {
        System_printf("Task - TEMP FAILED SETUP");
        System_flush();
        return false;
    }

    // Create a recurring 2Hz SWI swi_temp
    clkSensorParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiTemp, 1, &clkSensorParams);

    System_printf("Temperature setup\n");
    System_flush();
    return true;
}

// CURRENT SETUP
bool initCurrent(uint16_t thresholdCurrent) {
    setThresholdCurrent(thresholdCurrent);

    // Current sensors B and C on ADC, A is not and thus not done.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Current sensor B on D7 with ADC channel 4
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQB, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQB, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQB);
    ADCIntClear(ADC1_BASE, ADC_SEQB);

    // Current sensor C on E3 with ADC channel 0
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQC, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQC, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH0 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQC);
    ADCIntClear(ADC1_BASE, ADC_SEQC);

    clkSensorParams.period = 4;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swiCurrent, 1, &clkSensorParams);

    System_printf("Current setup\n");
    System_flush();

    return true;
}

// ACCELERATION SETUP
// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool initAcceleration(uint16_t thresholdAccel) {
    setThresholdAccel(thresholdAccel);

    // Create a recurring 200Hz SWI swi_acceleration
    clkSensorParams.period = 5;
    Clock_construct(&clockAccelerationStruct, (Clock_FuncPtr)swiAcceleration, 1, &clkSensorParams);

    // TODO Setup BMI160 Inertial Measurement Sensor. Probably I2C?

    // TODO Setup callback_accelerometer to trigger if acceleration above threshold. Like our light lab task interrupt.

    System_printf("Acceleration setup\n");
    System_flush();

    return true;
}

///////////**************??????????????
//              Getters              //
///////////**************??????????????

float getLight() {
    // Lux must be converted from the raw values as uint16_t is cheaper to store
    float sum = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowLight; i++) {
        sum += lightBuffer[i];
    }

    return (sum / (float)windowLight);
}

float getBoardTemp() {
    float sum = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowTemp; i++) {
        sum += boardTempBuffer[i];
    }
    return (sum / (float)windowTemp);}

float getMotorTemp() {
    float sum = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowTemp; i++) {
        sum += motorTempBuffer[i];
    }
    return (sum / (float)windowTemp);
}

float getCurrent() {
    float sum = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowCurrent; i++) {
        sum += currentBuffer[i];
    }
    return (sum / (float)windowCurrent);
}

float getPower() {
    float sum = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowCurrent; i++) {
        sum += powerBuffer[i];
    }
    return (sum / (float)windowCurrent);
}

float getAcceleration() {
    return 1;
}

void setThresholdTemp(uint16_t thresholdTemp) {
    glThresholdTemp = thresholdTemp;
}

void setThresholdCurrent(uint16_t thresholdCurrent) {
    glThresholdCurrent = (float)thresholdCurrent / 1000;
}

void setThresholdAccel(uint16_t thresholdAccel) {
    // TODO Update BMI160 with new limit...
}

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool initSensors(uint16_t thresholdTemp, uint16_t thresholdCurrent, uint16_t thresholdAccel) {
    // Used by separate init functions to create recurring SWIs. Period size is 1ms.
    Clock_Params_init(&clkSensorParams);
    clkSensorParams.startFlag = TRUE;

    return
            initLight() &&
            initTemp(thresholdTemp) &&
            initCurrent(thresholdCurrent) &&
            initAcceleration(thresholdAccel);
}
