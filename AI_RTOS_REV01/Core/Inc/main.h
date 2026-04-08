/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define ADC_DRDY_Pin GPIO_PIN_13
#define ADC_DRDY_GPIO_Port GPIOC
#define MUX_EN3_Pin GPIO_PIN_0
#define MUX_EN3_GPIO_Port GPIOC
#define ADC_CS_Pin GPIO_PIN_1
#define ADC_CS_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_4
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_5
#define LED3_GPIO_Port GPIOC
#define LED4_Pin GPIO_PIN_0
#define LED4_GPIO_Port GPIOB
#define LED5_Pin GPIO_PIN_1
#define LED5_GPIO_Port GPIOB
#define LED6_Pin GPIO_PIN_2
#define LED6_GPIO_Port GPIOB
#define LED7_Pin GPIO_PIN_10
#define LED7_GPIO_Port GPIOB
#define LED8_Pin GPIO_PIN_11
#define LED8_GPIO_Port GPIOB
#define MCP23_CS_Pin GPIO_PIN_12
#define MCP23_CS_GPIO_Port GPIOB
#define SPI2_MOSI_Pin GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB
#define MCP_BAUD2_Pin GPIO_PIN_7
#define MCP_BAUD2_GPIO_Port GPIOC
#define MCP_BAUD1_Pin GPIO_PIN_8
#define MCP_BAUD1_GPIO_Port GPIOC
#define MAX485_TX1_Pin GPIO_PIN_9
#define MAX485_TX1_GPIO_Port GPIOA
#define MAX485_RX1_Pin GPIO_PIN_10
#define MAX485_RX1_GPIO_Port GPIOA
#define MAX485_CTRL2_Pin GPIO_PIN_15
#define MAX485_CTRL2_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_10
#define LED1_GPIO_Port GPIOC
#define MAX485_CTRL1_Pin GPIO_PIN_12
#define MAX485_CTRL1_GPIO_Port GPIOC
#define MUX_S2_Pin GPIO_PIN_3
#define MUX_S2_GPIO_Port GPIOB
#define MUX_S1_Pin GPIO_PIN_4
#define MUX_S1_GPIO_Port GPIOB
#define MUX_S0_Pin GPIO_PIN_5
#define MUX_S0_GPIO_Port GPIOB
#define ADS_RESET_Pin GPIO_PIN_6
#define ADS_RESET_GPIO_Port GPIOB
#define MUX_EN1_Pin GPIO_PIN_7
#define MUX_EN1_GPIO_Port GPIOB
#define MUX_EN2_Pin GPIO_PIN_8
#define MUX_EN2_GPIO_Port GPIOB
#define MCP23_RST1_Pin GPIO_PIN_9
#define MCP23_RST1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
