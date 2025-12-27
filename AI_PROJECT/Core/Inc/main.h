/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "modbus485.h"
#include "dipswitch.h"
#include "ADS1247.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED6_Pin GPIO_PIN_13
#define LED6_GPIO_Port GPIOC
#define LED5_Pin GPIO_PIN_0
#define LED5_GPIO_Port GPIOC
#define LED4_Pin GPIO_PIN_1
#define LED4_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_2
#define LED3_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_0
#define LED1_GPIO_Port GPIOA
#define ADC_CS1_Pin GPIO_PIN_4
#define ADC_CS1_GPIO_Port GPIOA
#define MCP_CS2_Pin GPIO_PIN_12
#define MCP_CS2_GPIO_Port GPIOB
#define BAUD2_Pin GPIO_PIN_7
#define BAUD2_GPIO_Port GPIOC
#define BAUD1_Pin GPIO_PIN_8
#define BAUD1_GPIO_Port GPIOC
#define CTRL2_Pin GPIO_PIN_15
#define CTRL2_GPIO_Port GPIOA
#define CTRL0_Pin GPIO_PIN_10
#define CTRL0_GPIO_Port GPIOC
#define DRDY_Pin GPIO_PIN_11
#define DRDY_GPIO_Port GPIOC
#define DRDY_EXTI_IRQn EXTI4_15_IRQn
#define ADS_RESET_Pin GPIO_PIN_6
#define ADS_RESET_GPIO_Port GPIOB
#define MCP_RST1_Pin GPIO_PIN_9
#define MCP_RST1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
