#ifndef TMP107_H_
#define TMP107_H_

#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/System.h>
#include "hal.h"

#define TMP107_Timeout			40
#define TMP107_AddrInitTimeout	1250
#define TMP107_RESOLUTION       0.015625

/* Command Byte */
#define TMP107_Read_bit			0x2
#define TMP107_Global_bit		0x1

char TMP107_Init(UART_Handle uartMotor);
void TMP107_Set_Config(UART_Handle uartMotor, char motorAddr);
char TMP107_LastDevicePoll(UART_Handle uartMotor);
void TMP107_AlertOverClear(UART_Handle uartMotor);

float TMP107_DecodeTemperatureResult(int HByte, int LByte);
unsigned char TMP107_Encode5bitAddress(unsigned char addr);
unsigned char TMP107_Decode5bitAddress(unsigned char addr);
uint8_t reverse8Bits(uint8_t num);

#endif /* TMP107_H_ */
