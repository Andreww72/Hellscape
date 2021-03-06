#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>
#include "Board.h"
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

#define UI_TASKSTACKSIZE 4096
#define SE_TASKSTACKSIZE 4096

Char taskUiStack[UI_TASKSTACKSIZE];
Char taskSeStack[SE_TASKSTACKSIZE];
tContext sContext;

Clock_Struct clkUIStruct;
Clock_Struct clkDrawStruct;
Clock_Handle clkHandle;
Clock_Handle clkHandleGraph;

bool isDay = false;
bool shouldDrawDateTime = true;
bool shouldDrawDataOnGraph = false;
extern uint16_t drawingGraph;

extern void TouchScreenIntHandler(void);

time_t t1;
// Gets the current date and time
static char * getCurrentDateTime() {
    static char t[30];
    struct tm * timeinfo;
    timeinfo = localtime (&t1);
    timeinfo->tm_hour += 16;
    if (timeinfo->tm_hour > 24) {
        timeinfo->tm_hour -= 24;
        timeinfo->tm_mday +=1;
    }
    strcpy(t, "    "); // add an extra space
    strcpy(t, asctime(timeinfo));
    return t;
}

void ClockFxn(UArg arg0) {
    shouldDrawDateTime = true;
    t1++;
}

void shouldDrawDataClock(UArg arg0) {
    shouldDrawDataOnGraph = true;
}

// Draws the date, time
void DrawDateTime() {
    drawDayNight(getLight() > 5);
    if (shouldDrawDateTime && !drawingGraph) {
        GrStringDrawCentered(&sContext, getCurrentDateTime(), -1, 160, 8, true);
        GrFlush(&sContext);
        shouldDrawDateTime = false;
    }
}

void userInterfaceFxn(UArg ui32SysClock) {
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
        t1 = 3800284593;
    }

    UserInterfaceInit(ui32SysClock, &sContext);


    while(1) {
        UserInterfaceDraw(&sContext);
        DrawDateTime();
    }
}

void sensorsFxn() {
    // Default threshold integer parameters: degrees C, mA, m/s^2
    if (!initSensors(30, 1500, 20)) {
        System_abort("Failed sensor init");
        System_flush();
    }
}

bool setupSensorsAndGUI(uint32_t ui32SysClock) {
    Task_Params taskParams;
    Task_Params_init(&taskParams);

    // Init sensor setup task
    taskParams.stackSize = SE_TASKSTACKSIZE;
    taskParams.priority = 11;
    taskParams.stack = &taskSeStack;
    Task_Handle sensorTask = Task_create((Task_FuncPtr)sensorsFxn, &taskParams, NULL);
    if (sensorTask == NULL) {
        System_abort("Task - SENSOR FAILED SETUP");
    }

    // Init UI task
    taskParams.stackSize = UI_TASKSTACKSIZE;
    taskParams.priority = 10;
    taskParams.stack = &taskUiStack;
    taskParams.arg0 = ui32SysClock;
    Task_Handle uiTask = Task_create((Task_FuncPtr)userInterfaceFxn, &taskParams, NULL);
    if (uiTask == NULL) {
        System_abort("Task - GUI FAILED SETUP");
    }

    // Increment date time by a second once a second
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 1000;
    clkParams.startFlag = TRUE;
    Clock_construct(&clkUIStruct, (Clock_FuncPtr)ClockFxn, 1, &clkParams);
    clkHandle = Clock_handle(&clkUIStruct);
    if (clkHandle == NULL) {
        System_printf("Task - CLOCK SETUP FAILED");
        System_flush();
        return 0;
    }
    Clock_start(clkHandle);

    // Update an open graph at 10Hz
    clkParams.period = 100;
    Clock_construct(&clkDrawStruct, (Clock_FuncPtr)shouldDrawDataClock, 1, &clkParams);
    clkHandleGraph = Clock_handle(&clkDrawStruct);
    if (clkHandleGraph == NULL) {
        System_printf("Task - CLOCK SETUP FAILED");
        System_flush();
        return 0;
    }
    Clock_start(clkHandleGraph);
    return 1;
}

int main(void) {
    // Call board init functions
    Board_initGeneral();
    Board_initGPIO();
    Board_initI2C();
    Board_initUART();

    PWM_init();

    // Set system clock
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

    // Setup GUI and sensors
    setupSensorsAndGUI(ui32SysClock);

    // Start BIOS
    BIOS_start();

    return (0);
}
