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
#include "motor/motor_api.h"
#include "sensors/sensor_api.h"

#define UI_TASKSTACKSIZE   4096

Char taskStack[UI_TASKSTACKSIZE];
tContext sContext;

Clock_Struct clk0Struct;
Clock_Handle clkHandle;

bool shouldDrawDateTime = true;
bool shouldDrawDataOnGraph = false;
extern uint16_t drawingGraph;

extern void TouchScreenIntHandler(void);

time_t t1;
// Gets the current date and time
static char * getCurrentDateTime() {
    static char t[30];
    struct tm * timeinfo;
    timeinfo = localtime ( &t1 );
    timeinfo->tm_hour += 16;
    if (timeinfo->tm_hour>24) {
        timeinfo->tm_hour -= 24;
        timeinfo->tm_mday +=1;
    }
    strcpy(t,asctime(timeinfo));
    return t;
}

void ClockFxn(UArg arg0) {
    shouldDrawDateTime = true;
    shouldDrawDataOnGraph = true;
    t1++;
}

// Draws the date, time
void DrawDateTime() {
    if (shouldDrawDateTime && !drawingGraph) {
        GrStringDrawCentered(&sContext, getCurrentDateTime(), -1, 160, 8, true);
        GrFlush(&sContext);
        shouldDrawDateTime = false;
    }
}

void userInterfaceFxn(UArg arg0, UArg arg1) {
    if (!initSensors(40, 1, 1)) {
        System_abort("Failed sensor init");
        System_flush();
    }

    UserInterfaceInit(arg0, &sContext);

    while(1) {
        UserInterfaceDraw(&sContext);
//        DrawDateTime();
    }
}

bool setupSensorsAndGUI(uint32_t ui32SysClock) {
    // Need this for Touchscreen
    Hwi_Params hwiParams;
    Hwi_Params_init(&hwiParams);
    hwiParams.arg = 1;
    hwiParams.priority = 1;
    Hwi_Handle hallInt0 = Hwi_create(33, (Hwi_FuncPtr)TouchScreenIntHandler, &hwiParams, NULL);

    // Init touchscreen
    TouchScreenInit(ui32SysClock);
    TouchScreenCallbackSet(WidgetPointerMessage);

    // Init time
    t1 = time(NULL);
    if (t1 < 10000) {
        System_printf("Time - GET TIME FAILED");
        System_flush();
        // Set to a reasonable time
        t1 = 3798880499;
    }

    /* Init UI task */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = UI_TASKSTACKSIZE;
    taskParams.priority = 10;
    taskParams.stack = &taskStack;
    taskParams.arg0 = ui32SysClock;
    Task_Handle uiTask = Task_create((Task_FuncPtr)userInterfaceFxn, &taskParams, NULL);
    if (uiTask == NULL) {
        System_printf("Task - GUI FAILED SETUP");
        System_flush();
        return 0;
    }

    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 1000;
    clkParams.startFlag = TRUE;
    Clock_construct(&clk0Struct, (Clock_FuncPtr)ClockFxn, 1, &clkParams);
    clkHandle = Clock_handle(&clk0Struct);
    if (clkHandle == NULL) {
        System_printf("Task - CLOCK SETUP FAILED");
        System_flush();
        return 0;
    }
    Clock_start(clkHandle);
    return 1;
}

int main(void) {
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initUART();

    PWM_init();

    /* Set system clock */
    uint32_t ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
            SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
            SYSCTL_CFG_VCO_480), 120000000);

    if (!initMotor()) {
        System_printf("Motorlib initialisation failed\n");
        System_flush();
        System_abort("Motor init failed");
    }

    // Enable interrupts
    IntMasterEnable();

    // Set pinout
    PinoutSet(false, false);

    // Setup sensors
    //setupSensors();

    // Setup GUI and sensors
    setupSensorsAndGUI(ui32SysClock);

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();

    return (0);
}
