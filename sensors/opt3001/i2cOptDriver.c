#include "i2cOptDriver.h"

/*
 * Sets slave address to ui8Addr
 * Puts ui8Reg followed by two data bytes in *data and transfers
 * over i2c
 */
bool writeI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data)
{
    I2C_Transaction i2cTransaction;
    uint8_t txBuf[3];
    txBuf[0] = ui8Reg;
    txBuf[1] = data[0];
    txBuf[2] = data[1];

    i2cTransaction.slaveAddress = ui8Addr;
    i2cTransaction.writeBuf = txBuf;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    I2C_transfer(i2c, &i2cTransaction);
    return true;
}


/*
 * Sets slave address to ui8Addr
 * Writes ui8Reg over i2c to specify register being read from
 * Reads three bytes from i2c slave. The third is redundant but
 * helps to flush the i2c register
 * Stores first two received bytes into *data
 */
bool readI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *data) {
    I2C_Transaction i2cTransaction;
    uint8_t txBuf[1];
    uint8_t rxBuf[2];

    i2cTransaction.slaveAddress = ui8Addr;
    i2cTransaction.writeBuf = txBuf;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = rxBuf;
    i2cTransaction.readCount = 2;

    txBuf[0] = ui8Reg;
    I2C_transfer(i2c, &i2cTransaction);
    data[0] = rxBuf[0];
    data[1] = rxBuf[1];

    return true;
}
