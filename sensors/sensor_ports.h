#ifndef SENSORS_SENSOR_PORTS_H_
#define SENSORS_SENSOR_PORTS_H_

/* Slave address */
#define OPT3001_I2C_ADDRESS             0x47

/* Register addresses for opt3001 */
#define REG_RESULT                      0x00
#define REG_CONFIGURATION               0x01
#define REG_LOW_LIMIT                   0x02
#define REG_HIGH_LIMIT                  0x03

// BMI160 addresses (All but the first will not changed based on board position)
#define BMI160_I2C_ADDR                 0x68
#define BMI160_ERR_REG                  0x02
#define BMI160_ACC_CONF_REG             0x40
#define BMI160_ACC_RANGE_REG            0x41
#define BMI160_GYR_CONF_REG             0x42
#define BMI160_GYR_RANGE_REG            0x43

#endif /* SENSORS_SENSOR_PORTS_H_ */
