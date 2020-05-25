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
uint8_t boardTempBuffer[windowTemp];
uint8_t motorTempBuffer[windowTemp];
float currentSensorBBuffer[windowCurrent];
float currentSensorCBuffer[windowCurrent];
uint8_t accelerationBuffer[windowAcceleration];

// Current values (after filtering)
uint8_t light = 100;
uint8_t boardTemp = 25;
uint8_t motorTemp = 25;
float currentSensorB = 0;
float currentSensorC = 0;
uint8_t boardAcceleration = 0;

// Thresholds that trigger eStop
uint8_t thresholdTemp = 40;
uint16_t thresholdCurrent = 1000;
uint16_t countCurrentTicks = 0;
uint16_t thresholdAccel = 1;

// Setup handles
I2C_Handle lighti2c;
Char taskLightStack[512];
Semaphore_Struct semLightStruct;
Semaphore_Handle semLightHandle;
char motor_tmp107_addr;
Clock_Params clkParams;
Clock_Struct clockLightStruct;
Clock_Struct clockTempStruct;
Clock_Struct clockCurrentStruct;
Clock_Struct clockAccelerationStruct;

// Function prototypes that are not in the .h (deliberately)
bool initLight();
bool initBoardTemp();
bool initMotorTemp(uint8_t threshTemp);
bool initCurrent(uint16_t threshCurrent);
bool initAcceleration(uint16_t threshAccel);
void swiLight(UArg arg);
void taskLight(UArg arg);
void swiBoardTemp(UArg arg);
void swiMotorTemp(UArg arg);
void swiCurrent(UArg arg);
void swiAcceleration(UArg arg);

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool initSensors(uint8_t threshTemp, uint16_t threshCurrent, uint16_t threshAccel) {

    // Used by separate init functions to create recurring SWIs. Period size is 1ms.
    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;

    initLight();
    //initBoardTemp();
    //initMotorTemp(threshTemp);
    //initCurrent(threshCurrent);
    //initAcceleration(threshAccel);
    return 1;
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool initLight() {
    // Set up I2C
    I2C_Params i2cParams;
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    I2C_Handle lighti2c2 = I2C_open(Board_I2C2, &i2cParams);
    if (!lighti2c2) {
        System_printf("Light I2C did not open\n");
        System_flush();
    }

    bool status = sensorOpt3001Test(lighti2c2);
    while (!status) {
        System_printf("OPT3001 test failed, retrying\n");
        System_flush();
        status = sensorOpt3001Test(lighti2c);
    }
    sensorOpt3001Init(lighti2c2);
    sensorOpt3001Enable(lighti2c2, true);

    // Create task that reads light sensor
    // This retarded elaborate: swi - sem - task setup
    // is needed cause the read function doesn't work in a swi
    // yet still need the stupid recurringness a clock swi gives.
//    Semaphore_Params semParams;
//    Semaphore_Params_init(&semParams);
//    semParams.mode = Semaphore_Mode_BINARY;
//    Semaphore_construct(&semLightStruct, 0, &semParams);
//    semLightHandle = Semaphore_handle(&semLightStruct);
//
//    Task_Params taskParams;
//    Task_Params_init(&taskParams);
//    taskParams.stackSize = 512;
//    taskParams.priority = 10;
//    taskParams.stack = &taskLightStack;
//    Task_Handle lightTask = Task_create((Task_FuncPtr)taskLight, &taskParams, NULL);
//    if (lightTask == NULL) {
//        System_printf("Task - LIGHT FAILED SETUP");
//        System_flush();
//        status = 0;
//    }

    // Create a recurring 2Hz SWI swi_light to post task read event
//    clkParams.period = 500;
//    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swiLight, 1, &clkParams);

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
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiBoardTemp, 1, &clkParams);

    // TODO Setup UART connection for board TMP107
    // TODO Setup board TMP107 temperature sensor

    return true;
}

bool initMotorTemp(uint8_t threshTemp) {
    // Initialise
    init_motor_uart();
    char test = TMP107_Init();
    motor_tmp107_addr = TMP107_LastDevicePoll();
    int device_count = TMP107_Decode5bitAddress(motor_tmp107_addr);

    // TODO Setup interrupt for crossing threshold
    setThresholdTemp(threshTemp);

    // Create a recurring 2Hz SWI swi_temp
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiMotorTemp, 1, &clkParams);

    System_printf("Temperature setup on device: \n", device_count);
    System_flush();
    return true;
}

// CURRENT SETUP
bool initCurrent(uint16_t thresholdCurrent) {
    setThresholdCurrent(thresholdCurrent);

    // Current sensors B and C on ADCs, A is not and thus not done.
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

    clkParams.period = 4;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swiCurrent, 1, &clkParams);

    System_printf("Current setup\n", currentSensorC);
    System_flush();

    return true;
}

// ACCELERATION SETUP
// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool initAcceleration(uint16_t thresholdAccel) {
    setThresholdAccel(thresholdAccel);

    // Create a recurring 200Hz SWI swi_acceleration
    clkParams.period = 5;
    Clock_construct(&clockAccelerationStruct, (Clock_FuncPtr)swiAcceleration, 1, &clkParams);

    // TODO Setup BMI160 Inertial Measurement Sensor. Probably I2C?

    // TODO Setup callback_accelerometer to trigger if acceleration above threshold. Like our light lab task interrupt.

    System_printf("Acceleration setup\n");
    System_flush();

    return true;
}

///////////**************??????????????
// Implementations of SWI functions  //
///////////**************??????????????

// Read and filter light over I2C
void swiLight(UArg arg) {
    // On sensor booster pack
    Semaphore_post(semLightHandle);
}

void taskLight(UArg arg) {
    // Variables for the ring buffer (not quite a ring buffer though)
    System_printf("Lux test\n");
    System_flush();
    while(1) {
        Semaphore_pend(semLightHandle, BIOS_WAIT_FOREVER);

        static uint8_t light_head = 0;
        uint16_t rawData = 0;

        if (sensorOpt3001Read(lighti2c, &rawData)) {
            lightBuffer[light_head++] = rawData;
            light_head %= windowLight;

            float tester = 0;
            sensorOpt3001Convert(rawData, &tester);

            System_printf("Lux: %5.2f\n", tester);
            System_flush();
        }
    }
}

// Read and filter motor temperature sensors over UART
void swiBoardTemp(UArg arg) {
    // TODO read board temperature via UART
    // Probably copy swiMotorTemp
    boardTemp = 25;
}

// Read and filter motor temperature sensors over UART
void swiMotorTemp(UArg arg) {
    // Build global temperature read command packet
    char tx[2];
    char rx[2];
    //tx[0] = 0b10000010; // 0b01000001 flipped
    tx[0] = TMP107_Global_bit | TMP107_Read_bit | motor_tmp107_addr;
    tx[1] = TMP107_Temp_reg;

    // Transmit global temperature read command
    TMP107_Transmit(tx, 2);
    // Master cannot transmit again until after we've received
    // the echo of our transmit and given the TMP107 adequate
    // time to reply. thus, we wait.
    int number = TMP107_WaitForEcho(2, 2, TMP107_Timeout);
    // Copy the response from TMP107 into user variable
    TMP107_RetrieveReadback(2, rx, number);

    // Convert two bytes received from TMP107 into degrees C
    float tmp107_temp = TMP107_DecodeTemperatureResult(rx[1], rx[0]);

//    System_printf("Temp: %d, %d, %f\n", rx[1], rx[0], tmp107_temp);
//    System_flush();
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
    currentSensorB = (ADC_V_REF/2.0 - V_OutB) / (ADC_GAIN * ADC_RESISTANCE);

    // Convert digital value
    V_OutC = ((float)ADC1ValueC[0] * ADC_V_REF) / ADC_RESOLUTION;
    currentSensorC = (ADC_V_REF/2.0 - V_OutC) / (ADC_GAIN * ADC_RESISTANCE);

    // Variables for the ring buffer (not quite a ring buffer though)
    static uint8_t currentBHead = 0;
    static uint8_t currentCHead = 0;
    currentSensorBBuffer[currentBHead++] = currentSensorB;
    currentBHead %= windowCurrent;
    currentSensorCBuffer[currentCHead++] = currentSensorC;
    currentCHead %= windowCurrent;

    // Check once a second if current limit exceeded
    countCurrentTicks++;
    if (countCurrentTicks >= CURR_CHECK_TICKS) {
        countCurrentTicks = 0;

        if (currentSensorB > thresholdCurrent ||
                currentSensorC > thresholdCurrent) {
            eStopMotor();
        }

//        System_printf("CSB: %f\n", currentSensorB);
//        System_printf("CSC: %f\n\n", currentSensorC);
//        System_flush();
    }
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// NOTE: THIS IS AC CELERATION OF THE BOARD, NOT THE MOTOR
void swiAcceleration(UArg arg) {
    // BMI160 Inertial Measurement Sensor
    // TODO Get acceleration readings on three axes
    // ABS is calculated from those three in a getter
    boardAcceleration = 1;
}

// Accelerometer interrupt when crash threshold reached
void callbackAccelerometer(UArg arg) {
    eStopMotor();
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

    return sum/windowLight;
}

uint8_t getBoardTemp() {
    return boardTemp;
}

uint8_t getMotorTemp() {
    return motorTemp;
}

float getCurrent() {
    float sumB = 0;
    float sumC = 0;

    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i < windowCurrent; i++) {
        sumB += currentSensorBBuffer[i];
        sumC += currentSensorCBuffer[i];
    }

    float avgB = sumB / windowCurrent;
    float avgC = sumC / windowCurrent;

    return ((avgB + avgC) * 3.0 / 2.0);
}

uint8_t getAcceleration() {
    return boardAcceleration;
}

void setThresholdTemp(uint8_t threshTemp) {
    thresholdTemp = threshTemp;
    // TODO Update TMP107 with new limit...
}

void setThresholdCurrent(uint16_t threshCurrent) {
    thresholdCurrent = threshCurrent;
}

void setThresholdAccel(uint16_t threshAccel) {
    thresholdAccel = threshAccel;
    // TODO Update BMI160 with new limit...
}
