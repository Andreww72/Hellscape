#ifndef TMP107_H_
#define TMP107_H_

#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/UART.h>
#include <Board.h>

#define TMP107_Temp_reg         0b10100000 // At 0h thus PPPP0101 reversed 10100000
#define TMP107_Config_reg       0b10100001 // At 1h thus PPPP0101 reversed 10100001
#define TMP107_HL_reg           0b10100010 // At 2h thus PPPP1010 reversed 10100010
#define TMP107_TIMEOUT			40
#define TMP107_INIT_TIMEOUT     1250
#define TMP107_RESOLUTION       0.015625
#define TMP107_READ_BIT		    0x2
#define TMP107_GLBOAL_BIT	    0x1

// Setup UART7 for communication on the TMP107 daisy chain
UART_Handle TMP107InitUart();

// Initialise all TMP107s along the UART daisy chain
// Returns address of first device (our case board TMP107)
char TMP107Init(UART_Handle uart);

// Check chain initialised and return last device address (motor TMP107)
char TMP107LastDevicePoll(UART_Handle uart);

// Update TMP107 configs to 500ms conversions (default is 1s)
void TMP107SetConfig(UART_Handle uart, char addr);

// Transmit a message on the UART connection
// Message itself contains either global bit or specified TMP107
void TMP107Transmit(UART_Handle uart, char* tx_data, char tx_size);

// Waits for TMP107_TIMEOUT for a response and handles the transmit echo
void TMP107WaitForEcho(UART_Handle uart, char tx_size, char* rx_data, char rx_size);

// Converts a TMP107 register data response in temperature
float TMP107DecodeTemperatureResult(int HByte, int LByte);

// Some helper functions with bit reversals as LSB communication
unsigned char TMP107Encode5bitAddress(unsigned char addr);
unsigned char TMP107Decode5bitAddress(unsigned char addr);
uint8_t reverse8Bits(uint8_t num);

#endif /* TMP107_H_ */




