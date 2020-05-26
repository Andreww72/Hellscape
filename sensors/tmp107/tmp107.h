#include "hal.h"

#define TMP107_Timeout			40
#define TMP107_AddrInitTimeout	1250

/* Command Byte */
#define TMP107_Read_bit			0x2
#define TMP107_Global_bit		0x1

char TMP107_Init();
char TMP107_LastDevicePoll();
void TMP107_AlertOverClear();

float TMP107_DecodeTemperatureResult(int HByte, int LByte);
unsigned char TMP107_Encode5bitAddress(unsigned char addr);
unsigned char TMP107_Decode5bitAddress(unsigned char addr);
