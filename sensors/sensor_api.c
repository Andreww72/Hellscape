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
#define ADC_GAIN_BY_RESISTANCE 0.007 // Page 1861 says 2500 ohms for Radc
#define ADC_RESOLUTION 4095.0 // Page 1861 says resolution 12 bits

#define TMP_RESOLUTION 0.015625

// Data window size constants
#define windowLight 5
#define windowTemp 3
#define windowCurrent 5
#define windowAcceleration 5

// Data collectors (before filtering)
uint8_t lightBuffer[windowLight];
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
bool initMotorTemp();
bool initCurrent();
bool initAcceleration(uint8_t threshold);
void swiLight(UArg arg);
void swiBoardTemp(UArg arg);
void swiMotorTemp(UArg arg);
void swiCurrent(UArg arg);
void swiAcceleration(UArg arg);

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool initSensors(uint8_t accel_threshold){

    // Used by separate init functions to create recurring SWIs. Period size is 1ms.
    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;

    return
            initLight() &&
            initBoardTemp() &&
            initMotorTemp() &&
            initCurrent() &&
            initAcceleration(accel_threshold);
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool initLight() {
    // Create a recurring 2Hz SWI swi_light
    clkParams.period = 500;
    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swiLight, 1, &clkParams);

    // TODO Setup I2C connection to OPT3001

    // TODO Setup light OPT3001 on board

    System_printf("Light setup\n");
    System_flush();

    return true;
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

bool initMotorTemp() {
    // Create a recurring 2Hz SWI swi_temp
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swiMotorTemp, 1, &clkParams);

    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    // Setup UART connection for motor TMP107
    // UART7 and GPIO setup statically in EK file
    // TMP107_TX on PC4 with UART RX7
    // TMP107_RX on PC5 with UART TX7
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

    // Write setup data to TMP107
    int writeInit = UART_write(uartMotor, &initTMP, 3);
    int writeSet = UART_write(uartMotor, &setTMP, 3);
    //Read device ok response
    int readResp = UART_read(uartMotor, &response, 1);

    System_printf("Temperature setup\n");
    System_printf("%d %d %d\n", writeInit, writeSet, readResp);
    System_flush();

    return true;
}

// CURRENT SETUP
bool initCurrent() {
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

    System_printf("Current setup\n", currentSensorC);
    System_flush();

    return true;
}

// ACCELERATION SETUP
// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool initAcceleration(uint8_t threshold) {
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
    // Copy what we did in lab 4
    light = 100;
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
    // Example: 00 1100 1000 0000 = C80h = 3200 × (TMP_RESOLUTION / LSB)
    // Convert the 14-bit, left-justified, binary temperature to decimal
    uint16_t result = ((response[0] << 8) | (response[1] & 0xff)) >> 2;

    // Multiply decimal by resolution
    motorTemp = result * TMP_RESOLUTION;
}

// Read and filter two motor phase currents via analogue signals
void swiCurrent(UArg arg) {
    uint32_t ADC0ValueB[1], ADC1ValueC[1];
    uint32_t twelve_bitmask = 0xfff;
    float V_OutA = 0;
    float V_OutB = 0;

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
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQB, ADC0ValueB);
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQC, ADC1ValueC);

    // CONVERT TO DIGITAL THAN CURRENT
    // https://www.ti.com/lit/ds/symlink/tm4c1294ncpdt.pdf
    // Page 1861 section contains relevant information for constants

    // https://learn.sparkfun.com/tutorials/analog-to-digital-conversion/relating-adc-value-to-voltage
    // Analogue voltage = ADC reading * system voltage / ADC resolution
    // Current = (Vref/2 - Vsox) / Gcsa x Rsense

    // Convert digital value
    V_OutB = ((ADC0ValueB[0] & twelve_bitmask) * ADC_V_REF) / ADC_RESOLUTION;
    // I = V / R
    currentSensorB = (ADC_V_REF/2 - V_OutB) / ADC_GAIN_BY_RESISTANCE;

    // Convert digital value
    V_OutC = ((ADC1ValueC[0] & twelve_bitmask) * ADC_V_REF) / ADC_RESOLUTION;
    // I = V / R
    currentSensorC = (ADC_V_REF/2 - V_OutC) / ADC_GAIN_BY_RESISTANCE;
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

uint8_t getLight() {
    return light;
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
