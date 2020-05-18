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

uint16_t drawingGraph;

enum page {
    temperature = 0,
    motor = 1,
    current = 2,
    acceleration = 3,
};

struct E2PROM_SETTINGS
{
    uint64_t temperatureLimit;
    uint64_t motorSpeed;
    uint64_t currentLimit;
    uint64_t accelerationLimit;
};

uint16_t s_x;
uint16_t s_y;
uint16_t width;
uint16_t height;

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

static void setupGraphScreen();
static void DrawDataOnGraph(int yMin, int yMax, uint16_t last_sample);
static void returnFromGraph();
static void drawPowerGraph();
static void drawAmbientTemperatureGraph();
static void drawSpeedGraph();
static void drawAccelerationGraph();
static void drawMotorTemperatureGraph();
static void drawLightGraph();

void UserInterfaceInit(uint32_t systemclock, tContext * sContext);
void UserInterfaceDraw(tContext * sContext);

#endif /* UI_USER_INTERFACE_H_ */
