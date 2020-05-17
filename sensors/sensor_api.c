/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Clock.h>

#include "sensor_api.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <ti/sysbios/knl/Clock.h>


uint8_t lightBuffer[10];
uint8_t boardTempBuffer[5];
uint8_t motorTempBuffer[5];
uint8_t currentSensorBBuffer[10];
uint8_t currentSensorCBuffer[10];
uint8_t accelerationBuffer[10];
uint8_t light = 0;
uint8_t boardTemp = 0;
uint8_t motorTemp = 0;
uint8_t currentSensorB = 0;
uint8_t currentSensorC = 0;
uint8_t acceleration = 0;

Clock_Params clkParams;
Clock_Struct clockCurrentStruct;

// Function prototypes
void swi_light(UArg arg);
void swi_boardTemp(UArg arg);
void swi_motorTemp(UArg arg);
void swi_currentSensors(UArg arg);
void swi_acceleration(UArg arg);

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool init_sensors(uint8_t accel_threshold){

    Clock_Params_init(&clkParams);
    clkParams.startFlag = TRUE;

    return
            init_lightSensor() &&
            init_boardTempSensor() &&
            init_motorTempSensor() &&
            init_currentSensors() &&
            init_acceleration(accel_threshold);
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????

// Initialise light sensor via I2C
bool init_lightSensor() {
    // Create a recurring 2Hz SWI swi_light
    clkParams.period = 500;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_light, 1, &clkParams);

    // Setup I2C connection

    System_printf("Acceleration setup\n");
    System_flush();

    return true;
}

// Initialise board temperature sensors via UART
bool init_boardTempSensor() {
    // Create a recurring 2Hz SWI swi_board_temp
    clkParams.period = 500;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_motorTemp, 1, &clkParams);

    // Setup UART connection

    System_printf("Acceleration setup\n");
    System_flush();

    return true;
}

// Initialise motor temperature sensors via UART
bool init_motorTempSensor() {
    // GPIO setup statically (done)
    // RX(7): PC4
    // TX(7): PC5

    // Create a recurring 2Hz SWI swi_motor_temp
    clkParams.period = 500;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_boardTemp, 1, &clkParams);

    // Setup UART connection

    // Setup TMP107 sensor

    System_printf("Motor temp setup\n");
    System_flush();

    return true;
}

#define VCC 5 // According to sensor datasheet
#define SENSITIVITY 0.2 // 200 millVolts/A = 0.2 V/A for 10 AB (our current sensor)
#define FIRST_STEP 0
#define SEQUENCE_NUMBER 0 // Could be any value from 0 to 3 since only one sample is needed at a given request
#define ADC_PRIORITY 0
#define RESOLUTION 4095 // max digital value for 12 bit sample
#define REF_VOLTAGE_PLUS 3.3 // Reference voltage used for ADC process, given in page 2149 of TM4C129XNCZAD Microcontroller Data Sheet
#define NEUTRAL_VIOUT 0.5*VCC

// Initialise two of three motor phase current sensors via analogue signal (use ADC)
// Sample window size greater than 5 at >= 250Hz. Create a recurring SWI
bool init_currentSensors() {
    // Current B (D7) and current C (E3) GPIO setup statically
    // Setup ADC channels 4 (B) and 3 (C)
    // Create a recurring 250Hz SWI swi_currentSensors

    // Initialise ADC hardware (Page 11 of DK-TM4C129X User's Guide has relevant board pin information)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);

    // Enable sample sequence 3 with a processor signal trigger. Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion. Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3. This example is arbitrarily using sequence 3.
    ADCSequenceConfigure(ADC0_BASE, SEQUENCE_NUMBER, ADC_TRIGGER_PROCESSOR, ADC_PRIORITY);
    ADCSequenceConfigure(ADC1_BASE, SEQUENCE_NUMBER, ADC_TRIGGER_PROCESSOR, ADC_PRIORITY);

    // Configure step 0 on sequence 0. Sample channel 0 (ADC_CTL_CH0) in
    // differential mode (ADC_CTL_D) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done. Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END). Sequence
    // 3 has only one programmable step. Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps. Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0. For more
    // information on the ADC sequences and steps, refer to the datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, SEQUENCE_NUMBER, FIRST_STEP, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);
    ADCSequenceStepConfigure(ADC1_BASE, SEQUENCE_NUMBER, FIRST_STEP, ADC_CTL_IE | ADC_CTL_CH3 | ADC_CTL_END);

    // Since sample sequence 0 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, SEQUENCE_NUMBER);
    ADCSequenceEnable(ADC1_BASE, SEQUENCE_NUMBER);

    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    ADCIntClear(ADC0_BASE, SEQUENCE_NUMBER);
    ADCIntClear(ADC1_BASE, SEQUENCE_NUMBER);

    clkParams.period = 4;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_currentSensors, 1, &clkParams);

    System_printf("Current setup\n", currentSensorC);
    System_flush();

    return true;
}

// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool init_acceleration(uint8_t threshold) {
    // Create a recurring 200Hz SWI swi_currentSensors
    // Setup BMI160 Inertial Measurement Sensor
    // Setup callback_accelerometer to trigger if BM160 measures a value above threshold

    clkParams.period = 5;
    Clock_construct(&clockCurrentStruct, (Clock_FuncPtr)swi_acceleration, 1, &clkParams);

    System_printf("Acceleration setup\n");
    System_flush();

    return true;
}

///////////**************??????????????
// Implementations of SWI functions  //
///////////**************??????????????

// Read and filter light over I2C
// Sample window size greater than 5 at >= 2Hz
void swi_light(UArg arg) {
    // On sensor booster pack
    // Copy what we did in lab 4 ish
}

// Read and filter board temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
void swi_boardTemp(UArg arg) {
    // On sensor booster pack
    // TMP107 sensor
}

// Read and filter motor temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
void swi_motorTemp(UArg arg) {
    // Sensor next to the motor
    // TMP107 sensor
    // RX(7): PC4
    // TX(7): PC5
}

// Read and filter two motor phase currents via analogue signals on the current sensors
// Sample window size greater than 5 at >= 250Hz
void swi_currentSensors(UArg arg) {
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use. This example
    // uses sequence 3 which has a FIFO depth of 1. If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    uint32_t pui32ADC0ValueB[1], pui32ADC0ValueC[1], twelve_bitmask = 0xfff;
    double VIOUT = 0;

    // Trigger the ADC conversion.
    ADCProcessorTrigger(ADC0_BASE, SEQUENCE_NUMBER);
    ADCProcessorTrigger(ADC1_BASE, SEQUENCE_NUMBER);

    // Wait for conversion to be completed.
    while(!ADCIntStatus(ADC0_BASE, SEQUENCE_NUMBER, false));
    while(!ADCIntStatus(ADC1_BASE, SEQUENCE_NUMBER, false));

    //Clear ADC Interrupt
    ADCIntClear(ADC0_BASE, SEQUENCE_NUMBER);
    ADCIntClear(ADC1_BASE, SEQUENCE_NUMBER);

    // Read ADC Value.
    ADCSequenceDataGet(ADC0_BASE, SEQUENCE_NUMBER, pui32ADC0ValueB);
    ADCSequenceDataGet(ADC1_BASE, SEQUENCE_NUMBER, pui32ADC0ValueC);

    // Convert digital value to current reading (VREF- is 0, so it can be ignored)
    VIOUT = ((pui32ADC0ValueB[0] & twelve_bitmask) * REF_VOLTAGE_PLUS) / RESOLUTION;
    currentSensorB = (VIOUT - NEUTRAL_VIOUT) / SENSITIVITY;

    // Convert digital value to current reading (VREF- is 0, so it can be ignored)
    VIOUT = ((pui32ADC0ValueC[0] & twelve_bitmask) * REF_VOLTAGE_PLUS) / RESOLUTION;
    currentSensorC = (VIOUT - NEUTRAL_VIOUT) / SENSITIVITY;

    System_printf("B: %d\n", currentSensorB);
    System_printf("C: %d\n", currentSensorC);
    System_flush();
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// Sample window size (of each axis) greater than 5 at 200Hz
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
void swi_acceleration(UArg arg) {
    // BMI160 Inertial Measurement Sensor
}

// Accelerometer interrupt to detect user defined crash threshold (m/s^2)
void callback_accelerometer(UArg arg) {
    // BMI160 Inertial Measurement Sensor
    // Trigger interrupt when >= threshold
}

///////////**************??????????????
//              Getters              //
///////////**************??????????????

uint8_t get_light() {
    return light;
}

uint8_t get_board_temp() {
    return boardTemp;
}

uint8_t get_motorTemp() {
    return motorTemp;
}

uint8_t get_currentSensorB() {
    return currentSensorB;
}

uint8_t get_currentSensorC() {
    return currentSensorC;
}

uint8_t get_acceleration() {
    return acceleration;
}
