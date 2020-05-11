/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

// GPIO Interrupts

void hallAFxn(unsigned int index) {
    System_printf("Hall A Interrupt fired!\n");
    System_flush();
}

void hallBFxn(unsigned int index) {
    System_printf("Hall B Interrupt fired!\n");
    System_flush();
}

void hallCFxn(unsigned int index) {
    System_printf("Hall C Interrupt fired!\n");
    System_flush();
}

bool initMotor() {
    int return_val;
    Error_Block *eb;

    return_val = initMotorLib(50, eb);

    enableMotor();

    // if you need to configure these pins further, call GPIO_setConfig here

    GPIO_setCallback(Board_HALLA, hallAFxn);
    GPIO_setCallback(Board_HALLB, hallBFxn);
    GPIO_setCallback(Board_HALLC, hallCFxn);

    // Enable interrupts for Hall Sensors
    GPIO_enableInt(Board_HALLA);
    GPIO_enableInt(Board_HALLB);
    GPIO_enableInt(Board_HALLC);

    if (return_val == 0) {
        System_printf("%s\n", eb->msg);
        System_flush();
    }

    return return_val;
}

void startMotor() {
//    int return_val;
    setDuty(25);
    while (1) {
//        System_printf("%d, %d, %d\n", GPIO_read(Board_HALLA), GPIO_read(Board_HALLB), GPIO_read(Board_HALLC));
//        System_flush();
//        updateMotor(GPIO_read(Board_HALLA),
//                    GPIO_read(Board_HALLB),
//                    GPIO_read(Board_HALLC));
    }
}

int setSpeed(int speed_rpm) {
    //pass

    // get current speed, find error with new speed
    // calculate new pwm

    return 0;
}
