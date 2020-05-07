/*
 * motor_api.h
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */


#include <stdbool.h>
#include <xdc/runtime/Error.h>
#include "drivers/motorlib.h"

/**
 * Initialise the motor
 *
 * Returns 1 for success or 0 for failure
 */
bool initMotor();

/**
 * Perform an emergency stop
 *
 * Returns 1 for success or 0 for failure
 */
int eStop();

/**
 * Set speed of the motor in rpm
 *
 * returns 1 for success or 0 for failure
 */
int setSpeed(int speed_rpm);




