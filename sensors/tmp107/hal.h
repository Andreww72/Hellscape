/* --COPYRIGHT--,BSD
 * Copyright (c) 2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/*
 * hal.h
 *
 *  Created on: Feb 12, 2016
 *      Author: a0271474
 *
 * Hardware Abstraction Layer
 */

/* This example assumes a hardware UART with dedicated Tx and Rx pins. Both of
 * these pins must connect to TMP107's IO1 pin in order to operate. Since there
 * is a possibility for both Master Tx and TMP107 IO1 to actively drive the line
 * at the same, some kind of arbitration must be performed.
 *
 * The TMP107 datasheet suggests using a Tri-State driver (SN74LVC1G125) to
 * emulate an open-drain buffer like SN74LVC1G07. Either solution converts the
 * Master's Tx line to turn off (Hi-Z) when the Master is attempting to drive
 * a logic high (1.) This relies on a pull-up resistor to create the logic
 * high in the same way as used in I2C/SMBus. This is a safe and straight
 * forward way to solve the arbitration issue, but it requires an extra IC.
 *
 * A second way to solve the arbitration issue is to place a 10kohm resistor
 * in series with the Master's Tx line. In the case of a collision where both
 * Master and TMP107 attempt to drive the line, this resistor will limit the
 * Master's drive strength, and prevent any damage. Since UART idles at a logic
 * high, when the Master is inactive, this series resistor acts as a pull-up.
 * The pull-up is not required for TMP107 operation, but it is not harmful either.
 *
 * Both of the above configurations operate without any special software
 * considerations. In fact, they will work with PC controlled COM port. The
 * hardware configuration described below will only work when special care is taken
 * within software to prevent collisions and arbitration issues.
 *
 * Finally, the holy grail is to directly short the Master's Tx and Rx pins
 * together, and then connect them to TMP107 IO1. This hardware configuration
 * requires no external components, but does require software modifications. If
 * your hardware is connected in this way, uncomment the line below for TMP107
 * OneWireUART.
 */

#include <ti/drivers/UART.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <Board.h>

#define TMP107_Temp_reg 0xA0

void init_motor_uart();
void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size);

void TMP107_Transmit(char* tx_data, char tx_size);
char TMP107_WaitForEcho(char tx_size, char rx_size, int timeout_ms);
void TMP107_RetrieveReadback(char tx_size, char* rx_data, char rx_size);
