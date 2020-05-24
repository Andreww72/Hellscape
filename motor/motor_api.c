/*
 * motor_api.c
 *
 *  Created on: 3 May 2020
 *      Author: jshea
 */

#include "motor_api.h"

#define PWM 50.0
#define SYSTICK_PER_MIN 6000.0
#define PHASES_PER_ROTATION 24
#define BUFFER_SIZE 5
#define MIN_RPM 200
#define MAX_CUM_ERROR 9000
#define ACCEL_PER_TICK 3
#define DECCEL_PER_TICK 30
#define ESTOP_DECCEL_PER_TICK 40
#define Kp 0.02   // DO NOT TOUCH
#define Ki 0.0001 // DO NOT TOUCH

GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms;

int rotations = 0;
int speed_rpm = 0;
int desired_speed_rpm = 0;
int accel_speed = 0;
int cum_speed_error = 0;
int rpm_buffer[BUFFER_SIZE];
int rpm_index = 0;

int printed = 0;
int executed = 0;

bool motor_on = false;
bool estop = false;

// utility functions
void motorUpdateFunc();
void rotationCallbackFxn(unsigned int index);
void setSpeed();

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

    // check the return value from initMotorLib();
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
    stopMotor(false);
    setDesiredSpeed(rpm);
    motor_on = true;
    estop = false;
    motorUpdateFunc();
}

void eStopMotor() {
    setDesiredSpeed(0); // stop motor
    estop = true; // set estop flag
    motor_on = false; // turn off motor
}

void stopMotor_api() {
    setDesiredSpeed(0);
}

void setDesiredSpeed(int rpm) {
    UInt key;
    key = GateHwi_enter(gateHwi);
    desired_speed_rpm = rpm;
    GateHwi_leave(gateHwi, key);
}

int getSpeed() {
    int return_val;
    UInt key;
    key = GateHwi_enter(gateHwi);
    return_val = speed_rpm;
    GateHwi_leave(gateHwi, key);
}

void setSpeed() {
    int error;
    int duty;

    UInt key;
    key = GateHwi_enter(gateHwi);
    // calculate PWM duty cycle
    error = accel_speed - speed_rpm;
    GateHwi_leave(gateHwi, key);
    cum_speed_error += error;

    // Calculate the duty cycle
    duty = Kp*error + Ki*cum_speed_error;

    setDuty(duty);
}

// Function to check & filter the speed as per the clock
void checkSpeed() {
    UInt key;
    key = GateHwi_enter(gateHwi);

    // If we're trying to accelerate, give the motor another lil push
    // Courtesy of friction, sometimes the bastard just won't start
    if (!speed_rpm) {
        motorUpdateFunc();
    }

    // estop
    if (estop) {
        if (accel_speed > desired_speed_rpm) {
            accel_speed -= ESTOP_DECCEL_PER_TICK;
            if (accel_speed < MIN_RPM) {
                accel_speed = 0;
                stopMotor(true);
            }
        }
    }

    // Accelerate/deccelerate - I have genuinely no idea what the heck is going on here
    if (motor_on) {
        if (accel_speed < desired_speed_rpm) {
            accel_speed += ACCEL_PER_TICK;
        } else if (accel_speed > desired_speed_rpm) {
            accel_speed -= DECCEL_PER_TICK;
        }
    }

    // Add this measurement to buffer
    rpm_buffer[rpm_index] = rotations * (SYSTICK_PER_MIN / PHASES_PER_ROTATION);

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

// Function to update the motor commutation
void motorUpdateFunc() {
    setSpeed();
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

// Callback function to rotate the motor
void rotationCallbackFxn(unsigned int index) {
    UInt key;
    key = GateHwi_enter(gateHwi);
    rotations++;
    GateHwi_leave(gateHwi, key);
    motorUpdateFunc();
}
