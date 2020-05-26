#ifndef OPT3001_H
#define OPT3001_H

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <xdc/runtime/System.h>
#include <ti/drivers/I2C.h>
#include "i2cOptDriver.h"
#include "../sensor_ports.h"

extern bool sensorOpt3001Init(I2C_Handle i2c);
extern void sensorOpt3001Enable(I2C_Handle i2c, bool enable);
extern bool sensorOpt3001Read(I2C_Handle i2c, uint16_t *rawData);
extern void sensorOpt3001Convert(uint16_t rawData, float *convertedLux);
extern bool sensorOpt3001Test(I2C_Handle i2c);

#endif /* OPT3001_H */
