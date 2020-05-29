#ifndef SENSORS_BMI160_BMI160_H_
#define SENSORS_BMI160_BMI160_H_

#include <stdbool.h>
#include <stdint.h>
#include <ti/drivers/I2C.h>
#include <inc/hw_memmap.h>
#include "../sensor_ports.h"

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

bool sensorBmi160Init(I2C_Handle i2c);
bool sensorBmi160GetAccelData(I2C_Handle i2c, struct accel_data *data);
bool sensorBmi160GetGyrData(I2C_Handle i2c, struct gyr_data *data);
bool BMI160WriteI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data);
bool BMI160ReadI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint8_t rCount);

#endif /* SENSORS_BMI160_BMI160_H_ */
