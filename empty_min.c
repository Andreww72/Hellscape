/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>


/* Board Header file */
#include "Board.h"

/* Custom API */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "inc/hw_gpio.h"
#include "inc/hw_hibernate.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/systick.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "drivers/pinout.h"
#include "drivers/touch.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "ui/user_interface.h"

#define TASKSTACKSIZE   4096

Char taskStack[TASKSTACKSIZE];

tContext sContext;

extern void TouchScreenIntHandler(void);


// Gets the current date and time
static char * getCurrentDateTime()
{
    static char t[30];
    struct tm * timeinfo;
    time_t t1 = time (NULL);
    timeinfo = localtime ( &t1 );
    timeinfo->tm_hour += 16;
    if (timeinfo->tm_hour>24){
        timeinfo->tm_hour -= 24;
        timeinfo->tm_mday +=1;
    }
    strcpy(t,asctime(timeinfo));
    return t;
}

// Draws the date, time and lux sensor results to the title bar
void DrawDateTime()
{
    // Day time
    GrContextForegroundSet(&sContext, 0x00D8D4D5);
    GrCircleFill(&sContext,5, 5,10);
    GrContextForegroundSet(&sContext, 0x007584AD);
    GrLineDraw(&sContext,15, 15, 20, 20);
    GrLineDraw(&sContext,18, 10, 23, 13);
    GrLineDraw(&sContext,19, 5,25,6);

    GrContextBackgroundSet(&sContext, 0x007584AD);
    GrContextForegroundSet(&sContext, ClrWhite);
    GrContextFontSet(&sContext, g_psFontCmss18b);
    GrStringDrawCentered(&sContext, getCurrentDateTime(), -1, 160, 8, true);
    GrFlush(&sContext);

}

void userInterfaceFxn(UArg arg0, UArg arg1)
{
    UserInterfaceInit(arg0, &sContext);
    while(1)
    {
        UserInterfaceDraw(&sContext);
        DrawDateTime();
    }


}

void test_func() {
    System_printf("Here");
    System_flush();
}

bool setupGUI(uint32_t ui32SysClock) {
    // Need this for Touchscreen
    Hwi_Params hwiParams;
    Hwi_Params_init(&hwiParams);
    hwiParams.arg = 1;
    hwiParams.priority = 1;
    Hwi_Handle hallInt0 = Hwi_create(33, (Hwi_FuncPtr)TouchScreenIntHandler, &hwiParams, NULL);

    // Init touchscreen
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(WidgetPointerMessage);

    /* Init UI task */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 10;
    taskParams.stack = &taskStack;
    taskParams.arg0 = ui32SysClock;
    Task_Handle uiTask = Task_create((Task_FuncPtr)userInterfaceFxn, &taskParams, NULL);
    if (uiTask == NULL) {
        System_printf("Task - GUI FAILED SETUP");
        System_flush();
        return 0;
    }

    Clock_Params clkParamsGUI;
    Clock_Params_init(&clkParamsGUI);
    clkParamsGUI.period = 5000;
    clkParamsGUI.startFlag = TRUE;
    taskParams.priority = 15;
    Error_Block eb;
    //Clock_Handle clockRTOS = Clock_create((Clock_FuncPtr)DrawDateTime,
    //                                      5000, &clkParamsGUI, &eb);
    return 1;
}

int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();

    /* Set system clock */
    uint32_t ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
            SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
            SYSCTL_CFG_VCO_480), 120000000);

    // Enable interrupts
    IntMasterEnable();

    // Set pinout
    PinoutSet(false, false);

    // Setup GUI
    setupGUI(ui32SysClock);

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
