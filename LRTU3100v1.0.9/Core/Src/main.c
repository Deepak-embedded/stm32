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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include "Lora_AT_Types.h"
#include <string.h>

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
ADC_HandleTypeDef hadc1;

CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

QSPI_HandleTypeDef hqspi;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
CRC_HandleTypeDef hcrc;

uint32_t lastToggleTime = 0;
uint8_t  outputState = 0; // 0 = OFF period, 1 = ON period

unsigned char Volatile_1_Non_VOL_0 = 0;
uint8_t Id_Reg_Arry[3] = {0};
uint8_t reg1 = 0,reg2 = 0, reg3 = 0;
HAL_StatusTypeDef SPI_Status;
unsigned char Tx_Buffer = 'A',Tx_Buffer1;
osSemaphoreId sendExternalFlashSemaphore;

//****RTC****//
uint8_t rtctime[20], rtcdate[20];

//****LORA****//
uint8_t Lora_RX_Buff[525];        	//LoRA buffer to fill from Rx interrupt
lwrb_t lora_rx_rb;  				//LoRA Ring buffer instance for RX data
uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA

uint32_t stm32deviceID[3],STM32_CRC32;
uint8_t stm8deviceID[12];
char flag_day_night_reboot;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_RTC_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_I2C1_Init(void);
void MX_USART3_UART_Init(void);
static void MX_QUADSPI_Init(void);
static void MX_CRC_Init(void);
void StartDefaultTask(void const * argument);
void GPIO_Unused_Define_Low(void);

/* USER CODE BEGIN PFP */
cJSON_Hooks rtos_mem_fn = { pvPortMalloc, vPortFree };
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
unsigned int erase_push_counter=0,PushPressTime = 0,Fascia_Pin_status = 2;
/* USER CODE END 0 */

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/
	MPU_Config();

	CPU_CACHE_Enable();
	__enable_irq();
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
	GPIO_Unused_Define_Low();

	MX_ADC1_Init();
	MX_RTC_Init();

	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();

	MX_I2C1_Init();
	MX_QUADSPI_Init();
	MX_CRC_Init();

	/* USER CODE BEGIN 2 */
	// *********RTC***********//
	// set_time(t1);
	WriteLog(1, "\r\n........................START CODE EXCUTION.........................\r\n",1);
	strcpy((char *)EPROM_General.Rtu_Detail.Hex_Version,DEFAULT_FV_VERSION);
	snprintf((char *)print, sizeof(print),"Hex_Version: %s\r\n",EPROM_General.Rtu_Detail.Hex_Version);
	WriteLog(1, (char*)print, 1);

	if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2)
		set_time(t1);
	else
		get_time(t1);

	print_time();
	//**************************//
	HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port,DISPLAY_BKLT_Pin, GPIO_PIN_SET);//lcd

	#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
	HAL_IWDG_Refresh(&hiwdg1);
	#endif

	lwrb_init(&EC200U_RX_rb, EC200U_RX_rb_data, sizeof(EC200U_RX_rb_data));
	lwrb_init(&lora_rx_rb, lora_rx_rb_data, sizeof(lora_rx_rb_data));

	cJSON_InitHooks(&rtos_mem_fn);
	MX25L_Init();
	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* add semaphores, ... */
	stm32deviceID[0]=HAL_GetUIDw0();
	stm32deviceID[1]=HAL_GetUIDw1();
	stm32deviceID[2]=HAL_GetUIDw2();

	stm8deviceID[0] = (stm32deviceID[0]) & 0xff;
	stm8deviceID[1] = (stm32deviceID[0] >> 8) & 0xff;
	stm8deviceID[2] = (stm32deviceID[0] >> 16) & 0xff;
	stm8deviceID[3] = (stm32deviceID[0] >> 24);

	stm8deviceID[4] = stm32deviceID[1] & 0xff;
	stm8deviceID[5] = (stm32deviceID[1] >> 8) & 0xff;
	stm8deviceID[6] = (stm32deviceID[1] >> 16) & 0xff;
	stm8deviceID[7] = (stm32deviceID[1] >> 24);

	stm8deviceID[8] = stm32deviceID[2] & 0xff;
	stm8deviceID[9] = (stm32deviceID[2] >> 8) & 0xff;
	stm8deviceID[10] = (stm32deviceID[3] >> 16) & 0xff;
	stm8deviceID[11] = (stm32deviceID[4] >> 24);

	STM32_CRC32 = HAL_CRC_Calculate(&hcrc, (uint32_t *)stm8deviceID,12);

	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	ExtFlash_Read_RuntimePara(0);
	ExtFlash_Read_EPROM_General(0);
	ExtFlash_Read_EPROM_AI_Calibration(0);
	ExtFlash_Read_EPROM_Schedule(0);
	ExtFlash_Read_EPROM_Dync(0);
	ExtFlash_Read_EPROM_Modbus_Quary_Detail(0);
	ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(0);
	ExtFlash_Read_gPlcRecFlash(0);
	ExtFlash_Read_EPROM_Frequent(0);
	MODEM_GPIO_Define();

	for(unsigned char index=0;index<15;index++)
	{
		Dual_DO_Pulse_Stage[index] = 0;
		Dual_DO_actual_PulseWidth[index] = 500;
	}

	syncExtFlashVariableWithPCBPLCVariable();
	ExtFlash_Read_OTA_Data();

	erase_push_counter = 0;
	Fascia_Pin_status = HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin);
	if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)
	{
		HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
		HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
		while((HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)&&(erase_push_counter<500))
		{
			HAL_Delay(10);
			erase_push_counter++;
		}

		if(erase_push_counter >= 500)
		{

			HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
			HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
			erase_push_counter = 0 ;
			HAL_Delay(1000);
			if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_SET)
			{
				while(erase_push_counter<500)
				{
					if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin)==GPIO_PIN_RESET)
					{

						HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
						HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);					  erase_push_counter = erase_push_counter+20;
						PushPressTime++;
						HAL_Delay(200);

						HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
						HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
					}
					erase_push_counter++;
					HAL_Delay(10);
				}
				if(PushPressTime == 2)
				{
					PushPressTime = 0;
					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
					HAL_Delay(500);
					ExtFlash_Read_RuntimePara(1);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
					HAL_Delay(500);
					ExtFlash_Read_EPROM_General(1);
					ExtFlash_Read_EPROM_AI_Calibration(1);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
					HAL_Delay(500);
					ExtFlash_Read_EPROM_Schedule(1);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
					HAL_Delay(500);
					ExtFlash_Read_EPROM_Modbus_Quary_Detail(1);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
					HAL_Delay(500);
					ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(1);
					ExtFlash_Read_EPROM_Frequent(1);
					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
					HAL_Delay(500);
					ExtFlash_Read_gPlcRecFlash(1);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
					HAL_Delay(500);
					MX25L_EraseSector_QPI(EPROM_REC_MODIFIED_START_ADDRESS);

					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
					HAL_Delay(500);
					HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
					HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
					syncExtFlashVariableWithPCBPLCVariable();
				}
			}
		}
		HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
		HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
	}

	checkProductionMode();

	if(Pro_Application_flag)
	{
	  EPROM_General.pro_CheckByte = 0xAA;
	  ExtFlash_update_EPROM_General();
	  EPROM_General.pro_CheckByte = 0;
	  ExtFlash_Read_EPROM_General(0);
	  if(EPROM_General.pro_CheckByte == 0xAA)
	  {
		  pro_Flash_State = 1;
	  }
	  else
	  {
		  pro_Flash_State = 0;
	  }

	  ExtFlash_update_EPROM_Dync();
	}

	lcd_initialize();
	HAL_Delay(10);
	lcd_set_cursor(1,0);
	lcd_display_string("CIMCON SOFTWARE ");
	lcd_set_cursor(2,0);
	lcd_display_string("    LRTU3100    ");

	BuildModbusMasterQueryTelegrams();

	WriteLog(1, (char *)print, 1);
	//****** Get Astro Time ******//
	Get_Astro_time();

	// Initialize state
	HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_RESET);
	lastToggleTime = HAL_GetTick(); // record start time
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512*10);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  EC200U_start();
  RxRingProcess_start();
  Modem_MQTT_start();
  Lora_start();
  DIDO_start();
  COM_PORT_RS232_1_start();
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/* MPU Configuration */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1KB;//MPU_REGION_SIZE_512B;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00C0EAFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 74;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 25;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, DO1_Pin|DO2_Pin|DO3_Pin|DO4_Pin
                          |CHG_EN_Pin|DO5_Pin|DO6_Pin|DISPLAY_BKLT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DO7_Pin|DO8_Pin|DO9_Pin|DO10_Pin|DO_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, DO11_Pin|DO12_Pin|DO13_Pin|DO14_Pin
                          |DO15_Pin|LED_LORA_uC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, DO16_Pin|DO17_Pin|DO18_Pin|DO19_Pin
                          |LORA_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, GPIO_PIN_RESET);
//  HAL_GPIO_WritePin(GPIOB, DO_EN_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
  /*Configure GPIO pins : DO1_Pin DO2_Pin DO3_Pin DO4_Pin
                           CHG_EN_Pin DO5_Pin DO6_Pin DISPLAY_BKLT_Pin */
  GPIO_InitStruct.Pin = DO1_Pin|DO2_Pin|DO3_Pin|DO4_Pin
                          |CHG_EN_Pin|DO5_Pin|DO6_Pin|DISPLAY_BKLT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);



  /*Configure GPIO pins : DI1_Pin DI3_Pin DI2_Pin DI4_Pin */
  GPIO_InitStruct.Pin = DI1_Pin|DI3_Pin|DI2_Pin|DI4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : VOUT_EN_Pin DO7_Pin DO8_Pin DO9_Pin
                           DO10_Pin MODEM_PWEN_Pin MODEM_RESET_Pin DO_EN_Pin */
  GPIO_InitStruct.Pin = VOUT_EN_Pin|DO7_Pin|DO8_Pin|DO9_Pin
                          |DO10_Pin|DO_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STAT1_PE10_Pin STAT2_PE11_Pin BAT_FAULT_PE12_Pin KEY_IN_Pin */
  GPIO_InitStruct.Pin = STAT1_PE10_Pin|STAT2_PE11_Pin|BAT_FAULT_PE12_Pin|KEY_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : DO11_Pin DO12_Pin DO13_Pin DO14_Pin
                           DO15_Pin LED_LORA_uC_Pin MODEM_PWRKEY_Pin MODEM_DTR_Pin */
  GPIO_InitStruct.Pin = DO11_Pin|DO12_Pin|DO13_Pin|DO14_Pin
                          |DO15_Pin|LED_LORA_uC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : DO16_Pin DO17_Pin DO18_Pin DO19_Pin
                           LORA_RST_Pin */
  GPIO_InitStruct.Pin = DO16_Pin|DO17_Pin|DO18_Pin|DO19_Pin
                          |LORA_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : DO20_Pin */
  GPIO_InitStruct.Pin = DO20_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DO20_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);


/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

  /**************************************************************************//**
  * Function name 	: GPIO_Unused_Define_Low
  * arguments		: 1)
  * return 		 	: no return type
  * Note				: # Set the GPIO pins as input mode or analog mode

  *****************************************************************************/
  void GPIO_Unused_Define_Low(void)
  {
  	GPIO_InitTypeDef GPIO_InitStruct = {0};
  	GPIO_InitStruct.Pin = GPIO_PIN_11;
  	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  	GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_11;
  	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5;
  	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  	GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_13;
  	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  	GPIO_InitStruct.Pull = GPIO_NOPULL;
  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  }
  void MODEM_GPIO_Define()
  {
	  GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */
	if(EPROM_General.Modem_EC200_presence)
	{
		  /* If modem present turn ON 4V */
		  HAL_GPIO_WritePin(MODEM_PWEN_GPIO_Port, MODEM_PWEN_Pin|MODEM_RESET_Pin, GPIO_PIN_SET);

		  /*Configure GPIO pin Output Level */
		  HAL_GPIO_WritePin(GPIOD, MODEM_PWRKEY_Pin|MODEM_DTR_Pin, GPIO_PIN_RESET);

		  /*Configure GPIO pins : MODEM_PWEN_Pin MODEM_RESET_Pin */
		  GPIO_InitStruct.Pin = MODEM_PWEN_Pin|MODEM_RESET_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		  /*Configure GPIO pin : MODEM_RI_Pin */
		  GPIO_InitStruct.Pin = MODEM_RI_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(MODEM_RI_GPIO_Port, &GPIO_InitStruct);

		  /*Configure GPIO pins : DO11_Pin DO12_Pin DO13_Pin DO14_Pin
		                           DO15_Pin LED_LORA_uC_Pin MODEM_PWRKEY_Pin MODEM_DTR_Pin */
		  GPIO_InitStruct.Pin = MODEM_PWRKEY_Pin|MODEM_DTR_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		  /*Configure GPIO pin : MODEM_SLEEP_IND_Pin */
		  GPIO_InitStruct.Pin = MODEM_SLEEP_IND_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(MODEM_SLEEP_IND_GPIO_Port, &GPIO_InitStruct);

		  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
		  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

		  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
		  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	}
	else
	{
		/*Configure GPIO pins : VOUT_EN_Pin DO7_Pin DO8_Pin DO9_Pin
				                           DO10_Pin MODEM_PWEN_Pin MODEM_RESET_Pin DO_EN_Pin */
		  GPIO_InitStruct.Pin = MODEM_PWEN_Pin|MODEM_RESET_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(MODEM_PWEN_GPIO_Port, &GPIO_InitStruct);

		  /*Configure GPIO pin : MODEM_RI_Pin */
		  GPIO_InitStruct.Pin = MODEM_RI_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(MODEM_RI_GPIO_Port, &GPIO_InitStruct);

		  /*Configure GPIO pins : DO11_Pin DO12_Pin DO13_Pin DO14_Pin
								   DO15_Pin LED_LORA_uC_Pin MODEM_PWRKEY_Pin MODEM_DTR_Pin */
		  GPIO_InitStruct.Pin = MODEM_PWRKEY_Pin|MODEM_DTR_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		  /*Configure GPIO pin : MODEM_SLEEP_IND_Pin */
		  GPIO_InitStruct.Pin = MODEM_SLEEP_IND_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  HAL_GPIO_Init(MODEM_SLEEP_IND_GPIO_Port, &GPIO_InitStruct);

		  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
	}
  }
void reboot_device_func(void)
{
	flag_flashSaveRecipe = 1;
	flag_flashUpdateEPROM_General = 1;
	flag_flashUpdateEPROM_AI_Calibration = 1;
	flag_flashUpdateEPROM_Schedule = 1;
	flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;

	flag_flashSaveRecipe_WaitCounter=1;
	flag_flashUpdateEPROM_General_WaitCounter=1;
	flag_flashUpdateEPROM_AI_Calibration_WaitCounter=1;
	flag_flashUpdateEPROM_Schedule_WaitCounter=1;
	flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter=1;
	flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter=299;
	osDelay(6000);
	HAL_NVIC_SystemReset();
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	sendExternalFlashSemaphore= xSemaphoreCreateBinary();
	xSemaphoreGive(sendExternalFlashSemaphore);
	Backlight_Init();

  /* Infinite loop */
  for(;;)
  {
	  if(Pro_Application_flag == 0)
	  {
		  service_start();
		  service_stop();
	  }
	  osDelay(30000);
  }


  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
	DI1_waveLength++;//++;
	DI2_waveLength++;//++;
	//DI_Frequency_Counter+=2;//++;
	if(++DI_Frequency_Counter>EPROM_Frequent.Pulse_DI_frequency_time)  //3000
	{
		DI1_Freq = DI1_Pulse_Count_for_Frequency;///5.0;//((DI1_Pulse_Count-preDI1_Pulse_Count)/3.0);
		DI2_Freq = DI2_Pulse_Count_for_Frequency;///5.0;//((DI2_Pulse_Count-preDI2_Pulse_Count)/3.0);

		DI1_Pulse_Count_for_Frequency=0;
		DI2_Pulse_Count_for_Frequency=0;

		DI_Frequency_Counter=0;
	}
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
