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
#include "delay.h"
//#include "glcd.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
#ifndef GLCD_H
#define GLCD_H

#include "stm32f4xx_hal.h"

// Define pins (adjust to your setup)
#define GLCD_DATA_PORT GPIOB
#define GLCD_CTRL_PORT GPIOA

#define RS_Pin GPIO_PIN_0
#define RW_Pin GPIO_PIN_1
#define EN_Pin GPIO_PIN_2
#define CS1_Pin GPIO_PIN_3
#define CS2_Pin GPIO_PIN_4
#define RST_Pin GPIO_PIN_5

void GLCD_Init(void);
void GLCD_Command(uint8_t cmd, uint8_t chip);
void GLCD_Data(uint8_t data, uint8_t chip);
void GLCD_Clear(void);
void GLCD_SetPage(uint8_t page, uint8_t chip);
void GLCD_SetColumn(uint8_t col, uint8_t chip);
void GLCD_WritePixel(uint8_t x, uint8_t y, uint8_t color);

#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "glcd.h"

static void GLCD_EnablePulse(void) {
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, EN_Pin, GPIO_PIN_RESET);
}

static void GLCD_Select(uint8_t chip) {
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, CS1_Pin, (chip == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, CS2_Pin, (chip == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GLCD_Command(uint8_t cmd, uint8_t chip) {
    GLCD_Select(chip);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RW_Pin, GPIO_PIN_RESET);
    GLCD_DATA_PORT->ODR = cmd;
    GLCD_EnablePulse();
}

void GLCD_Data(uint8_t data, uint8_t chip) {
    GLCD_Select(chip);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RW_Pin, GPIO_PIN_RESET);
    GLCD_DATA_PORT->ODR = data;
    GLCD_EnablePulse();
}

void GLCD_Init(void) {
    // Reset pulse
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GLCD_CTRL_PORT, RST_Pin, GPIO_PIN_SET);

    // Turn ON both halves
    GLCD_Command(0x3F, 1);
    GLCD_Command(0x3F, 2);
}

void GLCD_SetPage(uint8_t page, uint8_t chip) {
    GLCD_Command(0xB8 | (page & 0x07), chip);
}

void GLCD_SetColumn(uint8_t col, uint8_t chip) {
    GLCD_Command(0x40 | (col & 0x3F), chip);
}

void GLCD_Clear(void) {
    for (uint8_t chip = 1; chip <= 2; chip++) {
        for (uint8_t page = 0; page < 8; page++) {
            GLCD_SetPage(page, chip);
            GLCD_SetColumn(0, chip);
            for (uint8_t col = 0; col < 64; col++) {
                GLCD_Data(0x00, chip);
            }
        }
    }
}

void GLCD_WriteChar(uint8_t x, uint8_t page, char c) {
    uint8_t chip = (x < 64) ? 1 : 2;     // Left or right half
    uint8_t col = x % 64;
//    const uint8_t *bitmap = font5x7[c - 32];  // Font lookup

    GLCD_SetPage(page, chip);
    GLCD_SetColumn(col, chip);

    for (uint8_t i = 0; i < 5; i++) {
        if (col >= 64) {           // if cross boundary, switch chip
            chip++;
            col = 0;
            GLCD_SetPage(page, chip);
            GLCD_SetColumn(col, chip);
        }
        GLCD_Data(bitmap[i], chip);
        col++;
    }
    // One column spacing between characters
    if (col < 64) {
        GLCD_Data(0x00, chip);
    }
}

void GLCD_WriteString(uint8_t x, uint8_t page, const char *str) {
    while (*str) {
        GLCD_WriteChar(x, page, *str++);
        x += 6; // 5 pixels for char + 1 pixel space
        if (x >= 128) { // move to next line if needed
            x = 0;
            page++;
        }
    }
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
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
  GLCD_Init();
  GLCD_Clear();

  // Write string to top line (page 0)
  GLCD_WriteString(0, 0, "HELLO WORLD!");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
