#include <signal_protocols/gpio.h>
#include <signal_protocols/usart.h>
#include "main.h"


#define CAMERA_TOGGLE_ON_OFF 1
#define CAMERA_START_RECORDING 2
#define CAMERA_STOP_RECORDING 3


bool camera_control(UART_HandleTypeDef *huart,int action);
int camera_remaining_recording_time(UART_HandleTypeDef *huart);
