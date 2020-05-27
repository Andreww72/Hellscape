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
uint16_t lightBuffer[windowLight];
float boardTempBuffer[windowTemp];
float motorTempBuffer[windowTemp];
float currentBuffer[windowCurrent];
uint8_t accelerationBuffer[windowAcceleration];

// Current thresholds that trigger eStop
uint16_t glThresholdCurrent = 1;
uint16_t countCurrentTicks = 0;

// Setup handles
UART_Handle uartMotor;
char motorAddr;
Char taskLightStack[512];
Char taskMotorTempStack[512];
Semaphore_Struct semLightStruct;
Semaphore_Handle semLightHandle;
Semaphore_Struct semMotorTempStruct;
Semaphore_Handle semMotorTempHandle;

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
void swiLight(UArg arg) {
    // On sensor booster pack
    Semaphore_post(semLightHandle);
}

void taskLight(UArg i2c_handle) {
    while(1) {
        Semaphore_pend(semLightHandle, BIOS_WAIT_FOREVER);

        // Variables for the ring buffer (not quite a ring buffer though)
        static uint8_t light_head = 0;
        uint16_t rawData = 0;

        if (sensorOpt3001Read((I2C_Handle)i2c_handle, &rawData)) {
            lightBuffer[light_head++] = rawData;
            light_head %= windowLight;

            //System_printf("Lux: %d\n", rawData);
            //System_flush();
        }
    }
}

// Read and filter motor temperature (TMP007) sensor over UART
void swiBoardTemp(UArg arg) {
    // TODO read board temperature via UART
    Semaphore_post(semMotorTempHandle);
}

void taskBoardTemp(UArg motor_addr) {
    // TODO read board temperature via UART
    // Probably copy taskMotorTemp
}

// Read and filter motor temperature (TMP107) sensor over UART
void swiMotorTemp(UArg arg) {
    // On sensor booster pack
    Semaphore_post(semMotorTempHandle);
}

void taskMotorTemp() {
    // Build temperature read command packet
    char tx_size = 3;
    char rx_size = 2;
    char tx[3];
    char rx[2];

    //tx[0] = 0b10000010; // Constructed 0b01000001 is flipped
    tx[0] = 0x55; // Calibration Byte
    tx[1] = TMP107_Read_bit | motorAddr;
    tx[2] = TMP107_Temp_reg;

    while(1) {
        Semaphore_pend(semMotorTempHandle, BIOS_WAIT_FOREVER);

        // Transmit global temperature read command
        TMP107_Transmit(uartMotor, tx, tx_size);
        // Master cannot transmit again until after we've received
        // the echo of our transmit and given the TMP107 adequate
        // time to reply. thus, we wait.
        // Copy the response from TMP107 into user variable
        TMP107_WaitForEcho(uartMotor, tx_size, rx, rx_size);

        // Convert two bytes received from TMP107 into degrees C
        float tmp107_temp = TMP107_DecodeTemperatureResult(rx[1], rx[0]);

        // Variables for the ring buffer (not quite a ring buffer though)
        static uint8_t motorTempHead = 0;
        motorTempBuffer[motorTempHead++] = tmp107_temp;
        motorTempHead %= windowTemp;

        System_printf("Temp: %f\n", tmp107_temp);
        System_flush();
    }
}

// Read and filter two motor phase currents via analogue signals
void swiCurrent(UArg arg) {
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
    float current = currentSensorB + currentSensorC;
    currentBuffer[currentHead++] = current;
    currentHead %= windowCurrent;

    // Check once a second if current limit exceeded
    countCurrentTicks++;
    if (countCurrentTicks >= CURR_CHECK_TICKS) {
        countCurrentTicks = 0;

        if (current > glThresholdCurrent) {
            eStopMotor();
        }
    }
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// NOTE: THIS IS AC CELERATION OF THE BOARD, NOT THE MOTOR
void swiAcceleration(UArg arg) {
    // BMI160 Inertial Measurement Sensor
    // TODO Get acceleration readings on three axes
    // ABS is calculated from those three in a getter
}

// Interrupt when temp, or acceleration threshold reached
void callbackTriggerTempEStop(unsigned int index) {
    eStopMotor();
    TMP107_AlertOverClear(uartMotor);
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool initLight() {
    // Set up I2C
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    I2C_Handle lighti2c = I2C_open(Board_I2C2, &i2cParams);
    if (!lighti2c) {
        System_printf("Light I2C did not open\n");
        System_flush();
    }

    Task_sleep(10);
    bool status = sensorOpt3001Test(lighti2c);
    while (!status) {
        System_printf("OPT3001 test failed, retrying\n");
        System_flush();
        status = sensorOpt3001Test(lighti2c);
    }
    sensorOpt3001Init(lighti2c);
    sensorOpt3001Enable(lighti2c, true);

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
    taskParams.arg0 = lighti2c; // You can indeed assign this type, just cast it back in the task
    Task_Handle lightTask = Task_create((Task_FuncPtr)taskLight, &taskParams, NULL);
    if (lightTask == NULL) {
        System_printf("Task - LIGHT FAILED SETUP");
        System_flush();
        status = 0;
    }

    // Create a recurring 2Hz SWI swi_light to post semaphore
    clkSensorParams.period = 500;
    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swiLight, 1, &clkSensorParams);

    if (status) {
        System_printf("Light setup\n");
        System_flush();
        return true;
    } else {
        return false;
    }
}

// TEMPERATURE SETUP
bool initBoardTemp() {
    // Create a recurring 2Hz SWI swi_temp
    //clkSensorParams.period = 500;
    //Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiBoardTemp, 1, &clkSensorParams);

    // TODO Setup UART connection for board TMP107
    // TODO Setup board TMP107 temperature sensor

    return true;
}

bool initMotorTemp(uint16_t thresholdTemp) {
    // Initialise UART
    uartMotor = initMotorUart();
    TMP107_Init(uartMotor);
    motorAddr = TMP107_LastDevicePoll(uartMotor); // motor_addr var will be a backwards 5 bit
    int device_count = TMP107_Decode5bitAddress(motorAddr);

    TMP107_Set_Config(uartMotor, motorAddr);

    // Setup TMP107 interrupt on Q1 for crossing user defined threshold
    GPIO_setConfig(Board_TMP107_INT, GPIO_CFG_INPUT | GPIO_FALLING_EDGE);
    GPIO_setCallback(Board_TMP107_INT, callbackTriggerTempEStop);
    GPIO_enableInt(Board_TMP107_INT);
    setThresholdTemp(thresholdTemp);

    // Create task that reads temp sensor
    // This retarded elaborate: swi - sem - task setup
    // is needed cause the read function doesn't work in a swi
    // yet still need the stupid recurringness a clock swi gives.
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&semMotorTempStruct, 0, &semParams);
    semMotorTempHandle = Semaphore_handle(&semMotorTempStruct);

    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = 512;
    taskParams.priority = 11;
    taskParams.stack = &taskMotorTempStack;
    Task_Handle motorTempTask = Task_create((Task_FuncPtr)taskMotorTemp, &taskParams, NULL);
    if (motorTempTask == NULL) {
        System_printf("Task - MOTOR TEMP FAILED SETUP");
        System_flush();
        return false;
    }

    // Create a recurring 2Hz SWI swi_temp
    clkSensorParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiMotorTemp, 1, &clkSensorParams);

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
    for (i = 0; i < windowLight; i++){
        float converted;
        sensorOpt3001Convert(lightBuffer[i], &converted);
        sum += converted;
    }

    return (sum / (float)windowLight);
}

float getBoardTemp() {
    return 25;
}

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
    return ((sum / (float)windowCurrent) * 3.0 / 2.0);
}

uint8_t getAcceleration() {
    return 1;
}

void setThresholdTemp(uint8_t thresholdTemp) {
    // Update TMP107 with user limit
    char tx_size = 5;
    char rx_size = 2;
    char tx[5];
    char rx[2];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = motorAddr;
    tx[2] = TMP107_High_Limit_reg;

    uint16_t encoded = (uint16_t) ((float)thresholdTemp / 0.015625);
    uint16_t reversed = reverseBits(encoded);
    char high_byte = ((uint16_t)reversed >> 8) & 0xFF;
    char low_byte = ((uint16_t)reversed >> 0) & 0xFF;  // shift by 0 not needed, of course, just stylistic
    tx[3] = high_byte; // Set threshold
    tx[4] = low_byte; // Set threshold

    // Transmit global temperature read command
    TMP107_Transmit((UART_Handle)uartMotor, tx, tx_size);
    // Master cannot transmit again until after we've received
    // the echo of our transmit and given the TMP107 adequate
    // time to reply. thus, we wait.
    // Copy the response from TMP107 into user variable
    TMP107_WaitForEcho((UART_Handle)uartMotor, tx_size, rx, rx_size);
}

void setThresholdCurrent(uint16_t thresholdCurrent) {
    glThresholdCurrent = thresholdCurrent;
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

    //initLight();
    //initBoardTemp();
    initMotorTemp(thresholdTemp);
    initCurrent(thresholdCurrent);
    //initAcceleration(thresholdAccel);
    return 1;
}
