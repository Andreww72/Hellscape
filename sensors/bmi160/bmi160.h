#ifndef SENSORS_BMI160_BMI160_H_
#define SENSORS_BMI160_BMI160_H_

#include <stdbool.h>
#include <stdint.h>
#include <driverlib/i2c.h>
#include <inc/hw_memmap.h>
#include "../sensor_ports.h"

// TODO: Since these are exactly the same, should I just use a sensordata stuct??
struct accel_data {
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct gyr_data{
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

bool sensorBmi160Init();
bool bmi160_writeI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length);
bool bmi160_readI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length);
bool sensorBmi160GetAccelData(struct accel_data *data);
bool sensorBmi160GetGyrData(struct gyr_data *data);

#endif /* SENSORS_BMI160_BMI160_H_ */
