#include "tmp107.h"

char TMP107_Init(UART_Handle uartMotor) {
	// Initialise the chain and return the last device address
    char tx_size = 3;
    char rx_size = 1;
    char tx[3];
	char rx[1];

	tx[0] = 0x55; // Calibration Byte
	tx[1] = 0x95; // AddressInit command code
	// Give the first device logical address 1
	tx[2] = 0x5 | TMP107_Encode5bitAddress(0x1);
	TMP107_Transmit(uartMotor, tx, tx_size);
    /* Must wait 1250ms after receiving last device response
     * during address init operation. This is not baud rate
     * dependent. This is because address init writes to the
     * internal eeprom, which takes additional time.
     */
	TMP107_WaitForEcho(uartMotor, tx_size, rx, rx_size);
	Task_sleep(TMP107_AddrInitTimeout);

    return rx[0] & 0xF8;
}

char TMP107_LastDevicePoll(UART_Handle uartMotor) {
    // Query the device chain to find the last device in the chain
    char tx_size = 2;
    char rx_size = 1;
    char tx[2];
    char rx[1];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = 0x57; // LastDevicePoll command code
    TMP107_Transmit(uartMotor, tx, tx_size);
    TMP107_WaitForEcho(uartMotor, tx_size, rx, rx_size);
    unsigned char retval = rx[0] & 0xF8; // mask the unused address bits that are always 0b11
    return retval;
}

void TMP107_AlertOverClear(UART_Handle uartMotor) {
    // clear all Alert2
    char tx_size = 2;
    char rx_size = 0;
    char tx[2];
    char rx[0];

    tx[0] = 0x55; // Calibration Byte
    tx[1] = 0x75; // GlobalAlertClear2 command code
    TMP107_Transmit(uartMotor, tx, tx_size);
    TMP107_WaitForEcho(uartMotor, tx_size, rx, rx_size);
    // no need to RetrieveReadback
}

float TMP107_DecodeTemperatureResult(int HByte, int LByte) {
    // Convert raw byte response to floating point temperature
    int Bytes = HByte << 8 | LByte;
    Bytes &= 0xFFFC; // Mask NVM bits not used in Temperature Result
    float temperature = (float) Bytes / 256;
    return temperature;
}

unsigned char TMP107_Encode5bitAddress(unsigned char addr) {
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

unsigned char TMP107_Decode5bitAddress(unsigned char addr) {
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
