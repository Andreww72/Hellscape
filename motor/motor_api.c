/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

#define PWM 50.0
#define MAX_DUTY_MS (PWM / 2.0)
#define SYSTICK_PERIOD_MIN (10.0 / 60000.0)
#define BUFFER_SIZE 10
#define Kp 1.0
#define Ki 1.0

GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;

float rotations = 0;
float speed_rpm = 0;
int cum_speed_error = 0;
int executed = 0;
int rpm_buffer[BUFFER_SIZE];
int rpm_index = 0;

void checkSpeed() {
    executed++;
    if (executed == 300) {
        System_printf("%f\n", speed_rpm);
        System_flush();
        executed = 0;
    }
    UInt key;
    key = GateHwi_enter(gateHwi);

    // Add this measurement to buffer
    rpm_buffer[rpm_index] = rotations / SYSTICK_PERIOD_MIN;

    rotations = 0; // reset rotations
    rpm_index++; // increment index
    if (rpm_index >= BUFFER_SIZE) { // Reset index counter as necessary
        rpm_index = 0;
    }

    // Calculate speed for this tick
    int i;
    int tot_speed = 0;
    for (i = 0; i < BUFFER_SIZE; i++) {
        tot_speed += rpm_buffer[i];
    }
    speed_rpm = tot_speed / BUFFER_SIZE;

    GateHwi_leave(gateHwi, key);
}

void motorUpdateFunc() {
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

void rotationCallbackFxn(unsigned int index) {
    UInt key;
    key = GateHwi_enter(gateHwi);
    rotations += 1.0 / 6.0;
    GateHwi_leave(gateHwi, key);
    motorUpdateFunc();
}

bool initMotor() {
    int return_val;
    Error_Block *eb;

    return_val = initMotorLib(PWM, eb);

    GateHwi_Params_init(&gHwiprms);
    gateHwi = GateHwi_create(&gHwiprms,NULL);
    if (gateHwi == NULL) {
        System_abort("Gate Hwi create has failed");
    }

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

    // initialise the speed buffer
    int i;
    for (i = 0; i < BUFFER_SIZE; i++) {
        rpm_buffer[i] = 0;
    }

    return return_val;
}

void startMotor(int rpm) {
    enableMotor();
    setSpeed(rpm);
    motorUpdateFunc();
}

void eStopMotor() {
    //pass
}

void stopMotor_api() {
    stopMotor(true);
    disableMotor();
}

int setSpeed(int rpm_in) {
    setDuty(25);
    return 0;
}
