#include "user_interface.h"
#include <driverlib/gpio.h>
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "drivers/pinout.h"
#include "drivers/frame.h"
#include "ui/interface_button.h"

tCanvasWidget g_sBackground;
Canvas(g_sBackground, WIDGET_ROOT, 0, 0,
       &g_sKentec320x240x16_SSD2119, 10, 25, 300, (240 - 25 -10),
       CANVAS_STYLE_FILL, ClrBlue, 0, 0, 0, 0, 0, 0);


void UserInterfaceInit(uint32_t systemclock, tContext * sContext)
{
    PinoutSet(false, false);
    Kentec320x240x16_SSD2119Init(systemclock);
    GrContextInit(sContext, &g_sKentec320x240x16_SSD2119);

    initInterfaceButton();

    // Root
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sBackground);

    FrameDraw(sContext, "Motor Controller - Group 7");

    WidgetPaint(WIDGET_ROOT);

}
