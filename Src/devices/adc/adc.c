#include "adc.h"

uint8_t convert_config2byte(uint8_t ch)
{
    uint8_t byte = PGA_GAIN_1;
	byte |= SAMPLE_RATE_15SPS_16BIT << 2;
	byte |= CONV_MODE_ONE_SHOT << 4; //CONV_MODE_ONE_SHOT
	byte |= ch << 5;

	return byte;
}

/**
 * read ADC value
 * @param I2C handle
 * @note PGA_Gain_1, 16Bit, One Shot Mode
 * @return value
 */
float MCP342X_adc_read_16bit(I2C_HandleTypeDef *hi2c,uint8_t slave_address,uint8_t ch) {
	float adc_output = 0.f;
	uint8_t buf[5] = { };
	uint8_t config_byte = convert_config2byte(ch);
	buf[0] = config_byte + RDY_BIT;

	uint32_t timeout = 1000U;   // timeout

	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hi2c, slave_address << 1, (uint8_t*) &buf, 1, 100);

	 do {
		 HAL_StatusTypeDef status1 = HAL_I2C_Master_Receive(hi2c, slave_address << 1, (uint8_t*) &buf, 3, 100);

	        if ((buf[2] & RDY_BIT) == 0) {
	            break;      // end of conversion
	        }
	        if (--timeout == 0) {
	            return 0;  // error
	        }
	        uint8_t spd = (buf[2] >> 2) & 0x03;
	        if (spd == SAMPLE_RATE_60SPS_14BIT) {
	        	HAL_Delay(6);     // conversion time = 16.7ms
	        } else if (spd == SAMPLE_RATE_15SPS_16BIT) {
	        	HAL_Delay(24);    // conversion time = 66.7ms
	        } else {  // == SAMPLE_RATE_240SPS_12BIT
	        	HAL_Delay(2);     // no wait
	        }

	    } while(true);

	int16_t adc16 =(uint16_t)buf[1] + ((uint16_t)buf[0] << 8);
	adc_output = ((float) (adc16) / 32768.0f) * 2.048f;

	return adc_output;
}
