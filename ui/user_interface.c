#include "user_interface.h"
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <inc/hw_ints.h>
#include <time.h>
#include <stdio.h>
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


// GUI - Canvas Drawing
// Set/Graph Menu Selection
Canvas(g_sMenuTypePage, WIDGET_ROOT, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x007584AD, 0, 0, 0, 0, 0, 0);//0x00E09873

RectangularButton(g_sSetOption, &g_sMenuTypePage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 25, 145, 200,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrBlack, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Control", 0, 0, 0, 0, DrawSetMenuScreen);
RectangularButton(g_sGraphOption, &g_sMenuTypePage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 161, 25, 145, 200,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Graph", 0, 0, 0, 0, 0);

// Set Menu Selection
Canvas(g_sMenuSetPage, &g_sMenuTypePage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240  -0),
       CANVAS_STYLE_FILL, 0x007584AD, 0, 0, 0, 0, 0, 0);

void something_test() {
    System_printf("Test");
    System_flush();
}

RectangularButton(g_sSetTemp, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 25, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Temp", 0, 0, 0, 0, something_test);
RectangularButton(g_sSetMotor, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 161, 25, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Motor", 0, 0, 0, 0, 0);
RectangularButton(g_sSetCurrent, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 115, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Current", 0, 0, 0, 0, 0);
RectangularButton(g_sSetAccel, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 161, 115, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Accel", 0, 0, 0, 0, 0);
RectangularButton(g_sSetBack, &g_sMenuSetPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Back To Home", 0, 0, 0, 0, DrawHomeScreen);

// Graph Menu Selection
Canvas(g_sMenuGraphPage, &g_sMenuTypePage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x007584AD, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sGraphPower, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 25, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Power", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphSpeed, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 161, 25, 149, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Speed", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphAccel, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Accel", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphTemp, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 113, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Temp", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphLight, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 214, 115, 96, 80,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Light", 0, 0, 0, 0, 0);
RectangularButton(g_sGraphBack, &g_sMenuGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Back To Home", 0, 0, 0, 0, DrawHomeScreen);

// Settings Pages
Canvas(g_sSettingPage, &g_sSetTemp, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, 0x007584AD, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sSetAdd, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 200, 35, 100, 70,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "+", 0, 0, 0, 0, 0);

RectangularButton(g_sSetSub, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 200, 120, 100, 70,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "-", 0, 0, 0, 0, 0);

CircularButton(g_sSetMotorMode, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 150, 70, 35,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x0023A369, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Turn On", 0, 0, 0, 0, 0);

RectangularButton(g_sSetSave, &g_sSettingPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 200, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, 0x00AED1D6, ClrWhite,
                   g_psFontCmss18b, "Confirm", 0, 0, 0, 0, 0);


// Graphing Page
Canvas(g_sGraphPage, &g_sMenuGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 0, 0, 320, (240 - 0 -0),
       CANVAS_STYLE_FILL, 0x007584AD, 0, 0, 0, 0, 0, 0);
Canvas(g_sGraph, &g_sGraphPage, 0, 0,
       &g_sKentec320x240x16_SSD2119, 20, 55, 280, 140,
       CANVAS_STYLE_FILL, 0x00314570, 0, 0, 0, 0, 0, 0);

RectangularButton(g_sGraphActBack, &g_sGraphPage, 0, 0,
                  &g_sKentec320x240x16_SSD2119, 10, 219, 300, 20,
                  (PB_STYLE_OUTLINE | PB_STYLE_TEXT_OPAQUE | PB_STYLE_TEXT |
                    PB_STYLE_FILL | PB_STYLE_RELEASE_NOTIFY),
                    0x00314570, ClrWhite, ClrWhite, ClrWhite,
                   g_psFontCmss18b, "Back", 0, 0, 0, 0, 0);

// Draws the home screen
static void DrawHomeScreen()
{
    WidgetPaint(WIDGET_ROOT);
    //WidgetMessageQueueProcess();
}

// Draws the setting menu
static void DrawSetMenuScreen(tContext * sContext)
{
    WidgetRemove((tWidget *)(&g_sMenuTypePage));
    WidgetPaint((tWidget *) &g_sMenuSetPage);
    WidgetAdd(WIDGET_ROOT, (tWidget *) &g_sMenuSetPage);
    //updateMenuOptions(params);
    WidgetMessageQueueProcess();
}

void UserInterfaceInit(uint32_t systemclock, tContext * sContext)
{
    // Init the display driver
    Kentec320x240x16_SSD2119Init(systemclock);

    // Init Graphics Context
    GrContextInit(sContext, &g_sKentec320x240x16_SSD2119);

    // New Main Screen
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sMenuTypePage);
    WidgetAdd((tWidget *)&g_sMenuTypePage, (tWidget *)&g_sSetOption);
    WidgetAdd((tWidget *)&g_sMenuTypePage, (tWidget *)&g_sGraphOption);

    // Set menu
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetTemp);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetCurrent);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetMotor);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetAccel);
    WidgetAdd((tWidget *)&g_sMenuSetPage, (tWidget *) &g_sSetBack);

    // Graph Menu
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphTemp);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphSpeed);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphPower);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphAccel);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphLight);
    WidgetAdd((tWidget *)&g_sMenuGraphPage, (tWidget *) &g_sGraphBack);

    // Settings page
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetAdd);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetSub);
    WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetSave);
   // WidgetAdd((tWidget *)&g_sSettingPage, (tWidget *) &g_sSetMotorMode);

    // Graph page
    WidgetAdd((tWidget *)&g_sGraphPage, (tWidget *) &g_sGraph);
    WidgetAdd((tWidget *)&g_sGraphPage, (tWidget *) &g_sGraphActBack);

    DrawHomeScreen(sContext);
}

void UserInterfaceDraw(tContext * sContext) {
    WidgetMessageQueueProcess();
}
