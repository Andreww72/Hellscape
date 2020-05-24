/*
 * sensor_api.h
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Clock.h>
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include <ti/drivers/UART.h>
#include "motor/motor_api.h"

// All the port macros are defined here
#include "sensor_ports.h"
#include "opt3001/opt3001.h"
#include "opt3001/i2cOptDriver.h"
#include "tmp107/tmp107.h"
#include "tmp107/hal.h"

// Initialises all the sensors:
// Light, board temp, motor temp, currents, acceleration (and interrupt)
bool initSensors(uint8_t thresholdTemp, uint16_t thresholdCurrent, uint16_t thresholdAccel);

// Read and filter light over I2C
float getLight();

// Read and filter board and motor temperature sensors over UART
uint8_t getBoardTemp();
uint8_t getMotorTemp();

// Read and filter two motor phase currents via analogue signals on the current sensors
float getCurrentSensorB();
float getCurrentSensorC();
float getCurrentTotal();

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t getAcceleration();

// Update threshold values that trigger an eStop
void setThresholdTemp(uint8_t threshTemp);
void setThresholdCurrent(uint16_t threshCurrent);
void setThresholdAccel(uint16_t threshAccel);

#endif /* SENSORS_SENSOR_API_H_ */
