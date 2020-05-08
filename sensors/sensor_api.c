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

bool init_lightSensor(){
    // TODO: Initialise light sensor
    return true;
}

bool init_boardTempSensor(){
    // TODO: Initialise board temperature sensors
    return true;
}

bool init_motorTempSensor() {
    // TODO: initialise motor temperature sensors

    return true;
}

bool init_currentSensors() {
    // TODO: initialise motor temperature sensors

    return true;
}

bool init_axisAcceleration() {
    // TODO: initialise motor temperature sensors

    return true;
}

bool init_accelerometer(uint8_t threshold) {
    // TODO: initialise motor temperature sensors

    return true;
}

bool init_speedo() {
    // TODO: initialise motor temperature sensors

    return true;
}

///////////**************??????????????
// Implementations of reading values //
///////////**************??????????????

// Read and filter light over I2C
uint8_t get_light() {
    // TODO: It

    return 1
}

// Read and filter board and motor temperature sensors over UART
uint8_t get_board_temp() {
    // TODO: It

    return 1
}

uint8_t get_motorTemp() {
    // TODO: It

    return 1
}

// Read and filter two motor phase currents via analog signals on the current sensors
uint8_t* get_currentSensors() {
    // TODO: It

    return 1
}

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t get_accelerations() {
    // TODO: It

    return 1
}

uint8_t get_absAcceleration() {
    // TODO: It

    return 1
}

// Interrupt to detect user defined crash threshold (m/s^2)
void accelerometer_callback() {
    // TODO: It

    return 1
}

// Measure and filter current motor speed (rpm)
uint8_t get_speedo() {
    // TODO: It

    return 1
}
