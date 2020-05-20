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
//#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/gates/GateHwi.h>

#include "sensors/sensor_api.h"
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
 * Stop the motor... quickly
 */
void eStopMotor();

/**
 * Stop the motor
 */
void stopMotor_api();

/**
 * Set the desired speed of the motor in rpm.
 */
void setDesiredSpeed(int rpm);

/**
 * Get the speed of the motor in RPM.
 */
int getSpeed();




