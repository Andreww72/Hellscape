#include "bmi160.h"

// BMI160 addresses (All but the first will not changed based on board position)
#define BMI160_I2C_ADDR                 0x69
#define BMI160_CMD_REG                  0x7E
#define BMI160_ERR_REG                  0x02
#define BMI160_PMU_REG                  0x03
#define BMI160_ACC_CONF_REG             0x40
#define BMI160_ACC_RANGE_REG            0x41
#define BMI160_GYR_CONF_REG             0x42
#define BMI160_GYR_RANGE_REG            0x43
#define BMI160_INT_EN_0_REG             0x50
#define BMI160_INT_EN_1_REG             0x51
#define BMI160_INT_EN_2_REG             0x52
#define BMI160_INT_OUT_CTRL_REG         0x53
#define BMI160_INT_LATCH_REG            0x54
#define BMI160_INT_MAP_0_REG            0x55
#define BMI160_INT_DATA_0_REG           0x58
#define BMI160_INT_DELAY_REG            0x5D
#define BMI160_INT_THRESHOLD_REG        0x5E

// Data from lsb to msb, x,y,z, mag, rhall,gyr,acc
#define BMI160_DATA_REG                 0x04
#define BMI160_DATA_GYR_REG             0x0C
#define BMI160_DATA_ACC_REG             0x12

#define MAX_16_INT                      65536
#define BMI160_RESOLUTION               8192.0
#define BMI160_G_TO_MPS                 10.2
#define BMI160_mG_PER_BIT               0.01563

bool BMI160Init(I2C_Handle i2c) {
    uint8_t conf;

    // BMI160_ACC_CONF_REG -> Page 83 of BMI160
    conf = 0b00010001; // Start up in normal mode
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_CMD_REG, &conf);
    Task_sleep(5);

    // BMI160_ACC_CONF_REG -> Page 57 of BMI160
    conf = 0b00101001; // Data rate 200Hz
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_ACC_CONF_REG, &conf);

    // BMI160_ACC_RANGE_REG -> Page 58
    conf = 0b0101; // \pm 4g range (high bits not used)
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_ACC_RANGE_REG, &conf);

    // The following are all interrupt settings
    // Interrupt for high g -> will happen in a crash
    // INT_EN (0x50-0x52)
    conf = 0b111;
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_EN_1_REG, &conf);

    // INT_OUT_CTRL (0x53)
    conf = 0b1111; // Not sure on last bit
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_OUT_CTRL_REG, &conf);

    // INT_LATCH (0x54)
    conf = 0b11010;
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_LATCH_REG, &conf);

    // INT_MAP (0x55-0x57)
    conf = 0b10;
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_MAP_0_REG, &conf);

    // INT_LOWHIGH (0x5A-0x5E)
    conf = 1; // High-g delay time (30ms default) 2.5ms
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_DELAY_REG, &conf);
    conf = 255; // High-g threshold (15.63mG per bit)
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_THRESHOLD_REG, &conf);

    // INT_STATUS REG (0x1C-0x1F) holds triggered status (must clear if in latched)
    return true;
}

void BMI160InterruptThreshold(I2C_Handle i2c, uint8_t threshold) {
    uint8_t bitThreshold = (float)threshold / (BMI160_mG_PER_BIT * BMI160_G_TO_MPS);
    BMI160WriteI2C(i2c, BMI160_I2C_ADDR, BMI160_INT_THRESHOLD_REG, &bitThreshold);
}

bool BMI160GetAccelData(I2C_Handle i2c, struct accel_data *data){
    uint8_t data_arr[6] = {0};

    BMI160ReadI2C(i2c, BMI160_I2C_ADDR, BMI160_DATA_ACC_REG, data_arr, 6);

    // Combine separate 8 bit data into 16 bit (stored in lsb msb order)
    uint16_t xuint = (uint16_t)(data_arr[1]<<8 | data_arr[0]);
    uint16_t yuint = (uint16_t)(data_arr[3]<<8 | data_arr[2]);
    uint16_t zuint = (uint16_t)(data_arr[5]<<8 | data_arr[4]);

    // Convert two complement unsigned form to signed
    int xint = xuint > (MAX_16_INT/2) ? xuint-MAX_16_INT : xuint;
    int yint = yuint > (MAX_16_INT/2) ? yuint-MAX_16_INT : yuint;
    int zint = zuint > (MAX_16_INT/2) ? zuint-MAX_16_INT : zuint;

    // Get Gs by dividing by resolution and m/s^2 by mulitplying by 9.8
    data->x = (float)xint / BMI160_RESOLUTION * BMI160_G_TO_MPS;
    data->y = (float)yint / BMI160_RESOLUTION * BMI160_G_TO_MPS;
    data->z = (float)zint / BMI160_RESOLUTION * BMI160_G_TO_MPS;

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
