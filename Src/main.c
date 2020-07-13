/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "i2c.h"
#include "lwip.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "message_ids.h"
#include "devices/adc/adc.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


int get_free_sd_card_space()
{
	FATFS *pfs;
	DWORD fre_clust;
	FATFS fs;  // file system

	FRESULT fresult = f_mount(&fs, "", 0); //mount sd card

	if (fresult == FR_OK)
	{
		f_getfree("", &fre_clust, &pfs);

		fresult = f_mount(NULL, "", 1);
		return  (uint32_t)(fre_clust * pfs->csize * 0.5);
	}

	return 0;
}

int bufsize (char *buf)
{
	int i=0;
	while (*buf++ != '\0') i++;
	return i;
}


FRESULT write_sd_card(char* file_name,void* src,int size)
{
	FILINFO filinfo;
	FATFS fs;  // file system
	FIL fil;  // file
	UINT br, bw;   // file read/write count

	FRESULT fresult = f_mount(&fs, "", 0); //mount sd card

	if (fresult == FR_OK)
	{
		fresult = f_open(&fil, file_name, FA_OPEN_ALWAYS | FA_READ | FA_WRITE); //open file, if it doesnt exist create the file
		fresult = f_stat(file_name, &filinfo); //read fileinfo
		fresult = f_lseek(&fil,filinfo.fsize); //offset to the end of file
		fresult = f_write(&fil, src, size, &bw);

		f_close(&fil);
		fresult = f_mount(NULL, "", 1);
	}

	return fresult;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

float rd(float value)
{
	int val = (int)(value * 100.f);
	return ((float)(val))/100;
}



int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LWIP_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_FATFS_Init();
  MX_I2C2_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
	tcp_echoserver_init();
	udp_echoclient_connect();

 int total = get_free_sd_card_space();
 char buffer[300] = {};
  sprintf(buffer,"HELLO FREE SPACEAAA : %d",total);


  strcpy (buffer, "This is a new test- and it says Hello from controllerstech newzufutuzfggzuizguigzuzugzugguzzgugzuuzggzu\n");


  write_sd_card("FILE55.txt",&buffer,bufsize(buffer));

	uint32_t ms_start_pneu = HAL_GetTick();
	uint32_t ms_start_pwr = HAL_GetTick();
	uint32_t ms_start_amb = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */



  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		MX_LWIP_Process();

		if ((HAL_GetTick() - ms_start_pneu) >= 10000) //every 1 sec.
				{
			//pneumatics data
			struct pneumatics pneu;

			pneu.pressure_tank = ((((MCP342X_adc_read_16bit(&hi2c1, 0x68,CHANNEL_2) * (125.f / 75.f)) - 0.25f) * 150) / (0.9f * 5.f))* 68.9476f;
			pneu.pressure_outside_structures = 1200.8f;
			pneu.pressure_inside_structures = 1178.6f;
			udp_echoclient_send(data_down_pneumatics, &pneu, sizeof(pneu));
			ms_start_pneu = HAL_GetTick();

		}

		if ((HAL_GetTick() - ms_start_pwr) >= 10000) //every 10 sec.
				{
			///power data
			struct power pwr;
			pwr.voltage_bexus = rd(MCP342X_adc_read_16bit(&hi2c1,I2C_ADRESS_POWER_ADC, CHANNEL_4) * 13.f);
			pwr.voltage_extra = rd(MCP342X_adc_read_16bit(&hi2c1,I2C_ADRESS_POWER_ADC, CHANNEL_3) * 13.f);
#define sensitivity 0.185f
			pwr.current_bexus = rd(((MCP342X_adc_read_16bit(&hi2c1,I2C_ADRESS_POWER_ADC, CHANNEL_1) * 2.f) - 2.426f) / sensitivity);
			pwr.current_extra = rd(((MCP342X_adc_read_16bit(&hi2c1,I2C_ADRESS_POWER_ADC, CHANNEL_2) * 2.f) - 2.4328f) / sensitivity);
			udp_echoclient_send(data_down_power, &pwr, sizeof(pwr));
			ms_start_pwr = HAL_GetTick();

		}

		if ((HAL_GetTick() - ms_start_amb) >= 10000) //every 60 sec.
				{
			//enviromental data
			struct enviromental env;
			env.pressure = 50.f;

			float v_temp_inside = MCP342X_adc_read_16bit(&hi2c1,0x69, CHANNEL_2);
			env.temp_inside = (1.f/((1.f/298.15f) + (1.f/3492.f) * log((v_temp_inside*10000)/(3.3f-v_temp_inside)/10000)))-273.15f;



			env.temp_outside = 70.f;
			udp_echoclient_send(data_down_enviromental, &env, sizeof(env));
			ms_start_amb = HAL_GetTick();
		}

		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {


			HAL_Delay(50);

		}

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure LSE Drive Capability 
  */
  HAL_PWR_EnableBkUpAccess();
  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2;
  PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
