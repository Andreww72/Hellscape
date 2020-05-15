/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

#define PWM 50

// GPIO Interrupts

void callbackFxn(unsigned int index) {
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

bool initMotor() {
    int return_val;
    Error_Block *eb;

    return_val = initMotorLib(PWM, eb);

    GPIO_setCallback(Board_HALLA, callbackFxn);
    GPIO_setCallback(Board_HALLB, callbackFxn);
    GPIO_setCallback(Board_HALLC, callbackFxn);

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

void startMotor(int duty_pct) {
    float duty_ms = (duty_pct / 100.0) * PWM;

//    int return_val;
    enableMotor();
    setDuty(duty_ms);
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

void stopMotor_api() {
    stopMotor(true);
    disableMotor();
}

int setSpeed(int duty_pct) {
    //pass

    // get current speed, find error with new speed
    // calculate new pwm

    return 0;
}
