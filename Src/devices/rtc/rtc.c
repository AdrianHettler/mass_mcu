#include "rtc.h"




/**
 * Conversion of BCD time format to integer
 * @params byte read from rtc register
 * @return time in decimal
 */
uint8_t bcd_to_decimal(unsigned char x) {
    return (uint8_t)(x - 6 * (x >> 4));
}


/**
 * Resets the RTC
 * @params I2C handle
 * @note time values get zeroed, clock is set to stopped state
 * @return status
 */
bool rtc_reset(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t reset_register[8] = {0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0}; //register zeroed, 8th bit of first register set to 1
	status = HAL_I2C_Master_Transmit(hi2c, RTC_SLAVE_ADDRESS << 1, (uint8_t*)&reset_register, 8, 100);

	if(status == HAL_OK)
		return true;

	return false;
}

/**
 * Starts the RTC
 * @params I2C handle
 * @return status
 */
bool rtc_start(I2C_HandleTypeDef *hi2c)
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t start_register[8] = { 0x0, 0x0 }; //8th bit of first register (CH) set to 0
	status = HAL_I2C_Master_Transmit(hi2c, RTC_SLAVE_ADDRESS << 1, (uint8_t*)&start_register, 2, 100);

	if (status == HAL_OK)
		return true;

	return false;
}

/**
 * Stops the RTC
 * @params I2C handle
 * @return status
 */
bool rtc_stop(I2C_HandleTypeDef *hi2c) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t stop_register[8] = { 0x0, 0x80}; //8th bit of first register (CH) set to 1
	status = HAL_I2C_Master_Transmit(hi2c, RTC_SLAVE_ADDRESS << 1, (uint8_t*)&stop_register, 2, 100);

	if (status == HAL_OK)
		return true;

	return false;
}

/**
 * Reads time from RTC
 * @params I2C handle
 * @return time "0,hour_byte,minute_byte,second_byte"
 */
int rtc_get_time(I2C_HandleTypeDef *hi2c) {
	uint8_t buffer[5] = {};
	uint8_t address = 0x0;

	HAL_I2C_Master_Transmit(hi2c, RTC_SLAVE_ADDRESS << 1, &address, 1, 100);
	HAL_I2C_Master_Receive(hi2c, RTC_SLAVE_ADDRESS << 1, (uint8_t*)&buffer, 3, 100);

	return ((bcd_to_decimal(buffer[2]) & 0xFF) << 16)
			| ((bcd_to_decimal(buffer[1]) & 0xFF) << 8)
			| ((bcd_to_decimal(buffer[0]) & 0xFF) << 0);
}

/**
 * Reads time from RTC
 * @params I2C handle
 * @return string "hh:mm:ss"
 */
char time_str[20] = {};
char* rtc_get_time_string(I2C_HandleTypeDef *hi2c)
{
	int time = rtc_get_time(hi2c);

	sprintf(time_str,"%02d:%02d:%02d",(int)(uint8_t)(time >> 16),(int)(uint8_t)(time >> 8),(int)(uint8_t)(time >> 0));
	return (char*)&time_str;
}


