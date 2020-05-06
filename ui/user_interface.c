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
#include "tabs.h"

void UserInterfaceInit(uint32_t systemclock, tContext * sContext)
{
    // Init the display driver
    Kentec320x240x16_SSD2119Init(systemclock);

    // Init Graphics Context
    GrContextInit(sContext, &g_sKentec320x240x16_SSD2119);

    // Draw frame
    FrameDraw(sContext, "Motor Controller - Group 7");

    // Setup tabs
    setup_tabs();

    WidgetPaint(WIDGET_ROOT);
}

void UserInterfaceDraw(tContext * sContext) {
    WidgetMessageQueueProcess();
}
