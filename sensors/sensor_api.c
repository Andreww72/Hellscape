/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include "sensor_api.h"

// Current sensor constants
#define ADC_SEQ 0
#define ADC_PRI 0
#define ADC_STEP 0
#define V_REF 3.3 // Page 1862 says ADC ref voltage 3.3V
#define RESISTANCE 2500.0 // Page 1861 says 2500 ohms for Radc
#define RESOLUTION 4095.0 // Page 1861 says resolution 12 bits
#define V_NEUTRAL V_REF / 2.0 // Centralise reading

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

// Function prototypes that are not in the .h
bool init_light();
bool init_temp();
bool init_current();
bool init_acceleration(uint8_t threshold);
void swi_light(UArg arg);
void swi_temp(UArg arg);
void swi_current(UArg arg);
void swi_acceleration(UArg arg);

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool init_sensors(uint8_t accel_threshold){

    // Used by separate init functions to create recurring SWIs. Period size is 1ms.
    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;

    return
            init_light() &&
            init_temp() &&
            init_current() &&
            init_acceleration(accel_threshold);
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????
// LIGHT SETUP
bool init_light() {
    // Create a recurring 2Hz SWI swi_light
    clkParams.period = 500;
    Clock_construct(&clockLightStruct, (Clock_FuncPtr)swi_light, 1, &clkParams);

    // TODO Setup I2C connection to OPT3001

    // TODO Setup light OPT3001 on board

    System_printf("Light setup\n");
    System_flush();

    return true;
}

// TEMPERATURE SETUP
bool init_temp() {
    // Create a recurring 2Hz SWI swi_temp
    clkParams.period = 500;
    Clock_construct(&clockTempStruct, (Clock_FuncPtr)swi_temp, 1, &clkParams);

//    UART_Params uartParams;
//    UART_Params_init(&uartParams);
//    uartParams.baudRate = 115200;

    // TODO Setup UART connection for board TMP107
//    uartBoard = UART_open(Board_UART?, &uartParams);
//    if (uartBoard == NULL) {
//         System_abort("Error opening the UART");
//     }
    // TODO Setup board TMP107 temperature sensor


    // TODO Setup UART connection for motor TMP107
    // UART7 and GPIO setup statically in EK file
    // TMP107_TX on PC4 with UART RX7
    // TMP107_RX on PC5 with UART TX7
//    uartMotor = UART_open(Board_UART7, &uartParams);
//    if (uartMotor == NULL) {
//         System_abort("Error opening the UART");
//     }
//    // TODO Setup motor TM107 temperature sensor
//    // Temperature register 0h (page 23)
//    // Configuration register 1h (page 24)
//
//    uint8_t setupCalibrationByte = 0x55;
//    uint8_t setupCommandPhase = 0b10101001;
//    //uint8_t setupAddressAssign = 0b101AAAAA;
//    uint8_t setup[3];
//    setup[0] = setupCalibrationByte;
//    setup[1] = setupCalibrationByte;
//    setup[2] = setupCalibrationByte;

    //UART_write(uartMotor, &setup, 3);

    // Address initialise (page 17)

    // Calibration phase

    // Command and address phase

    // Register pointer phase

    // Data phase

    // Initialise Calibration Constants

    //UART_read(uartMotor, &buffer, 3);

    System_printf("Temperature setup\n");
    System_flush();

    return true;
}

// CURRENT SETUP
bool init_current() {
    // Current sensors B and C on ADCs, A is not and thus not done.
    // Note GPIO ports already setup in EK file

    // Current sensor B on D7 with ADC channel 4
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQ, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQ, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQ);
    ADCIntClear(ADC0_BASE, ADC_SEQ);

    // Current sensor C on E3 with ADC channel 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ, ADC_TRIGGER_PROCESSOR, ADC_PRI);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ, ADC_STEP, ADC_CTL_IE | ADC_CTL_CH3 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);

    clkParams.period = 4;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_current, 1, &clkParams);

    System_printf("Current setup\n", currentSensorC);
    System_flush();

    return true;
}

// ACCELERATION SETUP
// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool init_acceleration(uint8_t threshold) {
    // Create a recurring 200Hz SWI swi_acceleration
    clkParams.period = 5;
    Clock_construct(&clockAccelerationStruct, (Clock_FuncPtr)swi_acceleration, 1, &clkParams);

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
void swi_light(UArg arg) {
    // On sensor booster pack
    // Copy what we did in lab 4
    light = 100;
}

// Read and filter board and motor temperature sensors over UART
void swi_temp(UArg arg) {
    // Board temperature
    boardTemp = 25;

    // Motor sensor next to the motor3
    // Get temp
    // Calc temp
    // To convert a positive digital data format to temperature:
    // Convert the 14-bit, left-justified, binary temperature result to a decimal number. Then, multiply the decimal
    // number by the resolution to obtain the positive temperature.
    // Example: 00 1100 1000 0000 = C80h = 3200 × (0.015625°C / LSB) = 50°C
    // Set temp
    motorTemp = 25;
}

// Read and filter two motor phase currents via analogue signals
void swi_current(UArg arg) {
    uint32_t ADC0ValueB[1], ADC1ValueC[1];
    uint32_t twelve_bitmask = 0xfff;
    float V_OutA = 0;
    float V_OutB = 0;

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQ);
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQ);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, ADC_SEQ, false));
    while(!ADCIntStatus(ADC1_BASE, ADC_SEQ, false));

    // Clear ADC Interrupt
    ADCIntClear(ADC0_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQ, ADC0ValueB);
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ, ADC1ValueC);

    // CONVERT TO DIGITAL THAN CURRENT
    // https://www.ti.com/lit/ds/symlink/tm4c1294ncpdt.pdf
    // Page 1861 section contains relevant information for constants

    // https://learn.sparkfun.com/tutorials/analog-to-digital-conversion/relating-adc-value-to-voltage
    // ADC resolution / system voltage = ADC reading / analogue voltage
    // Analogue voltage = ADC reading * system voltage / ADC resolution

    // Convert digital value
    V_OutA = ((ADC0ValueB[0] & twelve_bitmask) * V_REF) / RESOLUTION;
    // I = V / R
    currentSensorB = (V_OutA - V_NEUTRAL) / RESISTANCE;

    // Convert digital value
    V_OutB = ((ADC1ValueC[0] & twelve_bitmask) * V_REF) / RESOLUTION;
    // I = V / R
    currentSensorC = (V_OutB - V_NEUTRAL) / RESISTANCE;
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
void swi_acceleration(UArg arg) {
    // BMI160 Inertial Measurement Sensor
    // TODO Get acceleration readings on three axes
    // ABS is calculated from those three in a getter
    acceleration = 1;
}

// Accelerometer interrupt when crash threshold reached
void callback_accelerometer(UArg arg) {
    // TODO Change this to the estop method once made
    stopMotor_api();
}

///////////**************??????????????
//              Getters              //
///////////**************??????????????

uint8_t get_light() {
    return light;
}

uint8_t get_boardTemp() {
    return boardTemp;
}

uint8_t get_motorTemp() {
    return motorTemp;
}

float get_currentSensorB() {
    return currentSensorB;
}

float get_currentSensorC() {
    return currentSensorC;
}

float get_currentTotal() {
    return (currentSensorB + currentSensorC) * 3.0 / 2.0;
}

uint8_t get_acceleration() {
    return acceleration;
}
