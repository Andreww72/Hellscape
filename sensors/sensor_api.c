/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include "sensor_api.h"

// Current sensor constants
#define ADC_SEQB 0
#define ADC_SEQC 1
#define ADC_PRI 0
#define ADC_STEP 0
#define ADC_V_REF 3.3 // Page 1862 says ADC ref voltage 3.3V
#define ADC_RESISTANCE 0.07 // FAQ
#define ADC_GAIN 10.0 // FAQ
#define ADC_RESOLUTION 4095.0 // Page 1861 says resolution 12 bits

#define TMP_RESOLUTION 0.015625

// Data window size constants
#define windowLight 5
#define windowTemp 3
#define windowCurrent 5
#define windowAcceleration 5

// Data collectors (before filtering)
uint16_t lightBuffer[windowLight];
uint8_t boardTempBuffer[windowTemp];
uint8_t motorTempBuffer[windowTemp];
uint8_t currentSensorBBuffer[windowCurrent];
uint8_t currentSensorCBuffer[windowCurrent];
uint8_t accelerationBuffer[windowAcceleration];

// Current values (after filtering)
uint8_t light = 100;
uint8_t boardTemp = 25;
uint8_t motorTemp = 25;
float currentSensorB = 0;
float currentSensorC = 0;
uint8_t acceleration = 0;

// Thresholds that trigger eStop
uint16_t thresholdTemp = 40;
uint16_t thresholdCurrent = 1000;
uint16_t thresholdAccel = 1;

// Setup handles
UART_Handle uartBoard;
UART_Handle uartMotor;
Clock_Params clkParams;
Clock_Struct clockLightStruct;
Clock_Struct clockTempStruct;
Clock_Struct clockCurrentStruct;
Clock_Struct clockAccelerationStruct;

// Function prototypes that are not in the .h (deliberately)
bool initLight();
bool initBoardTemp();
bool initMotorTemp(uint16_t threshTemp);
bool initCurrent(uint16_t threshCurrent);
bool initAcceleration(uint16_t threshAccel);
void swiLight(UArg arg);
void swiBoardTemp(UArg arg);
void swiMotorTemp(UArg arg);
void swiCurrent(UArg arg);
void swiAcceleration(UArg arg);

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool initSensors(uint16_t threshTemp, uint16_t threshCurrent, uint16_t threshAccel) {

    // Used by separate init functions to create recurring SWIs. Period size is 1ms.
    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;

    return
            initLight() &&
            initBoardTemp() &&
            initMotorTemp(threshTemp) &&
            initCurrent(threshCurrent) &&
            initAcceleration(threshAccel);
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool initLight() {
    // Create a recurring 2Hz SWI swi_light
    clkParams.period = 500;
    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swiLight, 1, &clkParams);


    // I have just used the configuration for lab4b
    uint16_t config;
    readI2C(OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t*)&config);

    // Pin4 -> latch
    config |= (1<<4+8);

    // Pin3 -> polarity
    config &= ~(1<<3+8);

    writeI2C(OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t*)&config);

    uint16_t limit;
    readI2C(OPT3001_I2C_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&limit);

    // For this one, set bits [15:12] to zero (Table 12)
    limit &= ~(1<<7 | 1<<6 | 1<<5 | 1<<4);
    writeI2C(OPT3001_I2C_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&limit);

    // Set the high limit register to 2000-3000
    limit = 0b0010010010001001; // Don't think this is right
    bool status = writeI2C(OPT3001_I2C_ADDRESS, REG_HIGH_LIMIT, (uint8_t*)&limit);

    status &= sensorOpt3001Test();


    // TODO: setup the swi, because I definitely remember how to do that bit

    //uint8_t success = 1;
    if (status) {
        return true;
    } else {
        return false;
    }
}

// TEMPERATURE SETUP
bool initBoardTemp() {
    // Create a recurring 2Hz SWI swi_temp
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)initBoardTemp, 1, &clkParams);

    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    // TODO Setup UART connection for board TMP107
    // TODO Figure out the ports and pins then copy what I did for motor temp
    // PROBABLY ISN'T UART0
    uartBoard = UART_open(Board_UART0, &uartParams);
    if (uartBoard == NULL) {
         System_abort("Error opening the UART");
     }

    // TODO Setup board TMP107 temperature sensor

    return true;
}

bool initMotorTemp(uint16_t threshTemp) {
    // Create a recurring 2Hz SWI swi_temp
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiMotorTemp, 1, &clkParams);

    setThresholdTemp(threshTemp);

    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    // Setup UART connection for motor TMP107
    // UART7 and GPIO setup statically in EK file
    // TMP107_RX on PC4 with UART RX7
    // TMP107_TX on PC5 with UART TX7
    uartMotor = UART_open(Board_UART7, &uartParams);
    if (uartMotor == NULL) {
         System_abort("Error opening the UART");
     }
    // TODO Setup motor TM107 temperature sensor
    // Start on doc page 14
    uint8_t initTMP[3];
    uint8_t setTMP[5];
    uint8_t response[1];

    uint8_t calibrationByte = 0x55;
    uint8_t addressInitialiseByte = 0b10101001;
    uint8_t addressAssignByte = 0b10100001;
    uint8_t individualWriteByte = 0b00000001;
    // Configuration register 1h (page 24)
    uint8_t regPointerByte = 0b00000101;
    // What I want in config reg is 0b1000000100010000
    // However LSB is sent first thus:
    uint8_t configRegWord1 = 0b00001000;
    uint8_t configRegWord2 = 0b10000001;

    initTMP[0] = calibrationByte;
    initTMP[1] = addressInitialiseByte;
    initTMP[2] = addressAssignByte;
    setTMP[0] = calibrationByte;
    setTMP[1] = individualWriteByte;
    setTMP[2] = regPointerByte;
    setTMP[3] = configRegWord1;
    setTMP[4] = configRegWord2;

    // TODO Make tmp107 throw an interrupt (which throws estop) if threshold exceeded

    // Write setup data to TMP107
    int writeInit = UART_write(uartMotor, &initTMP, 3);
    // Might need a read here

    int writeSet = UART_write(uartMotor, &setTMP, 3);
    // Read device ok response
    // Investigate 7ms delay thing
    int readResp = UART_read(uartMotor, &response, 1);

    System_printf("Temperature setup\n");
    System_printf("%d %d %d\n", writeInit, writeSet, readResp);
    System_flush();

    return true;
}

// CURRENT SETUP
bool initCurrent(uint16_t thresholdCurrent) {
    setThresholdCurrent(thresholdCurrent);

    // Current sensors B and C on ADCs, A is not and thus not done.
    // Note GPIO ports already setup in EK file
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    // Current sensor B on D7 with ADC channel 4
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQB, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQB, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQB);
    ADCIntClear(ADC1_BASE, ADC_SEQB);

    // Current sensor C on E3 with ADC channel 3
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQC, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQC, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH3 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQC);
    ADCIntClear(ADC1_BASE, ADC_SEQC);

    clkParams.period = 4;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swiCurrent, 1, &clkParams);

    // TODO Throw an interrupt (which throws estop) if threshold exceeded

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
    // Copy what we did in lab 4 ish

    // Variables for the ring buffer (not quite a ring buffer though)
    static uint8_t light_head = 0;

    uint16_t data;
    if (!sensorOpt3001Read(&data))
        return; // If read is not successful, do nothing

    lightBuffer[light_head++] = data;
    light_head %= windowLight;
}

// Read and filter motor temperature sensors over UART
void swiBoardTemp(UArg arg) {
    // TODO read board temperature via UART
    // Probably copy swiMotorTemp
    boardTemp = 25;
}

// Read and filter motor temperature sensors over UART
void swiMotorTemp(UArg arg) {
    uint8_t read[3];
    uint8_t response[2];
    uint8_t calibrationByte = 0x55;
    uint8_t individualReadByte = 0b01000001;
    // Temperature register 0h (page 23)
    uint8_t regPointerByte = 000000101;

    read[0] = calibrationByte;
    read[1] = individualReadByte;
    read[2] = regPointerByte;

    // Write command
    UART_write(uartMotor, &read, 3);
    // Read response
    UART_read(uartMotor, &response, 2);

    System_printf("Temp:%d", response[0]);
    System_printf("Temp:%d", response[1]);
    System_flush();

    // Calculate temperature from binary result
    // Bits 15-2 contain temperature result. Bits 1-0 can be ignored
    // Example: 00 1100 1000 0000 = C80h = 3200 � (TMP_RESOLUTION / LSB)
    // Convert the 14-bit, left-justified, binary temperature to decimal
    uint16_t result = ((response[0] << 8) | (response[1] & 0xff)) >> 2;

    // Multiply decimal by resolution
    motorTemp = result * TMP_RESOLUTION;
}

// Read and filter two motor phase currents via analogue signals
void swiCurrent(UArg arg) {
    uint32_t ADC1ValueB[1], ADC1ValueC[1];
    uint32_t twelve_bitmask = 0xfff;
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
    // https://www.ti.com/lit/ds/symlink/tm4c1294ncpdt.pdf
    // Page 1861 section contains relevant information for constants

    // https://learn.sparkfun.com/tutorials/analog-to-digital-conversion/relating-adc-value-to-voltage
    // Analogue voltage = ADC reading * system voltage / ADC resolution
    // Current = (Vref/2 - Vsox) / Gcsa x Rsense

    // Convert digital value
    V_OutB = ((ADC1ValueB[0] & twelve_bitmask) * ADC_V_REF) / ADC_RESOLUTION;
    // I = V / R
    currentSensorB = (ADC_V_REF/2 - V_OutB) / (ADC_GAIN * ADC_RESISTANCE);

    // Convert digital value
    V_OutC = ((ADC1ValueC[0] & twelve_bitmask) * ADC_V_REF) / ADC_RESOLUTION;
    // I = V / R
    currentSensorC = (ADC_V_REF/2 - V_OutC) / (ADC_GAIN * ADC_RESISTANCE);
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
void swiAcceleration(UArg arg) {
    // BMI160 Inertial Measurement Sensor
    // TODO Get acceleration readings on three axes
    // ABS is calculated from those three in a getter
    acceleration = 1;
}

// Accelerometer interrupt when crash threshold reached
void callbackAccelerometer(UArg arg) {
    // TODO Change this to the estop method once made
    stopMotor_api();
}

///////////**************??????????????
//              Getters              //
///////////**************??????????????

float getLight() {
    // Lux must be converted from the raw values as uint16_t is cheaper to store

    float sum = 0;
    // This is fine since window is the size of the buffer
    uint8_t i;
    for (i = 0; i<windowLight; i++){
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

float getCurrentSensorB() {
    return currentSensorB;
}

float getCurrentSensorC() {
    return currentSensorC;
}

float getCurrentTotal() {
    return (currentSensorB + currentSensorC) * 3.0 / 2.0;
}

uint8_t getAcceleration() {
    return acceleration;
}

void setThresholdTemp(uint16_t threshTemp) {
    thresholdTemp = threshTemp;
    // TODO Update TMP107 with new limit...
}

void setThresholdCurrent(uint16_t threshCurrent) {
    thresholdCurrent = threshCurrent;
    // TODO Think this will be a manual check every so often
}

void setThresholdAccel(uint16_t threshAccel) {
    thresholdAccel = threshAccel;
    // TODO Update BMI160 with new limit...
}
