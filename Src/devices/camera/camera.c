#include "camera.h"

/**
 * Calculates checksum required for uart communication
 * @param request packet, length of request packet
 * @return time in seconds
 */

uint8_t calc_checksum(uint8_t* array,int size)
{
	uint8_t crc = 0x0;

	for(int i = 0; i< size-1;i++)
	{
	    crc ^= array[i];
	    for (int ii = 0; ii < 8; ++ii) {
	        if (crc & 0x80) {
	            crc = (crc << 1) ^ 0xD5;
	        } else {
	            crc = crc << 1;
	        }
	    }
	}
	return crc;
}

/**
 * Gets the remaining recoring time
 * @note time is saved as null terminated string in EPROM memory of camera
 *
 * @return time in seconds
 */
int camera_remaining_recording_time(UART_HandleTypeDef *huart)
{
	uint8_t out[100] = {0};
	uint8_t data[5] = { 0xCC, 0x10, 0x4, 0x2, 0x0 }; //request packet
	data[4] = calc_checksum(&data, sizeof(data));

	HAL_UART_Transmit(huart,&data, 5, 100);
	HAL_UART_Receive(huart,&out, 30, 100);

	int time_digits = 0;
	for (int i = 0;i < 5; i++){  //get digits of number, max 5

		if (out[19 + i] == '\0') //check for null termination
			break;
		time_digits++;
	}

	int time_remaining = 0;
	for (int i = 0; i < time_digits; i++) { //calculate number from chars
		int x = out[19 + i] - '0'; //ascii to integer

		time_remaining += x * pow(10, abs(i - 4));
	}

	return time_remaining;
}



/**
 * Control camera
 * @param UART handle, Action to execute
 * @return status
 */
bool camera_control(UART_HandleTypeDef *huart, int action) {

	HAL_StatusTypeDef status = HAL_OK;

	switch (action) {
	case CAMERA_TOGGLE_ON_OFF:
		;
		uint8_t data_toggle_power[] = { 0xCC, 0x1, 0x1, 0x0 };
		data_toggle_power[3] = calc_checksum(&data_toggle_power, sizeof(data_toggle_power));
		status = HAL_UART_Transmit(huart, (uint8_t*) &data_toggle_power, 4, 100);
		break;

	case CAMERA_START_RECORDING:
		;
		uint8_t data_start[] = { 0xCC, 0x1, 0x3, 0x0 };
		data_start[3] = calc_checksum(&data_start, sizeof(data_start));
		status = HAL_UART_Transmit(huart, (uint8_t*) &data_start, 4, 100);
		break;

	case CAMERA_STOP_RECORDING:
		;
		uint8_t data_stop[] = { 0xCC, 0x1, 0x4, 0x0 };
		data_stop[3] = calc_checksum(&data_stop, sizeof(data_stop));
		status = HAL_UART_Transmit(huart, (uint8_t*) &data_stop, 4, 100);
		break;

	default:
		break;
	}

	if (status == HAL_OK)
			return true;

		return false;
}
