#include "user_interface.h"
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <inc/hw_ints.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"
#include "drivers/pinout.h"
#include "drivers/frame.h"
#include "drivers/touch.h"
#include "constants.h"
#include <string.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/System.h>

// Keep track of which settings page we're on
int settingsPageIdentifier = temperature;

// GUI - Canvas Drawing
// Set/Graph Menu Selection
Canvas(g_sMenuTypePage, WIDGET_ROOT, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sSetOption, &g_sMenuTypePage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 25, 145, 150,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Settings", 0, 0, 0, 0, DrawSetMenuScreen);
RectangularButton(g_sGraphOption, &g_sMenuTypePage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 165, 25, 145, 150,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Graphs", 0, 0, 0, 0, DrawGraphMenuScreen);
RectangularButton(g_sMotorOption, &g_sMenuTypePage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 180, 300, 50,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Start Motor", 0, 0, 0, 0, StartStopMotor);

// Settings Menu Selection
Canvas(g_sMenuSetPage, &g_sMenuTypePage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240  -0),
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);

void something_test() {
    System_printf("Test");
    System_flush();
}

RectangularButton(g_sSetTemp, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 25, 144, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Temperature", 0, 0, 0, 0, DrawTemperatureSettingsPage);
RectangularButton(g_sSetMotor, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 166, 25, 144, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Motor", 0, 0, 0, 0, DrawMotorSettingsPage);
RectangularButton(g_sSetCurrent, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 115, 144, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Current", 0, 0, 0, 0, DrawCurrentSettingsPage);
RectangularButton(g_sSetAccel, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 166, 115, 144, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Acceleration", 0, 0, 0, 0, DrawAccelerationSettingsPage);
RectangularButton(g_sSetBack, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 35,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Main Menu", 0, 0, 0, 0, DrawHomeScreen);

// Graph Menu Selection
Canvas(g_sMenuGraphPage, &g_sMenuTypePage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sGraphPower, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 30, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Power", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphAmbientTemp, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 113, 30, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Amb Temp", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphSpeed, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 214, 30, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Speed", 0, 0, 0, 0, 0);

RectangularButton(g_sGraphAccel, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Acceleration", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphMotorTemp, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 113, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Motor Temp", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphLight, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 214, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Light", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphBack, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 35,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Main Menu", 0, 0, 0, 0, DrawHomeScreen);

// Settings Pages
Canvas(g_sSettingPage, &g_sSetTemp, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sSetAdd, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 40, 35, 100, 70,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "+", 0, 0, 0, 0, increaseSetting);

RectangularButton(g_sSetSub, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 40, 120, 100, 70,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "-", 0, 0, 0, 0, decreaseSetting);

RectangularButton(g_sSetName, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 150, 80, 150, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Setting (units): ", 0, 0, 0, 0, 0);

RectangularButton(g_sSetValue, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 150, 120, 150, 25,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Current Value", 0, 0, 0, 0, 0);

RectangularButton(g_sSetSave, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 40,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Save & Exit", 0, 0, 0, 0, DrawSetMenuScreen);

// Graphing Page
Canvas(g_sGraphPage, &g_sMenuGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);
Canvas(g_sGraph, &g_sGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 20, 55, 280, 140,
       CANVAS_STYLE_FILL, 0x00314570, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sGraphActBack, &g_sGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 219, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Back", 0, 0, 0, 0, 0);

void removeAllWidgets() {
    WidgetRemove((tWidget *)(&g_sMenuTypePage));
    WidgetRemove((tWidget *)(&g_sMenuSetPage));
    WidgetRemove((tWidget *)(&g_sMenuGraphPage));
    WidgetRemove((tWidget *)(&g_sSettingPage));
    WidgetRemove((tWidget *)(&g_sGraphPage));
}

// Draws the home screen
static void DrawHomeScreen()
{
    removeAllWidgets();
    WidgetPaint((tWidget *) &g_sMenuTypePage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sMenuTypePage);
}

// Draws the setting menu
static void DrawSetMenuScreen()
{
    removeAllWidgets();
    WidgetPaint((tWidget *) &g_sMenuSetPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sMenuSetPage);
}

// Draws the graph menu
static void DrawGraphMenuScreen()
{
    removeAllWidgets();
    WidgetPaint((tWidget *) &g_sMenuGraphPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sMenuGraphPage);
}

char textBuffer[20];
void DrawSettingsParameters(char * name, char * units, int value)
{
    sprintf(textBuffer, "%d %s", value, units);
    PushButtonTextSet((tPushButtonWidget *)&g_sSetName, name);
    PushButtonTextSet((tPushButtonWidget *)&g_sSetValue, textBuffer);
    WidgetPaint((tWidget *) &g_sSetName);
    WidgetPaint((tWidget *) &g_sSetValue);
}

static void DrawTemperatureSettingsPage()
{
    settingsPageIdentifier = temperature;
    removeAllWidgets();
    DrawSettingsParameters("Temperature Limit", "Celcius", motorTemperatureLimit);
    WidgetPaint((tWidget *) &g_sSettingPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sSettingPage);
}

static void DrawMotorSettingsPage()
{
    settingsPageIdentifier = motor;
    removeAllWidgets();
    DrawSettingsParameters("Motor Speed", "RPM", motorSpeedLimit);
    WidgetPaint((tWidget *) &g_sSettingPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sSettingPage);
}

static void DrawCurrentSettingsPage()
{
    settingsPageIdentifier = current;
    removeAllWidgets();
    DrawSettingsParameters("Current Limit", "Amps", motorCurrentLimit);
    WidgetPaint((tWidget *) &g_sSettingPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sSettingPage);
}

static void DrawAccelerationSettingsPage()
{
    settingsPageIdentifier = acceleration;
    removeAllWidgets();
    DrawSettingsParameters("Acceleration Limit", "m/s^2", motorAccelerationLimit);
    WidgetPaint((tWidget *) &g_sSettingPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sSettingPage);
}

static void increaseSetting() {
    switch (settingsPageIdentifier) {
        case temperature:
            motorTemperatureLimit += 5;
            DrawSettingsParameters("Temperature Limit", "Celcius", motorTemperatureLimit);
            break;
        case motor:
            motorSpeedLimit += 5;
            DrawSettingsParameters("Motor Speed", "RPM", motorSpeedLimit);
            break;
        case current:
            motorCurrentLimit += 5;
            DrawSettingsParameters("Current Limit", "Amps", motorCurrentLimit);
            break;
        case acceleration:
            motorAccelerationLimit += 5;
            DrawSettingsParameters("Acceleration Limit", "m/s^2", motorAccelerationLimit);
            break;
        default:
            System_printf("You shouldn't be here...");
    }
}

static void decreaseSetting() {
    switch (settingsPageIdentifier) {
        case temperature:
            motorTemperatureLimit -= 5;
            DrawSettingsParameters("Temperature Limit", "Celcius", motorTemperatureLimit);
            break;
        case motor:
            motorSpeedLimit -= 5;
            DrawSettingsParameters("Motor Speed", "RPM", motorSpeedLimit);
            break;
        case current:
            motorCurrentLimit -= 5;
            DrawSettingsParameters("Current Limit", "Amps", motorCurrentLimit);
            break;
        case acceleration:
            motorAccelerationLimit -= 5;
            DrawSettingsParameters("Acceleration Limit", "m/s^2", motorAccelerationLimit);
            break;
        default:
            System_printf("You shouldn't be here...");
    }
}

int motorState = 0;
static void StartStopMotor() {
    if (motorState == 0) {
        PushButtonTextSet((tPushButtonWidget *)&g_sMotorOption, "Stop Motor");
        motorState = 1;
    } else {
        PushButtonTextSet((tPushButtonWidget *)&g_sMotorOption, "Start Motor");
        motorState = 0;
    }
    WidgetPaint((tWidget *) &g_sMotorOption);
}

void initSettingValues() {
    motorTemperatureLimit = 50;
    motorSpeedLimit = 50;
    motorCurrentLimit = 50;
    motorAccelerationLimit = 50;
}

void UserInterfaceInit(uint32_t systemclock, tContext * sContext)
{
    // Init setting values
    initSettingValues();

    // Init the display driver
    Kentec320x240x16_SSD2119Init(systemclock);

    // Init Graphics Context
    GrContextInit(sContext, &g_sKentec320x240x16_SSD2119);

    // New Main Screen
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sMenuTypePage);
    WidgetAdd((tWidget *)&g_sMenuTypePage, (tWidget *)&g_sSetOption);
    WidgetAdd((tWidget *)&g_sMenuTypePage, (tWidget *)&g_sGraphOption);
    WidgetAdd((tWidget *)&g_sMenuTypePage, (tWidget *)&g_sMotorOption);

    // Set menu
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetTemp);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetCurrent);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetMotor);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetAccel);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetBack);

    // Graph Menu
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphAmbientTemp);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphMotorTemp);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphSpeed);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphPower);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphAccel);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphLight);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphBack);

    // Settings page
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetAdd);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetSub);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetSave);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetName);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetValue);

    // Graph page
    WidgetAdd((tWidget *)&g_sGraphPage, (tWidget *) &g_sGraph);
    WidgetAdd((tWidget *)&g_sGraphPage, (tWidget *) &g_sGraphActBack);

    DrawHomeScreen();
}

void UserInterfaceDraw(tContext * sContext) {
    WidgetMessageQueueProcess();
}
