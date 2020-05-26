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
#include "motor/motor_api.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"

// Define the y-axis limits for the graphs
#define POWER_VAL_LOW 0
#define POWER_VAL_HIGH 100
#define AMB_TEMP_VAL_LOW 0
#define AMB_TEMP_VAL_HIGH 100
#define SPEED_VAL_LOW 0
#define SPEED_VAL_HIGH 6000
#define ACCELERATION_VAL_LOW 0
#define ACCELERATION_VAL_HIGH 100
#define MOTOR_TEMP_VAL_LOW 0
#define MOTOR_TEMP_VAL_HIGH 100
#define LIGHT_VAL_LOW 0
#define LIGHT_VAL_HIGH 100

// EEPROM
#define E2PROM_ADRES 0x0000
#define EEPROM_EMPTY 0xFFFFFFFF

// Graph bounds
#define X_GRAPH 20
#define Y_GRAPH 55
#define WIDTH_GRAPH 280
#define HEIGHT_GRAPH 140
#define MAX_PLOT_SAMPLES 20

// Graph min and max
uint32_t minGraph;
uint32_t maxGraph;

// Get sContext for empty_min.c externally
extern tContext sContext;
extern bool shouldDrawDataOnGraph;

// EEPROM settings
uint32_t e2prom_write_settings[4] = {50, 50, 50, 50}; /* Write struct */
uint32_t e2prom_read_settings[4] =  {0, 1000, 0, 0}; /* Read struct */

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
                   g_psFontCmss18b, "Power", 0, 0, 0, 0, drawPowerGraph);
RectangularButton(g_sGraphAmbientTemp, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 113, 30, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Amb Temp", 0, 0, 0, 0, drawAmbientTemperatureGraph);
RectangularButton(g_sGraphSpeed, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 214, 30, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Speed", 0, 0, 0, 0, drawSpeedGraph);
RectangularButton(g_sGraphAccel, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Acceleration", 0, 0, 0, 0, drawAccelerationGraph);
RectangularButton(g_sGraphMotorTemp, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 113, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Motor Temp", 0, 0, 0, 0, drawMotorTemperatureGraph);
RectangularButton(g_sGraphLight, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 214, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Light", 0, 0, 0, 0, drawLightGraph);
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

// The actual graphing page
Canvas(g_sGraphPage, &g_sMenuGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, 240,
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);
// The graph itself
Canvas(g_sGraph, &g_sGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, X_GRAPH, Y_GRAPH, WIDTH_GRAPH, HEIGHT_GRAPH,
       CANVAS_STYLE_FILL, 0x00595D69, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sGraphActBack, &g_sGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 219, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x002546A1, ClrBlack, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Back", 0, 0, 0, 0, returnFromGraph);

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

static void doChangeToSetting(int amount) {
    switch (settingsPageIdentifier) {
        case temperature:
            motorTemperatureLimit += amount;
            e2prom_write_settings[0] = motorTemperatureLimit;
            writeToEEPROM();
            DrawSettingsParameters("Temperature Limit", "Celcius", motorTemperatureLimit);
            break;
        case motor:
            motorSpeedLimit += amount;
            e2prom_write_settings[1] = motorSpeedLimit;
            writeToEEPROM();
            DrawSettingsParameters("Motor Speed", "RPM", motorSpeedLimit);
            setDesiredSpeed(motorSpeedLimit);
            break;
        case current:
            motorCurrentLimit += amount;
            e2prom_write_settings[2] = motorCurrentLimit;
            writeToEEPROM();
            DrawSettingsParameters("Current Limit", "Amps", motorCurrentLimit);
            break;
        case acceleration:
            motorAccelerationLimit += amount;
            e2prom_write_settings[3] = motorAccelerationLimit;
            writeToEEPROM();
            DrawSettingsParameters("Acceleration Limit", "m/s^2", motorAccelerationLimit);
            break;
        default:
            System_printf("You shouldn't be here...");
    }
}

static void increaseSetting() {
    switch (settingsPageIdentifier) {
        case motor:
            if (motorSpeedLimit < 6000) {
                doChangeToSetting(100);
            }
            break;
        default:
            doChangeToSetting(5);
    }
}

static void decreaseSetting() {
    switch (settingsPageIdentifier) {
        case motor:
            if (motorSpeedLimit > 200) {
                doChangeToSetting(-100);
            }
            break;
        default:
            doChangeToSetting(-5);
    }
}

int motorState = 0;
static void StartStopMotor() {
    if (motorState == 0) {
        PushButtonTextSet((tPushButtonWidget *)&g_sMotorOption, "Stop Motor");
        motorState = 1;
        startMotor(motorSpeedLimit);
    } else {
        PushButtonTextSet((tPushButtonWidget *)&g_sMotorOption, "Start Motor");
        motorState = 0;
        stopMotor_api();
    }
    WidgetPaint((tWidget *) &g_sMotorOption);
}

static void drawPowerGraph()
{
    graphPageIdentifier = powerGraph;
    setupGraphScreen("Power Graph (Watts)", POWER_VAL_LOW, POWER_VAL_HIGH);
}

static void drawAmbientTemperatureGraph()
{
    graphPageIdentifier = ambTempGraph;
    setupGraphScreen("Ambient Temp (Celcius)", AMB_TEMP_VAL_LOW, AMB_TEMP_VAL_HIGH);
}

static void drawSpeedGraph()
{
    graphPageIdentifier = speedGraph;
    setupGraphScreen("Speed (RPM)", SPEED_VAL_LOW, SPEED_VAL_HIGH);
}

static void drawAccelerationGraph()
{
    graphPageIdentifier = accelerationGraph;
    setupGraphScreen("Acceleration (m/s^2)", ACCELERATION_VAL_LOW, ACCELERATION_VAL_HIGH);
}

static void drawMotorTemperatureGraph()
{
    graphPageIdentifier = motorTempGraph;
    setupGraphScreen("Motor Temp (Celcius)", MOTOR_TEMP_VAL_LOW, MOTOR_TEMP_VAL_HIGH);
}

static void drawLightGraph()
{
    graphPageIdentifier = lightGraph;
    setupGraphScreen("Light (Lux)", LIGHT_VAL_LOW, LIGHT_VAL_HIGH);
}

// Setup graphing screen
char lowLimit[5];
char highLimit[5];
static void setupGraphScreen(char * title, int yMin, int yMax)
{
    // Set graph bounds
    minGraph = yMin;
    maxGraph = yMax;

    sprintf(lowLimit, "%d", yMin);
    sprintf(highLimit, "%d", yMax);
    // Remove and paint
    removeAllWidgets();
    WidgetPaint((tWidget *) &g_sGraphPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sGraphPage);
    WidgetMessageQueueProcess();

    // Draw the graph
    GrStringDraw(&sContext, "Time", -1, 130, 202, false);
    GrLineDrawV(&sContext, 10, 45, 200);
    GrLineDrawH(&sContext, 10, 305, 200);
    GrLineDraw(&sContext, 10, 45, 8,50);
    GrLineDraw(&sContext, 10, 45, 12, 50);
    GrLineDraw(&sContext, 305, 200, 303, 198);
    GrLineDraw(&sContext, 305, 200, 303, 202);

    // Plot title, and y-axis limits
    GrStringDraw(&sContext, title, -1, 5, 15, true);
    GrStringDraw(&sContext, lowLimit, -1, 10, 202, false);
    GrStringDraw(&sContext, highLimit, -1, 18, 38, false);

    drawingGraph = 1;
    // AddValueToGraph(50, powerGraph);
    // AddValueToGraph(50, powerGraph);
    // DrawDataOnGraph(yMin, yMax, rand() % 100);
}

// Graphs the chosen data on the map and scales accordingly
uint16_t pointsCount = 0;
uint16_t previousSample = 0;
static void returnFromGraph()
{
    DrawGraphMenuScreen();
    drawingGraph = 0;

    pointsCount = 0;
    previousSample = 0;
}

float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

static void DrawDataOnGraph(uint32_t lastSample)
{
    if (pointsCount == MAX_PLOT_SAMPLES){
        // Reset graph
        pointsCount = 0;
        WidgetPaint((tWidget *) &g_sGraph);
        WidgetMessageQueueProcess();
        return;
    }

    // Draw current value
    static char currentValue[30];
    sprintf(currentValue, "%d", lastSample);
    GrContextBackgroundSet(&sContext, 0x00595D69);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrContextFontSet(&sContext, g_psFontCmss18b);
    GrStringDrawCentered(&sContext, "Current Value:", -1, 240, 15, true);
    GrStringDrawCentered(&sContext, "", 30, 240, 35, true); // draw a big blank line to remove overlap
    GrStringDrawCentered(&sContext, currentValue, -1, 240, 35, true);
    GrFlush(&sContext);

    // Calculate y and x scale
    float yscale = (float)height/(float)(maxGraph - minGraph);
    float xscale = (float)width/(float)MAX_PLOT_SAMPLES;

    // Declare line coordinates
    uint16_t x1, x2, y1, y2;

    // Draw onto graph
    x1 = (pointsCount * xscale) + s_x;
    y1 = ((s_y + height) - (previousSample - minGraph) * yscale);
    x2 = ((pointsCount + 1) * xscale)+s_x;
    y2 = ((s_y + height) - (lastSample - minGraph) * yscale);
    if (y1 < s_y) y1 = s_y + 1;
    if (y1 > s_y + height) y1 = s_y + height - 1;
    if (y2 < s_y) y2 = s_y + 1;
    if (y2 > s_y + height) y2 = s_y + height - 1;
    GrLineDraw(&sContext, x1, y1, x2, y2);
    GrFlush(&sContext);


    // Increase the number of points plotted count
    pointsCount +=1;

    // Change the previous sample
    previousSample = lastSample;
}

void AddValueToGraph(uint32_t lastSample, int graphPage) {
    if (drawingGraph && graphPageIdentifier == graphPage) {
        DrawDataOnGraph(lastSample);
    }
}

void initSettingValues() {
    // Init page state
    settingsPageIdentifier = temperature;
    graphPageIdentifier = powerGraph;

    // Settings
    motorTemperatureLimit = e2prom_read_settings[0];
    motorSpeedLimit = e2prom_read_settings[1];
    motorCurrentLimit = e2prom_read_settings[2];
    motorAccelerationLimit = e2prom_read_settings[3];

    // Initially not drawing a graph
    drawingGraph = 0;

    // Graph positioning
    s_x = X_GRAPH;
    s_y = Y_GRAPH + 1;
    width = WIDTH_GRAPH - 1;
    height = HEIGHT_GRAPH - 2;
}

void writeToEEPROM(uint32_t settings[4]) {
    EEPROMProgram((uint32_t *)&e2prom_write_settings, E2PROM_ADRES, sizeof(e2prom_write_settings));
}

void setupEEPROM() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); // EEPROM activate
    EEPROMInit(); // EEPROM start

    // Clear EEPROM
    //EEPROMMassErase();

    // Read the settings
    EEPROMRead((uint32_t *)&e2prom_read_settings, E2PROM_ADRES, sizeof(e2prom_read_settings));

    // If nothing in EEPROM, set to default
    if (e2prom_read_settings[0] == EEPROM_EMPTY) {
        EEPROMProgram((uint32_t *)&e2prom_write_settings, E2PROM_ADRES, sizeof(e2prom_write_settings)); //Write struct to EEPROM start from 0x0000
    }

    // Read the settings again after they've been set to default
    EEPROMRead((uint32_t *)&e2prom_read_settings, E2PROM_ADRES, sizeof(e2prom_read_settings));
}

void UserInterfaceInit(uint32_t systemclock, tContext * sContext)
{
    // EEPROM setup
    setupEEPROM();

    // Init setting values
    initSettingValues();

    // Init the display driver
    Kentec320x240x16_SSD2119Init(systemclock);

    // Init Graphics Context
    GrContextInit(sContext, &g_sKentec320x240x16_SSD2119);

    // Fonts and stuff
    GrContextBackgroundSet(sContext, 0x00595D69);
    GrContextForegroundSet(sContext, ClrWhite);
    GrContextFontSet(sContext, g_psFontCmss18b);

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
