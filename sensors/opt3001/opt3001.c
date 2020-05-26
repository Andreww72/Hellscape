#include "opt3001.h"

#define REG_MANUFACTURER_ID             0x7E
#define REG_DEVICE_ID                   0x7F

/* Register values */
#define MANUFACTURER_ID                 0x5449  // TI
#define DEVICE_ID                       0x3001  // Opt 3001
#define CONFIG_RESET                    0xC810
#define CONFIG_TEST                     0xCC10
#define CONFIG_ENABLE                   0x10C4 // 0xC410   - 100 ms, continuous       0b0001000011000100
#define CONFIG_DISABLE                  0x10C0 // 0xC010   - 100 ms, shutdown

/* Bit values */
#define DATA_RDY_BIT                    0x0080  // Data ready
#define REGISTER_LENGTH                 2
#define DATA_LENGTH                     2

/**************************************************************************************************
 * @fn          sensorOpt3001Init
 *
 * @brief       Initialize the temperature sensor driver
 *
 * @return      none
 **************************************************************************************************/
bool sensorOpt3001Init(I2C_Handle i2c) {
	sensorOpt3001Enable(i2c, false);
	return (true);
}


/**************************************************************************************************
 * @fn          sensorOpt3001Enable
 *
 * @brief       Turn the sensor on/off
 *
 * @return      none
 **************************************************************************************************/
void sensorOpt3001Enable(I2C_Handle i2c, bool enable) {
	uint16_t val_config;
	uint16_t val_low;
	uint16_t val_high;

	if (enable) {
		val_config = 0b0001000011000100;
		val_low = 0b1111111100001111;

		// Eq 1: Lux = 0.01 x 2^(exponent_value) x result_value
		// Exponent value of 10 (0b1010)
		// Result value of 245 (0b000011110101)
		val_high = 0b1111010110100000; // Results ~2509 lux
	} else {
	    val_config = CONFIG_DISABLE;
	    val_low = 0;
		val_high = 0;
	}

	// Write to configuration register 01h
	writeI2C(i2c, OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val_config);

	// Write to low-limit register 02h
	writeI2C(i2c, OPT3001_I2C_ADDRESS, REG_LOW_LIMIT, (uint8_t*)&val_low);

	// Write to high-limit register 03h
	writeI2C(i2c, OPT3001_I2C_ADDRESS, REG_HIGH_LIMIT, (uint8_t*)&val_high);
}


/**************************************************************************************************
 * @fn          sensorOpt3001Read
 *
 * @brief       Read the result register
 *
 * @param       Buffer to store data in
 *
 * @return      TRUE if valid data
 **************************************************************************************************/
bool sensorOpt3001Read(I2C_Handle i2c, uint16_t *rawData) {

	bool success;
	uint16_t val;

	success = readI2C(i2c, OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t *)&val);

	if (success)
	{
		success = (val & DATA_RDY_BIT) == DATA_RDY_BIT;
	}

	if (success)
	{
		success = readI2C(i2c, OPT3001_I2C_ADDRESS, REG_RESULT, (uint8_t *)&val);
	}

	if (success)
	{
		// Swap bytes
		*rawData = (val << 8) | (val>>8 &0xFF);
	}
	else
	{
		//	  sensorSetErrorData
	}

	return (success);
}

/**************************************************************************************************
 * @fn          sensorOpt3001Test
 *
 * @brief       Run a sensor self-test
 *
 * @return      TRUE if passed, FALSE if failed
 **************************************************************************************************/
bool sensorOpt3001Test(I2C_Handle i2c) {
	uint16_t val;

	// Check manufacturer ID
	readI2C(i2c, OPT3001_I2C_ADDRESS, REG_MANUFACTURER_ID, (uint8_t *)&val);
	val = (val << 8) | (val>>8 &0xFF);

	if (val != MANUFACTURER_ID)
	{
		return (false);
	}

	// Check device ID
	readI2C(i2c, OPT3001_I2C_ADDRESS, REG_DEVICE_ID, (uint8_t *)&val);
	val = (val << 8) | (val>>8 &0xFF);

	if (val != DEVICE_ID)
	{
		return (false);
	}

	return (true);
}

/**************************************************************************************************
 * @fn          sensorOpt3001Convert
 *
 * @brief       Convert raw data to object and ambience temperature
 *
 * @param       rawData - raw data from sensor
 *
 * @param       convertedLux - converted value (lux)
 *
 * @return      none
 **************************************************************************************************/
void sensorOpt3001Convert(uint16_t rawData, float *convertedLux) {
	uint16_t e, m;

	m = rawData & 0x0FFF;
	e = (rawData & 0xF000) >> 12;

	*convertedLux = m * (0.01 * exp2(e));
}
