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

void TMP107_Set_Config(UART_Handle uartMotor, char motorAddr) {
    // Setup half second conversions in config register
    char tx_size = 5;
	char tx[5];
    char rx[0];

	tx[0] = 0x55; // Calibration Byte
    tx[1] = motorAddr;
	tx[2] = TMP107_Config_reg;
	// Set defaults except change to 500ms conversions
	// 1000000000000000
	tx[3] = 0; //0b00000000
	tx[4] = 0b10000000;
    TMP107_Transmit(uartMotor, tx, tx_size);
    TMP107_WaitForEcho(uartMotor, tx_size, rx, 0);
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

float TMP107_DecodeTemperatureResult(int HByte, int LByte) {
    // Convert raw byte response to floating point temperature
    int Bytes = HByte << 8 | LByte;
    Bytes = Bytes >> 2; // Get that 14 bit thing without the weird two bits on the right
    float temperature = (float) Bytes * TMP107_RESOLUTION;
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

