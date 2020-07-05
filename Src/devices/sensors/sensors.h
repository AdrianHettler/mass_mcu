#include <signal_protocols/gpio.h>
#include <signal_protocols/i2c.h>
#include "main.h"
#include "../adc/adc.h"

#define MCP9804_TEMP_SENSOR_SLAVE_ADDRESS 0x18 // 11000


float MCP9804_temperature_read(I2C_HandleTypeDef *hi2c);
float ACS712_current_read(I2C_HandleTypeDef *hi2c);

