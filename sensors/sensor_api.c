/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */
#include "sensor_api.h"

///////////**************??????????????
// God tier make everything work fxn //
///////////**************??????????????
bool init_sensors(uint8_t accel_threshold){
    return
            init_lightSensor() &&
            init_boardTempSensor() &&
            init_motorTempSensor() &&
            init_currentSensors() &&
            init_axisAcceleration() &&
            init_accelerometer(accel_threshold) &&
            init_speedo();
}

///////////**************??????????????
//   Implementations sensor setups   //
///////////**************??????????????

// Initialise light sensor via I2C
// Sample window size greater than 5 at >= 2Hz. Create a recurring SWI
bool init_lightSensor() {
    return true;
}

// Initialise board temperature sensors via UART
// Sample window size greater than 3 at >= 2Hz. Create a recurring SWI
bool init_boardTempSensor() {
    return true;
}

// Initialise motor temperature sensors via UART
// Sample window size greater than 3 at >= 2Hz. Create a recurring SWI
bool init_motorTempSensor() {
    return true;
}

// Initialise two of three motor phase current sensors via analogue signals
// Sample window size greater than 5 at >= 250Hz. Create a recurring SWI
bool init_currentSensors() {
    // E3 D3 A6
    return true;
}

// Initialise sensors for acceleration on all three axes
// Sample window size (of each axis) greater than 5 at 200Hz. Create a recurring SWI
bool init_acceleration() {
    return true;
}

// Initialise motor accelerometer, to detect a 'crash' threshold via the chips output interrupt pin
bool init_accelerometer(uint8_t threshold) {
    return true;
}

// Initialise speed sensor to measure motor
// Sample window size greater than 5 at >= 100Hz. Create a recurring SWI
bool init_speedo() {
    return true;
}

///////////**************??????????????
// Implementations of reading values //
///////////**************??????????????

// Read and filter light over I2C
// Sample window size greater than 5 at >= 2Hz
uint8_t get_light() {

    return 1;
}

// Read and filter board temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
uint8_t get_board_temp() {

    return 1;
}

// Read and filter motor temperature sensors over UART
// Sample window size greater than 3 at >= 2Hz
uint8_t get_motorTemp() {
    return 1;
}

// Read and filter two motor phase currents via analogue signals on the current sensors
// Sample window size greater than 5 at >= 250Hz
uint8_t val = 1;
uint8_t* get_currentSensors() {
    return &val;
}

// Read and filter acceleration on all three axes, and calculate absolute acceleration.
// Sample window size (of each axis) greater than 5 at 200Hz
uint8_t get_acceleration() {
    return 1;
}

// Accelerometer interrupt to detect user defined crash threshold (m/s^2)
void accelerometer_callback() {
    // Pass
}

// Measure and filter current motor speed (rpm)
// Sample window size greater than 5 at >= 100Hz
uint8_t get_speedo() {
    return 1;
}
