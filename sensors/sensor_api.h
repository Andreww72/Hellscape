#ifndef SENSORS_SENSOR_API_H_
#define SENSORS_SENSOR_API_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

#include "motor/motor_api.h"
#include "opt3001/i2cOptDriver.h" // i2OptDriver must go before opt3001
#include "opt3001/opt3001.h"
#include "tmp107/tmp107.h"
#include "tmp107/hal.h"

#include "bmi160/bmi160.h"

// Initialises all the sensors:
// Light, board temp, motor temp, currents, acceleration (and interrupt)
bool initSensors(uint16_t threshTemp, uint16_t threshCurrent, uint16_t thresholdAccel);

// Read and filter light over I2C
float getLight();

// Read and filter board and motor temperature sensors over UART
uint8_t getBoardTemp();
uint8_t getMotorTemp();

// Read and filter two motor phase currents via analogue signals on the current sensors
// Just have the 'total' available for external.
float getCurrent();

// Read and filter acceleration on all three axes
// Calculate avg abs acceleration
uint8_t getAcceleration();

// Update threshold values that trigger an eStop
void setThresholdTemp(uint8_t threshTemp);
void setThresholdCurrent(uint16_t threshCurrent);
void setThresholdAccel(uint16_t threshAccel);

#endif /* SENSORS_SENSOR_API_H_ */
