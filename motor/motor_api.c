/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

bool initMotor() {
    int return_val;
    Error_Block *eb;

    return_val = initMotorLib(50, eb);
    enableMotor();

    if (return_val == 0) {
        System_printf("%s\n", eb->msg);
        System_flush();
    }

    return return_val;
}

int setSpeed(int speed_rpm) {
    //pass

    return 0;
}
