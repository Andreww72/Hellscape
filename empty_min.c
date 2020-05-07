/* XDCtools Header files */
#include <xdc/std.h>
//#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

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
#include "ui/user_interface.h"
#include "motor/motor_api.h"

#define TASKSTACKSIZE   2048

Char taskStack[TASKSTACKSIZE];
Char taskStack2[TASKSTACKSIZE];

tContext sContext;

void userInterfaceFxn(UArg arg0, UArg arg1)
{
    UserInterfaceInit(arg0, &sContext);

    if (!initMotor()) {
//        System_printf("Motorlib initialisation failed...");
//        System_flush();
        while (1) {

        }
    }

    while (1) {

    }
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

    /* Init UI task */
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 5;
    taskParams.stack = &taskStack;
    taskParams.arg0 = ui32SysClock;
    Task_Handle uiTask = Task_create((Task_FuncPtr)userInterfaceFxn, &taskParams, NULL);

    /* Turn on user LED  */
    GPIO_write(Board_LED0, Board_LED_ON);

    /* Start BIOS */
    BIOS_start();


    return (0);
}
