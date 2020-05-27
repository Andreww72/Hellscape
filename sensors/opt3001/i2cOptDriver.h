#ifndef I2COPTDRIVER_H_
#define I2COPTDRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <xdc/runtime/System.h>
#include <ti/drivers/I2C.h>

extern bool writeI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);
extern bool readI2C(I2C_Handle i2c, uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);

#endif /* I2COPTDRIVER_H_ */
