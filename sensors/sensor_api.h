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

// All the port macros are defined here
#include "opt3001/opt3001.h"
#include "opt3001/i2cOptDriver.h"
#include "tmp107/tmp107.h"
#include "tmp107/hal.h"

// Current sensor constants
#define ADC_SEQB 1
#define ADC_SEQC 2
#define ADC_PRI 0
#define ADC_STEP 0
#define ADC_V_REF 3.3 // Page 1862 says ADC ref voltage 3.3V
#define ADC_RESISTANCE 0.007 // FAQ
#define ADC_GAIN 10.0 // FAQ
#define ADC_RESOLUTION 4095.0 // Page 1861 says resolution 12 bits

#define TMP_RESOLUTION 0.015625
#define CURR_CHECK_TICKS 250

// Data window size constants
#define windowLight 5
#define windowTemp 3
#define windowCurrent 5
#define windowAcceleration 5

// Initialises all the sensors:
// Light, board temp, motor temp, currents, acceleration (and interrupt)
bool initSensors(uint8_t thresholdTemp, uint16_t thresholdCurrent, uint16_t thresholdAccel);

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
