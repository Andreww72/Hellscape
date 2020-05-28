#ifndef I2COPTDRIVER_H_
#define I2COPTDRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <xdc/runtime/System.h>
#include <ti/drivers/I2C.h>
#include "../sensor_api.h"

extern bool writeI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);
extern bool readI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data);

#endif /* I2COPTDRIVER_H_ */
