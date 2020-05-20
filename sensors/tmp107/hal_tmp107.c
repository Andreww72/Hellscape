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
 * hal_tmp107.c
 *
 *  Created on: Feb 12, 2016
 *      Author: a0271474
 *
 * Hardware Abstraction Layer
 */

#include <driverlib.h>

#include "hal_tmp107.h"
#include "tmp107.h"

//aplication globals
char tmp107_rx[128]; // receive buffer: filled by UART RX ISR
char tmp107_rxcnt; // receive count: tracks current location in rx buffer

//void initClocks(uint32_t mclkFreq)
//{
//	UCS_initClockSignal(
//			UCS_FLLREF,
//			UCS_REFOCLK_SELECT,
//			UCS_CLOCK_DIVIDER_1);
//
//	UCS_initClockSignal(
//			UCS_ACLK,
//			UCS_REFOCLK_SELECT,
//			UCS_CLOCK_DIVIDER_1);
//
//	UCS_initFLLSettle(
//			mclkFreq / 1000,
//			mclkFreq / 32768);
//}
//
//void initPC_UART(){
//	/* Configure USCI A1 as UART with 9600 baud for PC communication.
//	 * On 5529 LaunchPad, this appears in Windows as
//	 * "MSP Application UART." Open this virtual com port with a
//	 * terminal app such as putty to view output from demo.
//	 */
//	GPIO_setAsPeripheralModuleFunctionOutputPin(4,GPIO_PIN4);
//	GPIO_setAsPeripheralModuleFunctionInputPin(4,GPIO_PIN5);
//	//baud rate config explained on page 953 of family user's guide
//	//in this design, launchpad XT2 = 4MHz crystal
//	USCI_A_UART_initParam param = {0};
//	param.selectClockSource = USCI_A_UART_CLOCKSOURCE_ACLK;
//	param.clockPrescalar = 3; //BRx bits
//	param.secondModReg = 3; //BRSx bits
//	param.firstModReg = 0; //BRFx bits=
//	param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;
//	USCI_A_UART_init(USCI_A1_BASE, &param);
//	USCI_A_UART_enable(USCI_A1_BASE);
//}
//
//void PC_Transmit(char* tx_data, char tx_size){
//	// report to PC UART
//	char i;
//	for (i = 0; i < tx_size; i++)
//		USCI_A_UART_transmitData(USCI_A1_BASE,tx_data[i]);
//}

void initTMP107(){
#ifdef TMP107_OneWireUART
	GPIO_setAsOutputPin(1,GPIO_PIN0); // used to demonstrate tx direction
#endif
	GPIO_setAsOutputPin(TMP107_Tx_Port,TMP107_Tx_Pin);
	GPIO_setAsInputPin(TMP107_Rx_Port,TMP107_Rx_Pin);
	GPIO_setAsPeripheralModuleFunctionOutputPin(TMP107_Tx_Port,TMP107_Tx_Pin,GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionInputPin(TMP107_Rx_Port,TMP107_Rx_Pin,GPIO_PRIMARY_MODULE_FUNCTION);
	//baud rate config explained on page 585 of family user's guide
	//in this design, internal DCO = 8MHz, SMCLK = 2MHz
	EUSCI_A_UART_initParam param = {0};
	param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
	param.clockPrescalar = 6; //BRx bits
	param.secondModReg = 0x20; //BRSx bits
	param.firstModReg = 8; //BRFx bits=
	param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION; //UCOS16
	EUSCI_A_UART_init(EUSCI_A0_BASE, &param);
	EUSCI_A_UART_enable(EUSCI_A0_BASE);
	EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
}

void TMP107_Transmit(char* tx_data, char tx_size){
	char i;
	//clear receive buffer
	tmp107_rxcnt = 0;
	//send calibration byte
	EUSCI_A_UART_transmitData(EUSCI_A0_BASE,0x55);
	//send rest of packet
	for (i = 0; i < tx_size; i++)
		EUSCI_A_UART_transmitData(EUSCI_A0_BASE,tx_data[i]);
}

char TMP107_WaitForEcho(char tx_size, char rx_size, int timeout_ms){
	/* used after a call to Transmit, this function will exit once the transmit echo
	 * and any additional rx bytes are received. it will also exit due to time out
	 * if timeout_ms lapses without receiving new bytes. this function returns the
	 * number of bytes received after tx echo.
	 */
	char i = 0;
	int count_ms = 0;
	char expected_rxcnt;
	// echo of cal byte + echo of transmission + additional bytes if read command
	expected_rxcnt = 1 + tx_size + rx_size;
	/* loop synopsis:
	 * wait for expected_rxcnt
	 * check once per millisecond, up to 40ms time out
	 * reset time out counter when a byte is received
	 *
	 * this loop runs while UART RX is being handled by ISR,
	 * and reacts to the number of bytes that are currently
	 * in the RX buffer.
	 *
	 * in OneWireUART mode, this loop changes direction of TX
	 * pin after transmit has finished but before TMP107 can
	 * respond.
	 *
	 * it is essential that all bytes are received, or that the
	 * apropriate timeout has been endured, before another
	 * transmit can occur. otherwise, corruption can occur.
	 */
	while (count_ms < timeout_ms){
		if (tmp107_rxcnt < expected_rxcnt) {
#ifdef TMP107_OneWireUART
			if (tmp107_rxcnt == 1 + tx_size){
				P1OUT |= 1; //helps visualize direction of TX pin if monitored along side comm pins
				GPIO_setAsInputPinWithPullUpResistor(TMP107_Tx_Port,TMP107_Tx_Pin); // needs Tx-Rx short to avoid bad logic low from slave
				// don't use GPIO_setAsInput
			}
#endif
			if (tmp107_rxcnt > i)
				count_ms = 0;
			i = tmp107_rxcnt;
			__delay_cycles(TMP107_Wait_ms);
			count_ms++;
		} else {
			count_ms = timeout_ms;
		}
	}
#ifdef TMP107_OneWireUART
	P1OUT &= ~1; //helps visualize direction of TX pin if monitored along side comm pins
	GPIO_setAsPeripheralModuleFunctionOutputPin(TMP107_Tx_Port,TMP107_Tx_Pin); // return pin to normal function
#endif
	return (tmp107_rxcnt - 1 - tx_size);
}

char TMP107_CheckEcho(char* tx_data, char tx_size){
	// examine rx buffer for proper echo of transmitted bytes
	char i;
	char retval = TMP107_Success;
	if (!(tmp107_rx[0] == 0x55))
		retval &= TMP107_EchoError;
	for (i = 0; i < tx_size; i++){
		if (!(tmp107_rx[i] == tx_data[i]))
			retval &= TMP107_EchoError;
	}
	return retval;
}

void TMP107_RetrieveReadback(char tx_size, char* rx_data, char rx_size){
	// copy bytes received from UART buffer to user supplied array
	char i;
	if (rx_size > 0){
		for (i = 0; i < rx_size; i++){
			rx_data[i] = tmp107_rx[1 + tx_size + i];
		}
	}
}

void TMP107_Wait(int wait_ms){
	// force delay for wait_ms number of milliseconds
	int i;
	for (i = 0; i < wait_ms; i++){
		__delay_cycles(TMP107_Wait_ms);
	}
}

// UART RX ISR
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	switch(__even_in_range(UCA0IV,4))
	{
	case USCI_UART_UCRXIFG:
		tmp107_rx[tmp107_rxcnt] = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
		tmp107_rxcnt++;
		break;
	default: break;
	}
}
