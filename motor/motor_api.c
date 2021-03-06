// The author would like to note that the suppressions present throughout the script are due to what appears to be a CCStudio
// bug, as the code compiles and runs without issue despite this "ambiguous function" nonsense.

#include "motor_api.h"

#define PWM 50.0
#define SYSTICK_PER_MIN 6000.0
#define PHASES_PER_ROTATION 24
#define BUFFER_SIZE 6
#define MIN_RPM 200
#define MAX_CUM_ERROR 9000
#define ACCEL_PER_TICK 3
#define DECCEL_PER_TICK 6
#define ESTOP_DECCEL_PER_TICK 10
#define Kp 0.03   // DO NOT TOUCH
#define Ki 0.0004 // DO NOT TOUCH

GateHwi_Handle gateHwi;
GateHwi_Params gHwiprms; // @suppress("Ambiguous problem")

int rotations = 0;
int speed_rpm = 0;
int desired_speed_rpm = 0;
int accel_speed = 0;
int cum_speed_error = 0;
int rpm_buffer[BUFFER_SIZE];
int rpm_index = 0;
bool motor_on = false;
bool estop = false;

// utility functions
static void motorUpdateFunc();
static void rotationCallbackFxn(unsigned int index);
static void setSpeed();
void checkSpeedSwi();

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

    // Update LED to display status
    GPIO_write(Board_LED1, Board_LED_ON);
}

void eStopMotor() {
    estop = true;
    motor_on = false;

    // Update LED to display status
    GPIO_write(Board_LED1, Board_LED_OFF);
}

void stopMotor_api() {
    motor_on = false;

    // Update LED to display status
    GPIO_write(Board_LED1, Board_LED_OFF);
}

void setDesiredSpeed(int rpm) {
    UInt key;
    key = GateHwi_enter(gateHwi); // @suppress("Invalid arguments")
    desired_speed_rpm = rpm;
    GateHwi_leave(gateHwi, key); // @suppress("Invalid arguments")
}

int getSpeed() {
    int return_val;
    UInt key;
    key = GateHwi_enter(gateHwi); // @suppress("Invalid arguments")
    return_val = speed_rpm;
    GateHwi_leave(gateHwi, key); // @suppress("Invalid arguments")
    return return_val;
}

float getSpeedFloat() {
    return (float) getSpeed();
}

// Clock Swi for motor control
void checkSpeedSwi() {
    UInt key;
    key = GateHwi_enter(gateHwi); // @suppress("Invalid arguments")

    // If we're trying to accelerate, give the motor another lil push
    // Courtesy of friction, sometimes it just won't start
    if (!speed_rpm && motor_on) {
        motorUpdateFunc();
    }

    // estop
    if (estop) {
        if (accel_speed >= 0) {
            accel_speed -= ESTOP_DECCEL_PER_TICK;
            if (speed_rpm <= 0) {
                stopMotor(true);
                cum_speed_error = 0;
            }
        }
    }

    // Accelerate/decelerate
    else if (motor_on) {
        if (accel_speed < desired_speed_rpm) {
            accel_speed += ACCEL_PER_TICK;
        } else if (accel_speed >= desired_speed_rpm) {
            accel_speed -= DECCEL_PER_TICK;
            if (speed_rpm <= 0) {
                stopMotor(true);
                cum_speed_error = 0;
            }
        }
    } else {
        if (accel_speed >= 0) {
            accel_speed -= DECCEL_PER_TICK;
            if (speed_rpm <= 0) {
                stopMotor(true);
                cum_speed_error = 0;
            }
        }
    }

    // Add this measurement to buffer
    rpm_buffer[rpm_index] = rotations * (SYSTICK_PER_MIN / PHASES_PER_ROTATION);

    rotations = 0; // Reset rotations
    rpm_index++; // Increment index
    if (rpm_index >= BUFFER_SIZE) { // Reset index counter as necessary
        rpm_index = 0;
    }

    // Calculate speed for this tick (filtering for speed)
    int i;
    int tot_speed = 0;
    for (i = 0; i < BUFFER_SIZE; i++) {
        tot_speed += rpm_buffer[i];
    }
    speed_rpm = tot_speed / BUFFER_SIZE;

    GateHwi_leave(gateHwi, key); // @suppress("Invalid arguments")
}

// Function to update the motor commutation & set the duty cycle
static void motorUpdateFunc() {
    setSpeed();
    updateMotor(GPIO_read(Board_HALLA),
                GPIO_read(Board_HALLB),
                GPIO_read(Board_HALLC));
}

// Callback function to rotate the motor & count the number of rotations
static void rotationCallbackFxn(unsigned int index) {
    UInt key;
    key = GateHwi_enter(gateHwi); // @suppress("Invalid arguments")
    rotations++;
    GateHwi_leave(gateHwi, key); // @suppress("Invalid arguments")
    motorUpdateFunc();
}

// Set the duty cycle based on the current RPM requirements
static void setSpeed() {
    int error;
    int duty;

    UInt key;
    key = GateHwi_enter(gateHwi); // @suppress("Invalid arguments")
    // calculate PWM duty cycle
    error = accel_speed - speed_rpm;
    GateHwi_leave(gateHwi, key); // @suppress("Invalid arguments")
    cum_speed_error += error;

    if (cum_speed_error > 140000) {
        cum_speed_error = 140000;
    } else if (cum_speed_error < -20000) {
        cum_speed_error = -20000;
    }

    // Calculate the duty cycle
    duty = Kp*error + Ki*cum_speed_error;
    if (duty < 0) {
        duty = 0;
    }

    setDuty(duty);
}
