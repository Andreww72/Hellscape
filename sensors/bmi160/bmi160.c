/*
 * bmi160.c
 *
 *  Created on: 23 May 2020
 *      Author: Tristan
 */

#include "bmi160.h"



bool sensorBmi160Init() {
    return true;

    // id = BMI160_I2C_ADDR

//    bmi160_writeI2C(BMI_I2C_ADDR, )



}

// write length long data to given addresses
bool bmi160_writeI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length){

    // TODO: Unsure if I2C0_BASE is correct
    IC2MasterSlaveAddrSet(IC20_BASE, device_address, false);

    I2CMasterDataPut(I2C0_BASE, register_address);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C0_BASE)) { }

    uint8_t i=0;
    for(;i < length; i++){
        I2CMasterDataPut(I2C0_BASE, data[0]);
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(I2C0_BASE)) { }
    }

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);

    // Delay until transmission completes
    while(I2CMasterBusBusy(I2C0_BASE)) { }

    return true;
}

bool bmi160_readI2C(uint8_t device_address, uint8_t register_address, uint8_t* data, uint8_t length){

    IC2MasterSlaveAddrSet(IC20_BASE, device_address, false);

    I2CMasterDataPut(I2C0_BASE, register_address);
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C0_BASE)) { }

    // Set to read mode
    I2CMasterSlaveAddrSet(I2C0_BASE, device_address, true);

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C0_BASE)) { }
    data[0] = I2CMasterDataGet(I2C0_BASE);

    uint8_t i=1;
    while (i<length){
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(I2C0_BASE)) { }
        data[i++] = I2CMasterDataGet(I2C0_Base);
    }

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C0_BASE)) { }
    I2CMasterDataGet(I2C0_BASE); // Flush I2C, ignore result

    return true;

}






