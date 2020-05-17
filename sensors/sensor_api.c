/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include "sensor_api.h"

// Data window sizes
#define windowLight 5
#define windowTemp 3
#define windowCurrent 5
#define windowAcceleration 5

// Current sensor values
#define VCC 5 // According to sensor datasheet
#define SENSITIVITY 0.2 // 200 millVolts/A = 0.2 V/A for 10 AB (our current sensor)
#define FIRST_STEP 0
#define ADC_SEQUENCE 0 // Could be any value from 0 to 3 since only one sample is needed at a given request
#define ADC_PRIORITY 0
#define RESOLUTION 4095 // max digital value for 12 bit sample
#define REF_VOLTAGE_PLUS 3.3 // Reference voltage used for ADC process, given in page 2149 of TM4C129XNCZAD Microcontroller Data Sheet
#define NEUTRAL_VIOUT 0.5*VCC

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

// SWI function prototypes
void swi_light(UArg arg);
void swi_temp(UArg arg);
void swi_current(UArg arg);
void swi_acceleration(UArg arg);
static float calcTemp(float previous, float status);

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
    ADCSequenceConfigure(ADC0_BASE, ADC_SEQUENCE, ADC_TRIGGER_PROCESSOR, ADC_PRIORITY);
    ADCSequenceStepConfigure(ADC0_BASE, ADC_SEQUENCE, FIRST_STEP, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, ADC_SEQUENCE);
    ADCIntClear(ADC0_BASE, ADC_SEQUENCE);

    // Current sensor C on E3 with ADC channel 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    ADCSequenceConfigure(ADC1_BASE, ADC_SEQUENCE, ADC_TRIGGER_PROCESSOR, ADC_PRIORITY);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQUENCE, FIRST_STEP, ADC_CTL_IE | ADC_CTL_CH3 | ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE, ADC_SEQUENCE);
    ADCIntClear(ADC1_BASE, ADC_SEQUENCE);

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
    // Copy what we did in lab 4 ish
    light = 100;
}

// Read and filter board and motor temperature sensors over UART
void swi_temp(UArg arg) {
    // Board temperature
    boardTemp = calcTemp(25, 25);

    // Motor sensor next to the motor3
    // Get temp
    // Calc temp
    // To convert a positive digital data format to temperature:
    // Convert the 14-bit, left-justified, binary temperature result to a decimal number. Then, multiply the decimal
    // number by the resolution to obtain the positive temperature.
    // Example: 00 1100 1000 0000 = C80h = 3200 × (0.015625°C / LSB) = 50°C
    // Set temp
    motorTemp = calcTemp(25, 25);
}

// Read and filter two motor phase currents via analogue signals
void swi_current(UArg arg) {
    uint32_t pui32ADC0ValueB[1], pui32ADC0ValueC[1], twelve_bitmask = 0xfff;
    double VIOUT = 0;

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, ADC_SEQUENCE);
    ADCProcessorTrigger(ADC1_BASE, ADC_SEQUENCE);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, ADC_SEQUENCE, false));
    while(!ADCIntStatus(ADC1_BASE, ADC_SEQUENCE, false));

    // Clear ADC Interrupt
    ADCIntClear(ADC0_BASE, ADC_SEQUENCE);
    ADCIntClear(ADC1_BASE, ADC_SEQUENCE);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, ADC_SEQUENCE, pui32ADC0ValueB);
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQUENCE, pui32ADC0ValueC);

    // TODO Check these calcs and #defines
    // Convert digital value to current reading (VREF- is 0, so it can be ignored)
    VIOUT = ((pui32ADC0ValueB[0] & twelve_bitmask) * REF_VOLTAGE_PLUS) / RESOLUTION;
    currentSensorB = (VIOUT - NEUTRAL_VIOUT) / SENSITIVITY;

    // Convert digital value to current reading (VREF- is 0, so it can be ignored)
    VIOUT = ((pui32ADC0ValueC[0] & twelve_bitmask) * REF_VOLTAGE_PLUS) / RESOLUTION;
    currentSensorC = (VIOUT - NEUTRAL_VIOUT) / SENSITIVITY;
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

///////////**************??????????????
//              Helpers              //
///////////**************??????????????
static float calcTemp(float previous, float status) {
    return status;
}
