#ifndef SENSORS_SENSOR_PORTS_H_
#define SENSORS_SENSOR_PORTS_H_

// BMI160 addresses (All but the first will not changed based on board position)
// TODO: Use the correct base for the other configuration -> I2C0_BASE
// THis is currently for the motor config
#define BMI160_I2C_BASE                 I2C2_BASE
#define BMI160_I2C_ADDR                 0x68
#define BMI160_ERR_REG                  0x02
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

// Data from lsb to msb, x,y,z, mag, rhall,gyr,acc
#define BMI160_DATA_REG                 0x04
// We can just start at gyr
#define BMI160_DATA_GYR_REG             0x0C
#define BMI160_DATA_ACC_REG             0x12

#endif /* SENSORS_SENSOR_PORTS_H_ */
