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
 * hal.c
 *
 *  Created on: Feb 12, 2016
 *      Author: a0271474
 *
 * Hardware Abstraction Layer
 */

#include "hal.h"
#include "tmp107.h"
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

char tmp107_rx[128]; // receive buffer: filled by UART RX ISR
char tmp107_rxcnt; // receive count: tracks current location in rx buffer
UART_Handle uartMotor;
char temp[10];

void init_motor_uart() {
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.readMode = UART_MODE_CALLBACK;
    uartParams.readCallback = Uart_ReadCallback;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 9600;
    uartMotor = UART_open(Board_UART7, &uartParams);

    if (uartMotor == NULL) {
        System_printf("Error opening the UART");
        System_flush();
    }
}

void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size) {
    char read = *(char*)rxBuf;
    if (read) {
        tmp107_rx[tmp107_rxcnt] = *(char*)rxBuf;
        tmp107_rxcnt++;
    }
}

void TMP107_Transmit(char* tx_data, char tx_size){
	// Clear receive buffer
	tmp107_rxcnt = 0;

    // Send calibration byte
	char calibrationByte[1];
	calibrationByte[0] = 0x55;
	UART_write(uartMotor, calibrationByte, 1);
	// Send rest of packet
    UART_write(uartMotor, tx_data, tx_size);
}

char TMP107_WaitForEcho(char tx_size, char rx_size, int timeout_ms){
	/* Used after a call to Transmit, this function will exit once the transmit echo
	 * and any additional rx bytes are received. it will also exit due to time out
	 * if timeout_ms lapses without receiving new bytes. this function returns the
	 * number of bytes received after tx echo.
	 */
	char i = 0;
	int count_ms = 0;
	// Echo of cal byte + echo of transmission + additional bytes if read command
	char expected_rxcnt = 1 + tx_size + rx_size;
	/* Loop synopsis:
	 * Wait for expected_rxcnt
	 * Check once per millisecond, up to 40ms time out
	 * Reset time out counter when a byte is received
	 *
	 * This loop runs while UART RX is being handled by ISR,
	 * and reacts to the number of bytes that are currently
	 * in the RX buffer.
	 *
	 * It is essential that all bytes are received, or that the
	 * appropriate timeout has been endured, before another
	 * transmit can occur. otherwise, corruption can occur.
	 */
	while (count_ms < timeout_ms) {
		if (tmp107_rxcnt < expected_rxcnt) {
			if (tmp107_rxcnt > i)
				count_ms = 0;
			i = tmp107_rxcnt;
	        UART_read(uartMotor, &temp, 1);
			count_ms++;
		} else {
			count_ms = timeout_ms;
		}
	}
	return (tmp107_rxcnt - 1 - tx_size);
}

void TMP107_RetrieveReadback(char tx_size, char* rx_data, char rx_size){
	// Copy bytes received from UART buffer to user supplied array
	char i;
	if (rx_size > 0){
		for (i = 0; i < rx_size; i++){
			rx_data[i] = tmp107_rx[1 + tx_size + i];
		}
	}
}
