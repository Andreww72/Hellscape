#ifndef SENSORS_BMI160_BMI160_H_
#define SENSORS_BMI160_BMI160_H_

#include <stdbool.h>
#include <stdint.h>
#include <driverlib/i2c.h>
#include <inc/hw_memmap.h>
#include "../sensor_ports.h"

bool sensorBmi160Init();
bool bmi160_writeI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length);
bool bmi160_readI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length);

#endif /* SENSORS_BMI160_BMI160_H_ */