#include "tmp107.h"

UART_Handle TMP107InitUart() {
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.readMode = UART_MODE_BLOCKING;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readTimeout = TMP107_TIMEOUT*2;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    UART_Handle uartTemp = UART_open(Board_UART7, &uartParams);

    if (uartTemp == NULL) {
        System_printf("Error opening the motor temp UART");
        System_flush();
    }
    return uartTemp;
}

char TMP107Init(UART_Handle uart) {
    // Initialise the chain and return the last device address
    char tx_size = 3;
    char rx_size = 1;
    char tx[3];
    char rx[1];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = 0x95; // AddressInit command code
    // Give the first device logical address 1
    tx[2] = 0x5 | TMP107Encode5bitAddress(0x1);
    TMP107Transmit(uart, tx, tx_size);
    TMP107WaitForEcho(uart, tx_size, rx, rx_size);

    // Must wait 1250ms after receiving last device response
    // during address init to write to eeprom.
    Task_sleep(TMP107_INIT_TIMEOUT);
    return rx[0] & 0xF8;
}

char TMP107LastDevicePoll(UART_Handle uart) {
    // Query the device chain to find the last device in the chain
    char tx_size = 2;
    char rx_size = 1;
    char tx[2];
    char rx[1];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = 0x57; // LastDevicePoll command code

    TMP107Transmit(uart, tx, tx_size);
    TMP107WaitForEcho(uart, tx_size, rx, rx_size);

    unsigned char retval = rx[0] & 0xF8; // Mask unused address bits, always 0b11
    return retval;
}

void TMP107SetConfig(UART_Handle uart, char addr) {
    // Setup half second conversions in config register
    char tx_size = 5;
    char tx[5];
    char rx[0];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = addr;
    tx[2] = TMP107_Config_reg; // Sets to 500ms conversions
    // 0b1000000000000000:
    tx[3] = 0;
    tx[4] = 0b10000000;

    TMP107Transmit(uart, tx, tx_size);
    TMP107WaitForEcho(uart, tx_size, rx, 0);
}

void TMP107Transmit(UART_Handle uart, char* tx_data, char tx_size){
    UART_write(uart, tx_data, tx_size);
}

void TMP107WaitForEcho(UART_Handle uart, char tx_size, char* rx_data, char rx_size){
    // Used after a call to Transmit, this function will exit once the transmit echo
    // and any additional rx bytes are received. it will also exit due to time out
    // if timeout_ms lapses without receiving new bytes.
    // Echo of cal byte + echo of transmission + additional bytes if read command
    char expected_rxcnt = tx_size + rx_size + 1; // +1 as stop bit seems to be echoed too
    char tmp107_rx[32]; // receive buffer: filled by UART

    UART_read(uart, &tmp107_rx, expected_rxcnt);

    // Copy bytes received from UART buffer to user supplied array
    char i;
    if (rx_size > 0) {
        for (i = 0; i < rx_size; i++) {
            rx_data[i] = tmp107_rx[tx_size + i];
        }
    }
}

float TMP107DecodeTemperatureResult(int HByte, int LByte) {
    // Convert raw byte response to floating point temperature
    int Bytes = HByte << 8 | LByte;
    Bytes = Bytes >> 2; // Get that 14 bit thing without the weird two bits on the right
    float temperature = (float) Bytes * TMP107_RESOLUTION;
    return temperature;
}

unsigned char TMP107Encode5bitAddress(unsigned char addr) {
	// Bit-reverse logical address to get TMP107-encoded address
	char i;
	unsigned char out = 0;

	for (i = 0; i < 5; i++) {
		if (addr & (1 << i)) {
			out |= 1 << (3+i);
		}
	}
	return out;
}

unsigned char TMP107Decode5bitAddress(unsigned char addr) {
	// bit-reverse TMP107-encoded address to get logical address
	char i;
	unsigned char out = 0;

	for (i = 3; i < 8; i++) {
		if (addr & (1 << i)) {
			out |= 1 << (i-3);
		}
	}
	return out;
}

uint8_t reverse8Bits(uint8_t num) {
    uint8_t count = sizeof(num) * 8 - 1;
    uint8_t reverse_num = num;
    num >>= 1;

    while(num) {
        reverse_num <<= 1;
        reverse_num |= num & 1;
        num >>= 1;
        count--;
    }
    reverse_num <<= count;
    return reverse_num;
}
