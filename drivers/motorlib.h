/*
 * motordriver_lib.h
 *
 *  Created on: 20 May 2019
 *      Author: lehnert
 */

#ifndef MOTORDRIVER_LIB_H_
#define MOTORDRIVER_LIB_H_

//Definition of bit patterns for phases assuming order of Hall sensors is
// (HALL_A << 2) | (HALL_B << 1) | (HALL_C << 0)

#define PHASE_1 0b001
#define PHASE_2 0b101
#define PHASE_3 0b100
#define PHASE_4 0b110
#define PHASE_5 0b010
#define PHASE_6 0b011

//Need to ensure these are defined

#include <ti/drivers/pwm/PWMTimerTiva.h>
#include <stdbool.h>
#include <xdc/runtime/Error.h>


typedef enum MOTORLIB_PWMName {
    MOTORLIB_PWM0 = 0,
    HS_A_PWM,
    LS_A_PWM,
    HS_B_PWM,
    LS_B_PWM,
    HS_C_PWM, //TIMER PWM CCP0
    LS_C_PWM, //TIMER PWM CCP1

   MOTORLIB_PWMCOUNT
} MOTORLIB_PWMName;

#define MOTORLIB_PWMTIMERCOUNT 2

//#define HS_A_PWM EK_TM4C1294XL_PWM1
//#define LS_A_PWM EK_TM4C1294XL_PWM2
//#define HS_B_PWM EK_TM4C1294XL_PWM3
//#define LS_B_PWM EK_TM4C1294XL_PWM4
//#define HS_C_PWM EK_TM4C1294XL_CCP0
//#define LS_C_PWM EK_TM4C1294XL_CCP1

typedef struct{
    PWM_Handle hHS_A;
    PWM_Handle hLS_A;
    PWM_Handle hHS_B;
    PWM_Handle hLS_B;
    PWM_Handle hHS_C;
    PWM_Handle hLS_C;
    uint16_t Period;
    uint16_t MaxDuty;
    uint16_t duty;
} PWMStruct_t;

static volatile PWMStruct_t PWMStruct;

//Function Declarations
//uint8_t getPhase(); //Returns phase in binary format 0bXXX representing Hall effect sensor values


//Sets the duty cycle of the pwm
//Valid values for duty are 0 - PWMStruct.MaxDuty
//MaxDuty = Period in microseconds of the PWM
void setDuty(uint16_t duty);

//Main function for driving motor
//commutates the motor phases A,B,C to the correct values based on the current phase
void updateMotor(bool Hall_a, bool Hall_b, bool Hall_c);

//Brakes motor by turning all phases high or low
//If brakeType = True then all phases set to high
//If brakeType = False then all phases set to low

void stopMotor(bool brakeType);

//Sets Motor Drive Enable pin to low
void enableMotor();

//Sets Motor Drive Enable pin to High
void disableMotor();

//Initialise PWM module to ensure correct setup of High/Low side pins
//pwm_duty_period is in microseconds and should be set to be a value between 10 - 100 (100KHz - 10Khz)
int initMotorLib(int pwm_duty_period, Error_Block *eb);

//Seperate function for setting up PWM modules
void initMotorLibPWM();

//getter function for internal PWM period
//use to check max duty cycle value
int getMotorLibPWMPeriod();

#endif /* MOTORDRIVER_LIB_H_ */
