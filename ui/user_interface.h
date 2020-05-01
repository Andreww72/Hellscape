/*
 * user_interface.h
 *
 *  Created on: May 1, 2020
 *      Author: Lachlan
 */

#ifndef UI_USER_INTERFACE_H_
#define UI_USER_INTERFACE_H_

#include <stdint.h>
#include <stdbool.h>
#include <driverlib/gpio.h>
#include "grlib/grlib.h"
#include "grlib/widget.h"
#include "grlib/canvas.h"
#include "drivers/pinout.h"
#include "drivers/Kentec320x240x16_ssd2119_spi.h"
#include "drivers/frame.h"

void UserInterfaceInit(uint32_t systemclock, tContext * sContext);

#endif /* UI_USER_INTERFACE_H_ */
