#include "adc.h"

uint8_t convert_config2byte()
{
    uint8_t byte = PGA_GAIN_1;
	byte |= SAMPLE_RATE_15SPS_16BIT << 2;
	byte |= CONV_MODE_ONE_SHOT << 4;
	return byte;
}

/**
 * read ADC value
 * @param I2C handle
 * @note PGA_Gain_1, 16Bit, One Shot Mode
 * @return value
 */
float MCP3425_adc_read_16bit(I2C_HandleTypeDef *hi2c) {
	float adc_output = 0.f;
	uint8_t buf[5] = { };
	uint8_t config_byte = convert_config2byte();
	buf[0] = config_byte + RDY_BIT;

	HAL_I2C_Master_Transmit(hi2c, MCP3425A0_ADC_SLAVE_ADRESS << 1, (uint8_t*) &buf, 1, 100);
	HAL_I2C_Master_Receive(hi2c, MCP3425A0_ADC_SLAVE_ADRESS << 1, (uint8_t*) &buf, 3, 100);

	int16_t adc16 =(uint16_t)buf[1] + ((uint16_t)buf[0] << 8);
	adc_output = ((float) (adc16) / 32768.0f) * 2.048f;

	return adc_output;
}
