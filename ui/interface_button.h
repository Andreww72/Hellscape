#ifndef UI_INTERFACE_BUTTON_H_
#define UI_INTERFACE_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "grlib/pushbutton.h"
#include "drivers/frame.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/pinout.h"
#include "drivers/touch.h"
#include "drivers/buttons.h"

#ifndef BUTTON_NONE
    #define BUTTON_NONE 0
#endif
#ifndef BUTTON_UP
    #define BUTTON_UP 1
#endif
#ifndef BUTTON_DOWN
    #define BUTTON_DOWN 2
#endif
#ifndef BUTTON_SELECT
    #define BUTTON_SELECT 3
#endif


void initInterfaceButton();

uint8_t validInterfaceButton(uint8_t screen, uint8_t *button_pressed);

uint8_t OnButtonPress();



#endif /* UI_INTERFACE_BUTTON_H_ */
