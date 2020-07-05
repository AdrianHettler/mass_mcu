#include <signal_protocols/gpio.h>
#include <signal_protocols/i2c.h>
#include "main.h"

#define RTC_SLAVE_ADDRESS 0x68


int rtc_get_time(I2C_HandleTypeDef *hi2c);
char* rtc_get_time_string(I2C_HandleTypeDef *hi2c);

bool rtc_reset(I2C_HandleTypeDef *hi2c);
bool rtc_start(I2C_HandleTypeDef *hi2c);
bool rtc_stop(I2C_HandleTypeDef *hi2c);
