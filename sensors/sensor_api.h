/*
 * sensor_api.h
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */

#include <stdbool.h>


#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_


// Initialises all the sensors
bool init_sensors();

// Initialises the light sensor (opt3001)
bool init_lightSensor();

// Initialises the board Temperature Sensors
bool init_boardTempSensors();

// Initialises the Motor Temperature Sensors
bool init_motorTempSensors();







#endif /* SENSORS_SENSOR_API_H_ */
