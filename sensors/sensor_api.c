/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */
#include "sensor_api.h"

uint8_t lightBuffer[10];
uint8_t boardTempBuffer[5];
uint8_t motorTempBuffer[5];
uint8_t currentSensorsBuffer[10];
uint8_t accelerationBuffer[10];
uint8_t speedoBuffer[10];
uint8_t light = 0;
uint8_t boardTemp = 0;
uint8_t motorTemp = 0;
uint8_t currentSensors[] = {0, 0, 0};
uint8_t acceleration = 0;
uint8_t speedo = 0;
int rotations = 0;


///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool init_sensors(uint8_t accel_threshold){
    return
            init_lightSensor() &&
            init_boardTempSensor() &&
            init_motorTempSensor() &&
            init_currentSensors() &&
            init_acceleration(accel_threshold) &&
            init_speedo();
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????

// Initialise light sensor via I2C
bool init_lightSensor() {
    // Create a recurring 2Hz SWI swi_light
    // Setup I2C connection

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

// Initialise board temperature sensors via UART
bool init_boardTempSensor() {
    // Create a recurring 2Hz SWI swi_board_temp
    // Setup UART connection

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

// Initialise motor temperature sensors via UART
bool init_motorTempSensor() {
    // Create a recurring 2Hz SWI swi_motor_temp
    // Setup UART connection

    // PC_4 on A3
    // PC_5 on A4

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

// Initialise two of three motor phase current sensors via analogue signal (use ADC)
// Sample window size greater than 5 at >= 250Hz. Create a recurring SWI
bool init_currentSensors() {
    // Create a recurring 250Hz SWI swi_currentSensors
    // Setup analogue signal pins
    // Port E3 D7 A6 use ADCs 12, 4, 3
    // Do current C (E3) and current B (D7) as they are on analogue. A is UART.

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

// Initialise sensors for acceleration on all three axes
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
bool init_acceleration(uint8_t threshold) {
    // Create a recurring 200Hz SWI swi_currentSensors
    // Setup BMI160 Inertial Measurement Sensor
    // Setup callback_accelerometer to trigger if BM160 measures a value above threshold

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

// Initialise speed sensor to measure motor
// Sample window size greater than 5 at >= 100Hz. Create a recurring SWI
bool init_speedo() {
    // Hall sensors already setup by motor.c
    // Create a recurring 100Hz SWI swi_currentSensors

    uint8_t success = 1;
    if (success) {
        return true;
    } else {
        return false;
    }
}

///////////**************??????????????
// Implementations of SWI functions  //
///////////**************??????????????

// Read and filter light over I2C
// Sample window size greater than 5 at >= 2Hz
void swi_light() {
    // On sensor booster pack
    // Copy what we did in lab 4 ish
}

// Read and filter board temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
void swi_board_temp() {
    // On sensor booster pack
    // TMP107 sensor
}

// Read and filter motor temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
void swi_motorTemp() {
    // Sensor next to the motor
    // TMP107 sensor
    // RX(7): PC4
    // TX(7): PC5
}

// Read and filter two motor phase currents via analogue signals on the current sensors
// Sample window size greater than 5 at >= 250Hz
void swi_currentSensors() {
    // Port E3 D7 A6
    // ADC 12, 4, 3
    // Do current C (E3) and current B (D7) as they are on analogue. A is UART
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// Sample window size (of each axis) greater than 5 at 200Hz
// NOTE: THIS IS ACCELERATION OF THE BOARD, NOT THE MOTOR
void swi_acceleration() {
    // BMI160 Inertial Measurement Sensor
}

// Accelerometer interrupt to detect user defined crash threshold (m/s^2)
void callback_accelerometer() {
    // BMI160 Inertial Measurement Sensor
    // Trigger interrupt when >= threshold
}

// Measure and filter current motor speed (rpm)
// Sample window size greater than 5 at >= 100Hz
// I need degrees between a rotation off a datasheet
// Time between rotations of hall sensors should give velocity somehow
void swi_speedo() {
    // GPIO_read(Board_HALLA);
    // Seconds per rev
    // Convert to RPM
    // Return RPM
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

uint8_t* get_currentSensors() {
    return currentSensors;
}

uint8_t get_acceleration() {
    return acceleration;
}

uint8_t get_speedo() {
    return speedo;
}

void inc_rotations() {
    rotations++;
}
