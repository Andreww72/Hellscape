#include <stdbool.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <driverlib/gpio.h>
#include <ti/drivers/GPIO.h>
#include <ti/sysbios/gates/GateHwi.h>

#include "ui/user_interface.h"
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
void startMotor(int rpm);

/**
 * eStop of the motor - use in sensor interrupts for estop conditions
 */
void eStopMotor();

/**
 * Stop the motor normally
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

/**
 * Get the speed of the motor in RPM (float).
 */
float getSpeedFloat();




