/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

#define PWM 50.0
#define SYSTICK_PERIOD_MIN (10.0 / 60000.0)

float rotations = 0;
float speed_rpm = 0;

// GPIO Interrupts

void updateSpeed() {
    speed_rpm = rotations / SYSTICK_PERIOD_MIN;
    rotations = 0;
}

void checkSpeed() {
    updateSpeed();
}

void inc_rotations() {
    rotations += 1.0 / 6.0;
//    System_printf("%f\n", speed_rpm);
//    System_flush();
}

void motorUpdateFunc() {
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

void callbackFxn(unsigned int index) {
    motorUpdateFunc();
}

void rotationCallbackFxn(unsigned int index) {
    inc_rotations();
    motorUpdateFunc();
}

bool initMotor() {
    int return_val;
    Error_Block *eb;

    return_val = initMotorLib(PWM, eb);

    GPIO_setConfig(Board_HALLA, GPIO_CFG_INPUT | GPIO_CFG_IN_INT_BOTH_EDGES);
    GPIO_setConfig(Board_HALLB, GPIO_CFG_INPUT | GPIO_CFG_IN_INT_BOTH_EDGES);
    GPIO_setConfig(Board_HALLC, GPIO_CFG_INPUT | GPIO_CFG_IN_INT_BOTH_EDGES);

    GPIO_setCallback(Board_HALLA, rotationCallbackFxn);
    GPIO_setCallback(Board_HALLB, rotationCallbackFxn);
    GPIO_setCallback(Board_HALLC, rotationCallbackFxn);

    // Enable interrupts for Hall Sensors
    GPIO_enableInt(Board_HALLA);
    GPIO_enableInt(Board_HALLB);
    GPIO_enableInt(Board_HALLC);

    // set up a swi for 100kHz

    if (return_val == 0) {
        System_printf("%s\n", eb->msg);
        System_flush();
    }

    return return_val;
}

void startMotor(int duty_pct) {
//  int return_val;
    enableMotor();
    setSpeed(duty_pct);
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

void stopMotor_api() {
    stopMotor(true);
    disableMotor();
}

int setSpeed(int duty_pct) {
    float duty_ms = duty_pct * 100.0 / PWM;
    setDuty(duty_ms);
    return 0;
}
