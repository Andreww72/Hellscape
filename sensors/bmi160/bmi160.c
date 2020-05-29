/*
 * bmi160.c
 *
 *  Created on: 23 May 2020
 *      Author: Tristan
 */

#include "bmi160.h"



bool sensorBmi160Init() {

    uint8_t conf;

    // BMI160_ACC_CONF_REG -> Page 57 of BMI160
    conf = 0b00101001; // Date rate 200Hz
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_ACC_CONF_REG, &conf, 1);

    // BMI160_ACC_RANGE_REG -> Page 58
    conf = 0b101; // \pm 4g range (high bits not used)
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_ACC_RANGE_REG, &conf, 1);


    //BMI160_GYR_CONF_REG -> Page 59
    conf = 0b0011001; // (3db cutoff freq), 200Hz output
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_GYR_CONF_REG, &conf, 1);

    // BMI160_GYR_RANGE_REG -> Page 60
    conf = 0b100; //\pm 100\deg/s
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_GYR_RANGE_REG, &conf, 1);

    // The following are all interrupt settings
    // Interrupt for high g -> will happen in a crash
    // conf = 0b111; //high g[z,y,x] -> interrupt no longer required
    //bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_EN_1_REG, &conf, 1);

    //Actually just disable interrupts
    conf = 0;
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_EN_0_REG, &conf, 1);

    conf = 0;
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_EN_1_REG, &conf, 1);

    // Configure interrupt behaviour
    // Page 65 INT_OUT_CTRL
    conf = 0b1100; // Enable interrupt for pin1
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_OUT_CTRL_REG, &conf, 1);

    // Page 66 INT_LATCH -> higher pins are reserved, gotta keep them
    bmi160_readI2C(BMI160_I2C_ADDR, BMI160_INT_LATCH_REG, &conf, 1);
    conf |= 0b1111; //Latched mode
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_LATCH_REG, &conf, 1);


    // Page 67 INT_MAP
    // Set interrupt high-g to output int pin 1
    conf = 0b10;
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_MAP_0_REG, &conf, 1);

    // Page 68 INT_DATA
    bmi160_readI2C(BMI160_I2C_ADDR, BMI160_INT_DATA_0_REG, &conf, 1);
    // Select filtered data (bit 7 set to 0)
    conf &= ~(1<<7);
    bmi160_writeI2C(BMI160_I2C_ADDR, BMI160_INT_DATA_0_REG, &conf, 1);

    // config_high_g_int_settg -> I have literally no clue what this is supposed to be set to


    // TODO: Check BMI160_ERR_REG
    return true;
}

// write length long data to given addresses
bool bmi160_writeI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length){

    // TODO: Unsure if I2C0_BASE is correct
    I2CMasterSlaveAddrSet(BMI160_I2C_BASE, device_address, false);

    I2CMasterDataPut(BMI160_I2C_BASE, register_address);
    I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(BMI160_I2C_BASE)) { }

    uint8_t i=0;
    for(;i < length; i++){
        I2CMasterDataPut(BMI160_I2C_BASE, data[0]);
        I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(BMI160_I2C_BASE)) { }
    }

    I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    // Delay until transmission completes
    while(I2CMasterBusBusy(BMI160_I2C_BASE)) { }

    return true;
}

bool bmi160_readI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length){

    I2CMasterSlaveAddrSet(BMI160_I2C_BASE, device_address, false);

    I2CMasterDataPut(BMI160_I2C_BASE, register_address);
    I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(BMI160_I2C_BASE)) { }

    // Set to read mode
    I2CMasterSlaveAddrSet(BMI160_I2C_BASE, device_address, true);

    I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(BMI160_I2C_BASE)) { }
    data[0] = I2CMasterDataGet(BMI160_I2C_BASE);

    uint8_t i=1;
    while (i<length){
        I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(BMI160_I2C_BASE)) { }
        data[i++] = I2CMasterDataGet(BMI160_I2C_BASE);
    }

    I2CMasterControl(BMI160_I2C_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(BMI160_I2C_BASE)) { }
    I2CMasterDataGet(BMI160_I2C_BASE); // Flush I2C, ignore result

    return true;

}

bool sensorBmi160GetAccelData(struct accel_data *data){
    uint8_t data_arr[9] = {0};



    bmi160_readI2C(BMI160_I2C_ADDR, BMI160_DATA_ACC_REG, data_arr, 6);

    // Has to be done this way as it's stored in lsb msb order
    data->x = data_arr[1]<<8 | data_arr[0];
    data->y = data_arr[3]<<8 | data_arr[2];
    data->z = data_arr[5]<<8 | data_arr[4];

    return true;
}

bool sensorBmi160GetGyrData(struct gyr_data *data){
    uint8_t data_arr[9] = {0};


    // read x,x y,y z,z (6 uint8_t)
    bmi160_readI2C(BMI160_I2C_ADDR, BMI160_DATA_GYR_REG, data_arr, 6);

    // Has to be done this way as it's stored in lsb msb order
    data->x = data_arr[1]<<8 | data_arr[0];
    data->y = data_arr[3]<<8 | data_arr[2];
    data->z = data_arr[5]<<8 | data_arr[4];

    return true;
}







