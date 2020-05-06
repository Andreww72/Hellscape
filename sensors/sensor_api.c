/*
 * sensor_api.c
 *
 *  Created on: 6 May 2020
 *      Author: Tristan
 */
#include "sensor_api.h"


bool init_sensors(){
    return
            init_lightSensor() &&
            init_boardTempSensors() &&
            init_motorTempSensors();
}

bool init_lightSensor(){
    // TODO: Initialise light sensor
    return true;
}

bool init_boardTempSensors(){
    // TODO: Initialise board temperature sensors
    return true;
}

bool init_motorTempSensors() {
    // TODO: initialise motor temperature sensors

    return true;

}
