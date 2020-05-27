#include "hal.h"

char temp[1];

UART_Handle initMotorUart() {
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.readMode = UART_MODE_BLOCKING;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readTimeout = TMP107_Timeout;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    UART_Handle uartMotor = UART_open(Board_UART7, &uartParams);

    if (uartMotor == NULL) {
        System_printf("Error opening the motor temp UART");
        System_flush();
    }
    return uartMotor;
}

void TMP107_Transmit(UART_Handle uartMotor, char* tx_data, char tx_size){
    UART_write(uartMotor, tx_data, tx_size);
}

void TMP107_WaitForEcho(UART_Handle uartMotor, char tx_size, char* rx_data, char rx_size){
	// Used after a call to Transmit, this function will exit once the transmit echo
	// and any additional rx bytes are received. it will also exit due to time out
	// if timeout_ms lapses without receiving new bytes.
    // Echo of cal byte + echo of transmission + additional bytes if read command
    char expected_rxcnt = tx_size + rx_size + 1; // +1 as stop bit seems to be echoed too
    char tmp107_rx[32]; // receive buffer: filled by UART

    UART_read(uartMotor, &tmp107_rx, expected_rxcnt);

	// Copy bytes received from UART buffer to user supplied array
	char i;
	if (rx_size > 0) {
		for (i = 0; i < rx_size; i++) {
			rx_data[i] = tmp107_rx[tx_size + i];
		}
	}
}
