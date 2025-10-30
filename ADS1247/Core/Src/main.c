/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "math.h"
#include "ADS1247.h"


uint8_t status;
int32_t raw;
double voltage;
double result_temp=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void test_RTD(){
	static double R0 =  100;//0 degree for 0 ohm
	static double A =  0.00390802;//by IEC-60751
	static double B = -0.0000005802;
	adc124xx_t ads={
			.sel_channel_P=P_AIN0,
			.sel_channel_N=N_AIN1,
			.sel_bias=EN_BIAS_AIN0,
			.sel_dr=DOR3_5,
			.sel_exc_mag=IMAG2_OFF,
			.sel_exc_out=I1DIR_OFF|I2DIR_OFF,
			.sel_internal_ref=VREFCON1_ON,
			.sel_ref=REFSELT1_ON,
			.sel_pga=PGA2_0,
			.sel_sys_moniter=MUXCAL2_NORMAL
	};
	rtd_init(&ads);
	raw =  ADS1247_ReadData();
	double result_volt=ads1247_raw_to_voltage(raw, 2.048, 1);
	double result_resistance = result_volt/0.002; //
	result_temp = (-R0*A+sqrt(pow(R0*A,2)-4*R0*B*(R0-result_resistance)))/(2*R0*B);
	//Define PT100 Coeffients

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	char *data="UART2 TESTING\n\r";

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	HAL_Init();

  /* USER CODE BEGIN Init */
 // uint8_t temp[2];
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  ADS1247_begin();




  while (1)
  {
	 status=HAL_GPIO_ReadPin(DRDY_PORT, DRDY_PIN);
    if (HAL_GPIO_ReadPin(DRDY_PORT, DRDY_PIN) == GPIO_PIN_RESET)
    {
//    	raw =  ADS1247_ReadData();
//    	voltage = (2.048 / 16777216)*raw;

      raw = ADS1247_ReadData();
     // result_temp =ads1247_raw_to_voltage(raw, 2.048,1);
      result_temp =ads1247_raw_to_voltage(raw, 2.96,1);

//      HAL_Delay(1000);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
