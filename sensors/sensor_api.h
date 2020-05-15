/*
 * sensor_api.h
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include <stdbool.h>
#include <stdint.h>


#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_


// Initialises all the sensors
bool init_sensors(uint8_t accel_threshold);

// Initialises the light sensor (opt3001)
bool init_lightSensor();

// Initialises the board Temperature Sensors
bool init_boardTempSensor();

// Initialises the Motor Temperature Sensors
bool init_motorTempSensor();

// Initialises two current sensors
bool init_currentSensors();

// Initialises the sensors to find acceleration on the three axis
// Also setup accelerometer threshold callback.
bool init_acceleration(uint8_t threshold);

bool init_speedo();

// Read and filter light over I2C
uint8_t get_light();

// Read and filter board and motor temperature sensors over UART
uint8_t get_board_temp();
uint8_t get_motorTemp();

// Read and filter two motor phase currents via analogue signals on the current sensors
uint8_t* get_currentSensors();

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t get_acceleration();

// Measure and filter current motor speed (rpm)
uint8_t get_speedo();
void inc_rotations();

#endif /* SENSORS_SENSOR_API_H_ */
