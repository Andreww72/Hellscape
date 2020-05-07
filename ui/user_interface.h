/*
 * user_interface.h
 *
 *  Created on: May 1, 2020
 *      Author: Lachlan
 */

#ifndef UI_USER_INTERFACE_H_
#define UI_USER_INTERFACE_H_

#include <stdint.h>
#include <stdbool.h>
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"

int motorTemperatureLimit;
int motorSpeedLimit;
int motorCurrentLimit;
int motorAccelerationLimit;

enum page {
    temperature = 0,
    motor = 1,
    current = 2,
    acceleration = 3,
};

static char * getCurrentDateTime();
void DrawDateTime();
static void DrawHomeScreen();
static void DrawSetMenuScreen();
static void DrawGraphMenuScreen();
static void DrawSettingsPage();

static void DrawTemperatureSettingsPage();
static void DrawMotorSettingsPage();
static void DrawCurrentSettingsPage();
static void DrawAccelerationSettingsPage();

static void increaseSetting();
static void decreaseSetting();

static void StartStopMotor();

static void SetupGraphScreen();

void UserInterfaceInit(uint32_t systemclock, tContext * sContext);
void UserInterfaceDraw(tContext * sContext);

#endif /* UI_USER_INTERFACE_H_ */
