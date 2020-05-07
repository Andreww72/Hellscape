/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

bool initMotor() {
    bool return_val = false;
    Error_Block *eb;

    return_val = initMotorLib(50, eb);
    enableMotor();

    return return_val;
}

int setSpeed(int speed_rpm) {
    //pass

    return 0;
}
