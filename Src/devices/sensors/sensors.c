#include "sensors.h"

/**
 * Reads the Temperature of MCP9804 digital temperature sensors
 * @params I2C handle
 * @note slave address hardcoded to 0x18 -> A0,A1,A2 connected to GND
 * @return temperature in °C
 */
float MCP9804_temperature_read(I2C_HandleTypeDef *hi2c) {
	char temp_buffer[10] = {0};
	float temperature;

	uint8_t temp_register_ptr = 0x5;

	HAL_I2C_Master_Transmit(&hi2c, MCP9804_TEMP_SENSOR_SLAVE_ADDRESS << 1, &temp_register_ptr, 1, 100);
	HAL_I2C_Master_Receive(&hi2c, MCP9804_TEMP_SENSOR_SLAVE_ADDRESS << 1, &temp_buffer, 2, 100);


	uint8_t upper_byte = temp_buffer[0];
	uint8_t lower_byte = temp_buffer[1];

	if ((upper_byte & 0x80) == 0x80) { //TA ≥ TCRIT
	}
	if ((upper_byte & 0x40) == 0x40) { //TA > TUPPER
	}
	if ((upper_byte & 0x20) == 0x20) { //TA < TLOWER
	}

	upper_byte = upper_byte & 0x1F; //Clear flag bits

	if ((upper_byte & 0x10) == 0x10) { //TA < 0°C
		upper_byte = upper_byte & 0x0F;
		//Clear SIGN
		temperature = 256.f - ((float) upper_byte * 16.f + (float) lower_byte / 16.f);
	}
	else //TA ≥ 0°C
	{
		temperature = ((float) upper_byte * 16.f + (float) lower_byte / 16.f);
	}

	return temperature;
}


/**
 * Reads the current of ACS712 current sensors
 * @params I2C handle
 * @note 100mV/A, 2x Voltage divider, zero at 2.461
 * @return Current in Ampere
 */

float ACS712_current_read(I2C_HandleTypeDef *hi2c) {
#define sensitivity 0.1f
#define zero_current_output 2.461f

	float adc_out = MCP3425_adc_read_16bit(hi2c);
	return ((adc_out * 2.f) - zero_current_output) / sensitivity;
}
