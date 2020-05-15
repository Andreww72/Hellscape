/*
 * motor_api.h
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */


#include <stdbool.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <driverlib/gpio.h>
#include <ti/drivers/GPIO.h>
#include <inc/hw_memmap.h>

#include "drivers/motorlib.h"
#include "Board.h"

/**
 * Initialise the motor
 *
 * Returns 1 for success or 0 for failure
 */
bool initMotor();

/**
 * Start the motor
 */
void startMotor(int duty_pct);

/**
 * Perform an emergency stop
 *
 * Returns 1 for success or 0 for failure
 */
int eStop();

void stopMotor_api();

/**
 * Set speed of the motor in rpm
 *
 * returns 1 for success or 0 for failure
 */
int setSpeed(int speed_rpm);




