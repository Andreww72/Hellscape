#include "tmp107.h"

char TMP107_Init() {
	// Initialise the chain and return the last device address
	char data[2];
	char rx[1];
	char rx_size;

	data[0] = 0x95; // AddressInit command code
	data[1] = 0x5 | TMP107_Encode5bitAddress(0x1); // give the first device logical address 1
	TMP107_Transmit(data, 2);
    /* Must wait 1250ms after receiving last device response
     * during address init operation. This is not baud rate
     * dependent. This is because address init writes to the
     * internal eeprom, which takes additional time.
     */
	rx_size = TMP107_WaitForEcho(2, 1, TMP107_AddrInitTimeout);
	TMP107_RetrieveReadback(2, rx, rx_size);

    return rx[0] & 0xF8;
}

char TMP107_LastDevicePoll() {
    // Query the device chain to find the last device in the chain
    char tx[1];
    char rx[1];
    unsigned char retval;
    tx[0] = 0x57; // LastDevicePoll command code
    TMP107_Transmit(tx, 1);
    TMP107_WaitForEcho(1, 1, TMP107_Timeout); // normal timeout
    TMP107_RetrieveReadback(1, rx, 1);
    retval = rx[0] & 0xF8; // mask the unused address bits that are always 0b11
    return retval;
}

void TMP107_AlertOverClear() {
    // clear all Alert2
    char tx[1];
    tx[0] = 0x75; // GlobalAlertClear2 command code
    TMP107_Transmit(tx, 1);
    TMP107_WaitForEcho(1, 0, TMP107_Timeout);
    // no need to RetrieveReadback
}

float TMP107_DecodeTemperatureResult(int HByte, int LByte) {
    // Convert raw byte response to floating point temperature
    int Bytes;
    float temperature;
    Bytes = HByte << 8 | LByte;
    Bytes &= 0xFFFC; // Mask NVM bits not used in Temperature Result
    temperature = (float) Bytes / 256;
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
