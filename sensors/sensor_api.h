#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <driverlib/adc.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>

#include "motor/motor_api.h"
#include "ui/user_interface.h"
#include "opt3001/i2cOptDriver.h" // i2OptDriver must go before opt3001
#include "opt3001/opt3001.h"
#include "tmp107/tmp107.h"
#include "tmp107/hal.h"
#include "bmi160/bmi160.h"

// Initialises all the sensors:
// Light, board & motor temp, currents, acceleration, and eStop interrupts
// Thresholds: Temp in int degrees C, current in mA, acceleration in m/s^2
bool initSensors(uint16_t thresholdTemp, uint16_t thresholdCurrent, uint16_t thresholdAccel);

// Read and filter light over I2C
float getLight();

// Read and filter board and motor temperature sensors over UART
float getBoardTemp();
float getMotorTemp();

// Read and filter two motor phase currents via analogue signals on the current sensors
// Just have the 'total' available for external.
float getCurrent();
float getPower();

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t getAcceleration();

// Update threshold values that trigger an eStop
void setThresholdTemp(uint16_t thresholdTemp); // Use Celsius
void setThresholdCurrent(uint16_t thresholdCurrent); // Use mA
void setThresholdAccel(uint16_t thresholdAccel); // Use m/s^2

#endif /* SENSORS_SENSOR_API_H_ */
