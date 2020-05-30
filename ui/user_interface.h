#ifndef UI_USER_INTERFACE_H_
#define UI_USER_INTERFACE_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <constants.h>
#include "grlib/grlib.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <inc/hw_ints.h>
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"
#include "drivers/pinout.h"
#include "drivers/frame.h"
#include "drivers/touch.h"
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/System.h>
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"

#include "../sensors/sensor_api.h"
#include "../motor/motor_api.h"

uint32_t motorTemperatureLimit;
uint32_t motorSpeedLimit;
uint32_t motorCurrentLimit;
uint32_t motorAccelerationLimit;

uint16_t drawingGraph;

enum page {
    temperature = 0,
    motor = 1,
    current = 2,
    acceleration = 3,
};

enum graphPage {
    powerGraph = 0,
    ambTempGraph = 1,
    speedGraph = 2,
    accelerationGraph = 3,
    motorTempGraph = 4,
    lightGraph = 5
};

// Keep track of which page we're on
int settingsPageIdentifier;
int graphPageIdentifier;

uint16_t s_x;
uint16_t s_y;
uint16_t width;
uint16_t height;

void drawDayNight(bool isDay);

// EEPROM
void writeToEEPROM();

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
void eStopGUI();

static void setupGraphScreen(char * title, int yMin, int yMax);
static void DrawDataOnGraph(float lastSample);
static void returnFromGraph();
static void drawPowerGraph();
static void drawAmbientTemperatureGraph();
static void drawSpeedGraph();
static void drawAccelerationGraph();
static void drawMotorTemperatureGraph();
static void drawLightGraph();

// External graph api
void AddValueToGraph(uint32_t lastSample, int graphPage);

void UserInterfaceInit(uint32_t systemclock, tContext * sContext);
void UserInterfaceDraw(tContext * sContext);

#endif /* UI_USER_INTERFACE_H_ */
