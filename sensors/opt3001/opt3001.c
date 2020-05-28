#include "opt3001.h"

#define REG_MANUFACTURER_ID             0x7E
#define REG_DEVICE_ID                   0x7F

/* Register values */
#define MANUFACTURER_ID                 0x5449  // TI
#define DEVICE_ID                       0x3001  // Opt 3001
#define CONFIG_RESET                    0xC810
#define CONFIG_TEST                     0xCC10
#define CONFIG_ENABLE                   0xC410 // 0xC410   - 100 ms, continuous
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
bool sensorOpt3001Init() {
	sensorOpt3001Enable(false);
	return true;
}


/**************************************************************************************************
 * @fn          sensorOpt3001Enable
 *
 * @brief       Turn the sensor on/off
 *
 * @return      none
 **************************************************************************************************/
void sensorOpt3001Enable(bool enable) {
	uint16_t val_config;

	// Starts as 0b11001 00 000010000
	// Enable by 0b11001110 00010000 MSB but swap order of bytes
	if (enable) {
		val_config = 0b0001000011001110;
	} else {
	    val_config = CONFIG_DISABLE;
	}

	// Write to configuration register 01h
	writeI2C(OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t*)&val_config);
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
bool sensorOpt3001Read(uint16_t *rawData) {
	bool success;
	uint16_t val;

	success = readI2C(OPT3001_I2C_ADDRESS, REG_CONFIGURATION, (uint8_t *)&val);

	if (success) {
		success = (val & DATA_RDY_BIT) == DATA_RDY_BIT;
	}

	if (success) {
		success = readI2C(OPT3001_I2C_ADDRESS, REG_RESULT, (uint8_t *)&val);
	}

	if (success) {
		// Swap bytes
		*rawData = (val << 8) | (val>>8 &0xFF);
	}

	return success;
}

/**************************************************************************************************
 * @fn          sensorOpt3001Test
 *
 * @brief       Run a sensor self-test
 *
 * @return      TRUE if passed, FALSE if failed
 **************************************************************************************************/
bool sensorOpt3001Test() {
	uint16_t val;

	// Check manufacturer ID
	readI2C(OPT3001_I2C_ADDRESS, REG_MANUFACTURER_ID, (uint8_t *)&val);
	val = (val << 8) | (val>>8 &0xFF);

	if (val != MANUFACTURER_ID) {
		return false;
	}

	// Check device ID
	readI2C(OPT3001_I2C_ADDRESS, REG_DEVICE_ID, (uint8_t *)&val);
	val = (val << 8) | (val>>8 &0xFF);

	if (val != DEVICE_ID) {
		return false;
	}

	return true;
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
