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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "lwrb.h"
#include "modbus.h"
#include "cmsis_os.h"
#include "ADC.h"
#include "common.h"
#include "COM_PORT_RS232_1.h"
#include "EC200U.h"
#include "RxRingProcess.h"
#include "Modem_MQTT.h"
#include "modem_GPS.h"
#include "ATmodemTypes.h"
#include "json_parser.h"
#include "OTA.h"
#include "DIDO.h"
#include "w25q_mem.h"
#include "libs.h"
#include "RTC_Time.h"
#include "time.h"
#include "pcbplcTimerTask.h"
#include "pcbplcService.h"
#include "define.h"
#include "Configuration.h"
#include "pcbplc.h"
#include "pcbplcTask.h"
#include "Lcd_16x2.h"
#include "stm32h7xx_hal_crc.h"
#include "timers.h"
#include "MX25L_flash.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

extern int count_DO;
extern uint32_t count_ADC;
extern void reboot_device_func(void);

extern uint32_t stm32deviceID[3],STM32_CRC32;

extern ADC_HandleTypeDef hadc1;

extern QSPI_HandleTypeDef hqspi;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern osSemaphoreId sendExternalFlashSemaphore;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FLASH_IO2_Pin GPIO_PIN_2
#define FLASH_IO2_GPIO_Port GPIOE
#define DO1_Pin GPIO_PIN_3
#define DO1_GPIO_Port GPIOE
#define DO2_Pin GPIO_PIN_4
#define DO2_GPIO_Port GPIOE
#define DO3_Pin GPIO_PIN_5
#define DO3_GPIO_Port GPIOE
#define DO4_Pin GPIO_PIN_6
#define DO4_GPIO_Port GPIOE
#define MODEM_RI_Pin GPIO_PIN_13
#define MODEM_RI_GPIO_Port GPIOC
#define MODEM_RI_EXTI_IRQn EXTI15_10_IRQn
#define DI1_Pin GPIO_PIN_0
#define DI1_GPIO_Port GPIOA
#define DI1_EXTI_IRQn EXTI0_IRQn
#define DI3_Pin GPIO_PIN_1
#define DI3_GPIO_Port GPIOA
#define DI3_EXTI_IRQn EXTI1_IRQn
#define DI2_Pin GPIO_PIN_2
#define DI2_GPIO_Port GPIOA
#define DI2_EXTI_IRQn EXTI2_IRQn
#define DI4_Pin GPIO_PIN_3
#define DI4_GPIO_Port GPIOA
#define DI4_EXTI_IRQn EXTI3_IRQn
#define AI1_Pin GPIO_PIN_4
#define AI1_GPIO_Port GPIOA
#define AI2_Pin GPIO_PIN_5
#define AI2_GPIO_Port GPIOA
#define AI3_Pin GPIO_PIN_6
#define AI3_GPIO_Port GPIOA
#define AI4_Pin GPIO_PIN_7
#define AI4_GPIO_Port GPIOA
#define AI5_Pin GPIO_PIN_4
#define AI5_GPIO_Port GPIOC
#define VOUT_EN_Pin GPIO_PIN_0
#define VOUT_EN_GPIO_Port GPIOB
#define FLASH_CLK_Pin GPIO_PIN_2
#define FLASH_CLK_GPIO_Port GPIOB
#define STAT1_PE10_Pin GPIO_PIN_10
#define STAT1_PE10_GPIO_Port GPIOE
#define STAT2_PE11_Pin GPIO_PIN_11
#define STAT2_PE11_GPIO_Port GPIOE
#define BAT_FAULT_PE12_Pin GPIO_PIN_12
#define BAT_FAULT_PE12_GPIO_Port GPIOE
#define CHG_EN_Pin GPIO_PIN_13
#define CHG_EN_GPIO_Port GPIOE
#define DO5_Pin GPIO_PIN_14
#define DO5_GPIO_Port GPIOE
#define DO6_Pin GPIO_PIN_15
#define DO6_GPIO_Port GPIOE
#define FLASH_CS_Pin GPIO_PIN_10
#define FLASH_CS_GPIO_Port GPIOB
#define DO7_Pin GPIO_PIN_12
#define DO7_GPIO_Port GPIOB
#define DO8_Pin GPIO_PIN_13
#define DO8_GPIO_Port GPIOB
#define DO9_Pin GPIO_PIN_14
#define DO9_GPIO_Port GPIOB
#define DO10_Pin GPIO_PIN_15
#define DO10_GPIO_Port GPIOB
#define DO11_Pin GPIO_PIN_8
#define DO11_GPIO_Port GPIOD
#define DO12_Pin GPIO_PIN_9
#define DO12_GPIO_Port GPIOD
#define DO13_Pin GPIO_PIN_10
#define DO13_GPIO_Port GPIOD
#define FLASH_IO3_Pin GPIO_PIN_11
#define FLASH_IO3_GPIO_Port GPIOD
#define FLASH_IO1_Pin GPIO_PIN_12
#define FLASH_IO1_GPIO_Port GPIOD
#define FLASH_IO0_Pin GPIO_PIN_13
#define FLASH_IO0_GPIO_Port GPIOD
#define DO14_Pin GPIO_PIN_14
#define DO14_GPIO_Port GPIOD
#define DO15_Pin GPIO_PIN_15
#define DO15_GPIO_Port GPIOD
#define DO16_Pin GPIO_PIN_6
#define DO16_GPIO_Port GPIOC
#define DO17_Pin GPIO_PIN_7
#define DO17_GPIO_Port GPIOC
#define DO18_Pin GPIO_PIN_8
#define DO18_GPIO_Port GPIOC
#define DO19_Pin GPIO_PIN_9
#define DO19_GPIO_Port GPIOC
#define DO20_Pin GPIO_PIN_8
#define DO20_GPIO_Port GPIOA
#define SERIAL_TX_Pin GPIO_PIN_9
#define SERIAL_TX_GPIO_Port GPIOA
#define SERIAL_RX_Pin GPIO_PIN_10
#define SERIAL_RX_GPIO_Port GPIOA
#define SERIAL_PD_Pin GPIO_PIN_12
#define SERIAL_PD_GPIO_Port GPIOA
#define LORA_TX_Pin GPIO_PIN_10
#define LORA_TX_GPIO_Port GPIOC
#define LORA_RX_Pin GPIO_PIN_11
#define LORA_RX_GPIO_Port GPIOC
#define LORA_RST_Pin GPIO_PIN_12
#define LORA_RST_GPIO_Port GPIOC
#define LED_LORA_uC_Pin GPIO_PIN_0
#define LED_LORA_uC_GPIO_Port GPIOD
#define MODEM_PWRKEY_Pin GPIO_PIN_1
#define MODEM_PWRKEY_GPIO_Port GPIOD
#define MODEM_DTR_Pin GPIO_PIN_2
#define MODEM_DTR_GPIO_Port GPIOD
#define MODEM_CTS_Pin GPIO_PIN_3
#define MODEM_CTS_GPIO_Port GPIOD
#define MODEM_RTS_Pin GPIO_PIN_4
#define MODEM_RTS_GPIO_Port GPIOD
#define MODEM_RX_Pin GPIO_PIN_5
#define MODEM_RX_GPIO_Port GPIOD
#define MODEM_TX_Pin GPIO_PIN_6
#define MODEM_TX_GPIO_Port GPIOD
#define MODEM_SLEEP_IND_Pin GPIO_PIN_7
#define MODEM_SLEEP_IND_GPIO_Port GPIOD
#define MODEM_SLEEP_IND_EXTI_IRQn EXTI9_5_IRQn

#define MODEM_PWEN_Pin GPIO_PIN_5
#define MODEM_PWEN_GPIO_Port GPIOB

#define MODEM_RESET_Pin GPIO_PIN_6
#define MODEM_RESET_GPIO_Port GPIOB
#define DO_EN_Pin GPIO_PIN_7
#define DO_EN_GPIO_Port GPIOB
#define DISPLAY_SCL_Pin GPIO_PIN_8
#define DISPLAY_SCL_GPIO_Port GPIOB
#define DISPLAY_SDA_Pin GPIO_PIN_9
#define DISPLAY_SDA_GPIO_Port GPIOB
#define DISPLAY_BKLT_Pin GPIO_PIN_0
#define DISPLAY_BKLT_GPIO_Port GPIOE
#define KEY_IN_Pin GPIO_PIN_1
#define KEY_IN_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
void MODEM_GPIO_Define();
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
