#include "bmi160.h"

bool sensorBmi160Init(I2C_Handle i2c) {
    uint8_t conf;

    // BMI160_ACC_CONF_REG -> Page 57 of BMI160
    conf = 0b00101001; // Date rate 200Hz
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_ACC_CONF_REG, &conf);

    // BMI160_ACC_RANGE_REG -> Page 58
    conf = 0b101; // \pm 4g range (high bits not used)
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_ACC_RANGE_REG, &conf);


    //BMI160_GYR_CONF_REG -> Page 59
    conf = 0b0011001; // (3db cutoff freq), 200Hz output
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_GYR_CONF_REG, &conf);

    // BMI160_GYR_RANGE_REG -> Page 60
    conf = 0b100; //\pm 100\deg/s
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_GYR_RANGE_REG, &conf);

    // The following are all interrupt settings
    // Interrupt for high g -> will happen in a crash
    // conf = 0b111; //high g[z,y,x] -> interrupt no longer required
    //bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_EN_1_REG, &conf, 1);
    //Actually just disable interrupts

    conf = 0;
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_EN_0_REG, &conf);

    conf = 0;
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_EN_1_REG, &conf);
    return true;
}

bool sensorBmi160GetAccelData(I2C_Handle i2c, struct accel_data *data){
    uint8_t data_arr[9] = {0};

    BMI160ReadI2C(i2c, BMI160_I2C_ADDR, BMI160_DATA_ACC_REG, data_arr, 6);

    // Has to be done this way as it's stored in lsb msb order
    data->x = data_arr[1]<<8 | data_arr[0];
    data->y = data_arr[3]<<8 | data_arr[2];
    data->z = data_arr[5]<<8 | data_arr[4];

    return true;
}

bool sensorBmi160GetGyrData(I2C_Handle i2c, struct gyr_data *data){
    uint8_t data_arr[9] = {0};

    // read x,x y,y z,z (6 uint8_t)
    BMI160ReadI2C(i2c, BMI160_I2C_ADDR, BMI160_DATA_GYR_REG, data_arr, 6);

    // Has to be done this way as it's stored in lsb msb order
    data->x = data_arr[1]<<8 | data_arr[0];
    data->y = data_arr[3]<<8 | data_arr[2];
    data->z = data_arr[5]<<8 | data_arr[4];

    return true;
}

bool BMI160WriteI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuf[2];
    txBuf[0] = ui8Reg;
    txBuf[1] = data[0];

    i2cTransaction.slaveAddress = ui8Addr;
    i2cTransaction.writeBuf = txBuf;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
    return true;
}

bool BMI160ReadI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data, uint8_t rCount) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuf[1];
    txBuf[0] = ui8Reg;

    i2cTransaction.slaveAddress = ui8Addr;
    i2cTransaction.writeBuf = txBuf;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = data;
    i2cTransaction.readCount = rCount;

    I2C_transfer(i2c, &i2cTransaction);
    return true;
}
