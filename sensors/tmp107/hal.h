#ifndef HAL_H_
#define HAL_H_

#include <ti/drivers/UART.h>
#include <xdc/runtime/System.h>
#include <Board.h>

#define TMP107_Temp_reg 0xA0 // At 0h thus 1010PPPP=10100000
#define TMP107_Config_reg 0b10100001 // At 1h thus 1010PPPP=10100001
//#define TMP107_High_Limit_reg 1010 + 0x64
#define TMP107_Timeout 40

UART_Handle initMotorUart();

void TMP107_Transmit(UART_Handle uartMotor, char* tx_data, char tx_size);
void TMP107_WaitForEcho(UART_Handle uartMotor, char tx_size, char* rx_data, char rx_size);

#endif /* HAL_H_ */
