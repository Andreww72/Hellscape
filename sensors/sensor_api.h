/*
 * sensor_api.h
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include <stdbool.h>
#include <stdint.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Clock.h>
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <ti/drivers/UART.h>
#include "motor/motor_api.h"

#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_


// Initialises all the sensors
bool init_sensors(uint8_t accel_threshold);

// Initialises the light sensor (opt3001)
bool init_light();

// Initialises the board and motor temperature Sensors
bool init_temp();

// Initialises two current sensors
bool init_current();

// Initialises the sensors to find acceleration on the three axis
// Also setup accelerometer threshold callback.
bool init_acceleration(uint8_t threshold);

// Read and filter light over I2C
uint8_t get_light();

// Read and filter board and motor temperature sensors over UART
uint8_t get_boardTemp();
uint8_t get_MotorTemp();

// Read and filter two motor phase currents via analogue signals on the current sensors
float get_currentSensorB();
float get_currentSensorC();
float get_currentTotal();

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t get_acceleration();

#endif /* SENSORS_SENSOR_API_H_ */
