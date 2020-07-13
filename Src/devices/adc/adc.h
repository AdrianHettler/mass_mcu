#include <signal_protocols/gpio.h>
#include <signal_protocols/i2c.h>
#include "main.h"

#define RDY_BIT                     (1UL << 7)
#define PGA_GAIN_1                  0
#define PGA_GAIN_2                  1
#define PGA_GAIN_4                  2
#define PGA_GAIN_8                  3

#define CONV_MODE_CONTINUOUS        1
#define CONV_MODE_ONE_SHOT          0

#define SAMPLE_RATE_240SPS_12BIT    0
#define SAMPLE_RATE_60SPS_14BIT     1
#define SAMPLE_RATE_15SPS_16BIT     2

#define CHANNEL_1 0
#define CHANNEL_2 1
#define CHANNEL_3 2
#define CHANNEL_4 3

#define MCP3425A0_ADC_SLAVE_ADRESS 0x6E



uint8_t convert_config2byte(uint8_t ch);
float MCP342X_adc_read_16bit(I2C_HandleTypeDef *hi2c,uint8_t slave_address,uint8_t ch);
