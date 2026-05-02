/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include"string.h"
#include <stdbool.h>
#include<HC595.h>
#include<storage.h>
#include<modbus485.h>
#include"ADS1247.h"
#include"dipswitch.h"
#include<math.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

//int cnt=0;
uint8_t slave_id=0x01;
int cnt=0;
int16_t adc_conversion_mode[8]={0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004,0x0004};//V0_10
uint8_t tempuart1[64];
uint8_t tempuart2[64];
int32_t raw1,raw2,raw3;
uint8_t status;
float voltageReading0,voltageReading2,result_temp=0.0;
uint8_t data[UART_RX_BUF_SIZE];
uint32_t indx=0;
uint16_t uart1crc;
uint16_t uart2crc;
uint8_t uart1_frame_ready=0;
uint8_t uart3_frame_ready=0;
uint8_t is_tx_cmpleate=0;
eMode_t conversion_mode,conversion_mode1=0;
int channal=0x00;
float supply[8];
int16_t modebus_regs_range[10]={0};
uint8_t RxData1[64];
uint8_t RxData[64];
volatile uint16_t uart1_rx_size = 0;
volatile uint16_t uart3_rx_size = 0;
volatile uint16_t uart1len =0;
volatile uint16_t uart2len =0;
const MapEntry configMap[] = {
	{0,MV0_50},
    {1, MV0_100},
	{2, MV0_250},
    {3, V0_5},
    {4, V0_10},
    {5,MA4_20},
	{6,MA0_20},
	{7,VN10_10},
	{8,VN5_5},
};
//Reg=((v/100)*22000)-2000
float voltage_x[50];

#define ICNT 1

flash_t W_flash={.channalmode={4,4,4,4,4,4,4,4},
		.MB_Zero_offset={-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000},
		.MB_Span_offset={20000,20000,20000,20000,20000,20000,20000,20000},
		.BaudRate=9600,
		.SlaveID=0x01,
};

flash_t R_flash={.channalmode={4,4,4,4,4,4,4,4},
		.MB_Zero_offset={-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000},
		.MB_Span_offset={20000,20000,20000,20000,20000,20000,20000,20000},
		.BaudRate=9600,
		.SlaveID=0x01,
};

int32_t Input_Registers_Database[50]={0};
int16_t Holding_Registers_Database[50]={0};
RingBuffer_t uart2_rx_rb = { .head = 0, .tail = 0 };
RingBuffer_t uart1_rx_rb = { .head = 0, .tail = 0 };

uint32_t selectedBaudRate;

osSemaphoreId txCompleteSem;
volatile uint32_t last_rx_time = 0;


uint8_t uart1_buf[64];
uint8_t uart3_buf[64];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;

osThreadId defaultTaskHandle;
osThreadId MODBUS_TASKHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
void StartDefaultTask(void const * argument);
void MODBUS_HANDLER(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void UART_DMA_Init(void)
{
	HAL_UART_DMAStop(&huart1);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxData1, sizeof(RxData1));
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

    HAL_UART_DMAStop(&huart2);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxData, sizeof(RxData));
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
}

void UART_SetBaudRate(UART_HandleTypeDef *huart, uint32_t baudRate)
{
	HAL_UART_DeInit(huart);
	huart->Init.BaudRate = baudRate;
	if (HAL_UART_Init(huart) != HAL_OK)
	{
		Error_Handler();
	}
}

void UpdateUartBaudRate(void)
{
	static uint32_t previousBaud = 0;

	uint8_t gpioVal = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) << 1) |
							HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7);

	switch (gpioVal)
	{
		case 0x00: selectedBaudRate = 9600; break;
		case 0x01: selectedBaudRate = 19200; break;
		case 0x02: selectedBaudRate = 38400; break;
		default: selectedBaudRate = 115200; break;
	}

	// 🔥 Only change if baud is different
	if (selectedBaudRate != previousBaud)
	{
		previousBaud = selectedBaudRate;

		// Stop UART first
		HAL_UART_DMAStop(&huart2);
		HAL_UART_DMAStop(&huart1);

		UART_SetBaudRate(&huart2, selectedBaudRate);
		UART_SetBaudRate(&huart1, selectedBaudRate);

		HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxData, sizeof(RxData));
		__HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxData1, sizeof(RxData1));
		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
	}
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        if (Size >= 6 && Size <= sizeof(RxData1))
        {
            if (RxData1[0] == slave_id && !uart1_frame_ready)
            {
                memcpy(uart1_buf, RxData1, Size);

                uart1_rx_size = Size;

                uart1_frame_ready = 1;

//                uart1_recovering = 0;
            }
        }
       // memset(RxData1,0,sizeof(RxData1));
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxData1, sizeof(RxData1));
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }

    else if (huart->Instance == USART2)
    {
        if (Size >= 6 && Size <= sizeof(RxData))
        {
            if (RxData[0] == slave_id && !uart3_frame_ready)
            {
                memcpy(uart3_buf, RxData, Size);

                uart3_rx_size = Size;
//                uart3_last_rx_time = HAL_GetTick();
//                uart3_process_start = uart3_last_rx_time;

                uart3_frame_ready = 1;

//                uart3_recovering = 0;
            }
        }
        memset(RxData,0,sizeof(RxData));
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxData, sizeof(RxData));
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    }
}
void UART1_Recover(void)
{

	uint8_t error = 0;
    // Check all errors first
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) error = 1;
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE))  error = 1;
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE))  error = 1;
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_PE))  error = 1;

    if (error)
    {
    	//cnt2++;
        // 1. Stop DMA
        HAL_UART_DMAStop(&huart1);

        // 2. Clear all error flags
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        __HAL_UART_CLEAR_FEFLAG(&huart1);
        __HAL_UART_CLEAR_NEFLAG(&huart1);
        __HAL_UART_CLEAR_PEFLAG(&huart1);

        // 3. Flush DR (VERY IMPORTANT)
        volatile uint32_t temp;
        temp = huart1.Instance->SR;
        temp = huart1.Instance->DR;
        (void)temp;

        // 4. Restart DMA reception
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxData1, sizeof(RxData1));

//         5. Disable Half Transfer interrupt (good for Modbus)
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
}
void UART3_Recover(void)
{
    uint8_t error = 0;

    // Check all errors first
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE)) error = 1;
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_FE))  error = 1;
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_NE))  error = 1;
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_PE))  error = 1;

    if (error)
    {
        // 1. Stop DMA
        HAL_UART_DMAStop(&huart2);

        // 2. Clear all error flags
        __HAL_UART_CLEAR_OREFLAG(&huart2);
        __HAL_UART_CLEAR_FEFLAG(&huart2);
        __HAL_UART_CLEAR_NEFLAG(&huart2);
        __HAL_UART_CLEAR_PEFLAG(&huart2);

        // 3. Flush DR (VERY IMPORTANT)
        volatile uint32_t temp;
        temp = huart2.Instance->SR;
        temp = huart2.Instance->DR;
        (void)temp;

        // 4. Restart DMA reception
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxData, sizeof(RxData));

        // 5. Disable Half Transfer interrupt (good for Modbus)
        __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    }
}

eMode_t check_and_get_adc_conversion_mode(uint16_t *conversion_mode,channal_t channal){


	for(int i=0;i<sizeof(configMap)/sizeof(MapEntry);i++){
				if(configMap[i].regValue==conversion_mode[channal])
					return configMap[i].config;// set channal on the basis of conversion_mode value

		}
		return configMap[channal].config;//default setting if conversion_mode is mismatch with regValue
}

uint8_t GAIN=1;
uint8_t set_adc_conversion_mode(uint16_t conversion_mode,uint8_t channel){
	static uint16_t enbit=0;
//	channel=channel+1;
		for(int i=0;i<sizeof(configMap)/sizeof(MapEntry);i++){
					if(configMap[i].regValue==conversion_mode){
						switch(configMap[i].config){
							case V0_5:
								 //enable mux1

								if(!(enbit&0x01)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, RESET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, SET);
									enbit |=1<<0;//enable divider mux
									enbit &=~(1<<1);//disable other mux
									GAIN=1;
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN1>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);

								//enable specific channel

								break;
							case V0_10 :
								 //enable mux1

								if(!(enbit&0x01)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, RESET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, SET);
									enbit |=1<<0;//enable divider mux
									enbit &=~(1<<1);//disable other mux
									GAIN=1;
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN1>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);

								//enable specific channel
								break;

							case MV0_50:
								 //enable mux2

								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit |=1<<1;//enable other mux
									enbit &=~(1<<0);//disable divider  mux
									GAIN=8;
									ADS1247_write_register(REG_SYS0,1,PGA2_8 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN0>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);
								//enable specific channal
								break;
							case MV0_100:
								 //enable mux2

								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit &=~(1<<0);//disable divider  mux
									enbit |=1<<1;//enable other mux
									GAIN=8;
									ADS1247_write_register(REG_SYS0,1,PGA2_8 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN0>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);
								//enable specific channal
								break;
							case MV0_250 :
								 //enable mux2

								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit |=1<<1;//enable other mux
									enbit &=~(1<<0);//disable divider  mux
									GAIN=8;
									ADS1247_write_register(REG_SYS0,1,PGA2_8 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN0>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);
								//enable specific channal
								break;
							case MA0_20:
								 //enable mux1

								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit |=1<<1;//enable other mux
									enbit &=~(1<<0);//disable divider  mux
									GAIN=1;
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN0>>1)|N_AIN3));//enable AI1
									//ADS1247_write_register(REG_VBIAS,1,EN_BIAS_AIN0);
								}
								ENABLE_CHANNEL(channel);

								break;
							case MA4_20 :
								 //enable mux1

								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit |=1<<1;//enable other mux
									enbit &=~(1<<0);//disable divider  mux
									GAIN=1;
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN0>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);

								break;

							case VN5_5 :
								if(!(enbit&0x01)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, RESET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, SET);
									enbit |=1<<0;//enable divider mux
									enbit &=~(1<<1);//disable other mux
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN1>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);
								break;

							case VN10_10 :
								if(!(enbit&0x02)){
									HAL_GPIO_WritePin(MUX_EN2_GPIO_Port, MUX_EN2_Pin, SET);
									HAL_GPIO_WritePin(MUX_EN1_GPIO_Port, MUX_EN1_Pin, RESET);
									enbit |=1<<1;//enable other mux
									enbit &=~(1<<0);//disable divider  mux
									GAIN=1;
									ADS1247_write_register(REG_SYS0,1,PGA2_0 | DOR3_20);
									ADS1247_write_register(REG_MUX0,1,((P_AIN1>>1)|N_AIN3));//enable AI1

								}
								ENABLE_CHANNEL(channel);
								break;
						}

					}

			}
	return NLMB_OK;
}




//bool get_adc_conversion_value(uint8_t *adc_conversion_mode,UART_HandleTypeDef *uartx){
//	bool status;
//
//
//	if(uartx->Instance==USART1){
//		uart1len=0;
//		uart1rxcount= RingBuffer_GetCount(&uart1_rx_rb);
//		if(uart1rxcount == 0)
//			return false;  // no data yet
//		if(uart1rxcount > UART_RX_BUF_SIZE )
//			uart1rxcount = UART_RX_BUF_SIZE; // prevent overflow
//
//		 while((uart1rxcount=RingBuffer_GetCount(&uart1_rx_rb))){
//			 RingBuffer_Read(&uart1_rx_rb, &uart1RxData[uart1len++]);
//			 HAL_Delay(1);
//		 }
//	}
//	else if(uartx->Instance==USART2){
//		uart2len=0;
//
//		uart2rxcount= RingBuffer_GetCount(&uart2_rx_rb);
//		if(uart2rxcount==0)
//			return false;  // no data yet
//
//		if(uart2rxcount > UART_RX_BUF_SIZE )
//			uart2rxcount = UART_RX_BUF_SIZE; // prevent overflow
//
//		while((uart2rxcount=RingBuffer_GetCount(&uart2_rx_rb))){
//			 RingBuffer_Read(&uart2_rx_rb, &uart2RxData[uart2len++]);
//			 HAL_Delay(1);
//		 }
//	}
//	read_single_reg(uartx)?(status =true ):(status =false);
//	//read_multi_reg()?(status= true) :(status= false);
//
//    return status;
//}
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim->Instance == TIM2) {
//        HAL_TIM_Base_Stop_IT(&htim2); // Stop timer until next packet
//
//        // Calculate actual bytes received via DMA
//        uint16_t len = RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(huart1.hdmarx);
//
//        if (len >= 8) { // Minimum Modbus RTU frame size
//   		 memcpy(tempuart1,uart1RxData,len);
//
//        }
//
//        // Prepare for next packet: reset DMA if not in Circular mode
//  		 HAL_UART_DMAStop(&huart1);
//  		 HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1RxData, sizeof(uart1RxData));
//  		__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
//    }
//}
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  RST_HIGH_B();
  csHIGH_B();
  enable_MCPA_B();
  port_configA_B();
  latch_configA_B();
  enable_MCPB_B();
  port_configB_B();
  polarity_configB_B();
  latch_configB_B();
  CS_HIGH;//asd1247 cs

  ADS1247_begin();
  UART_DMA_Init();

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of MODBUS_TASK */
  osThreadDef(MODBUS_TASK, MODBUS_HANDLER, osPriorityHigh, 0, 128);
  MODBUS_TASKHandle = osThreadCreate(osThread(MODBUS_TASK), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ADC_DRDY_Pin|MUX_EN3_Pin|ADC_CS_Pin|LED2_Pin
                          |LED3_Pin|MCP_BAUD2_Pin|MCP_BAUD1_Pin|LED1_Pin
                          |MAX485_CTRL1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |LED8_Pin|MCP23_CS_Pin|MUX_S2_Pin|MUX_S1_Pin
                          |MUX_S0_Pin|ADS_RESET_Pin|MUX_EN1_Pin|MUX_EN2_Pin
                          |MCP23_RST1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MAX485_CTRL2_GPIO_Port, MAX485_CTRL2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ADC_DRDY_Pin MUX_EN3_Pin ADC_CS_Pin LED2_Pin
                           LED3_Pin MCP_BAUD2_Pin MCP_BAUD1_Pin LED1_Pin
                           MAX485_CTRL1_Pin */
  GPIO_InitStruct.Pin = ADC_DRDY_Pin|MUX_EN3_Pin|ADC_CS_Pin|LED2_Pin
                          |LED3_Pin|MCP_BAUD2_Pin|MCP_BAUD1_Pin|LED1_Pin
                          |MAX485_CTRL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_Pin LED5_Pin LED6_Pin LED7_Pin
                           LED8_Pin MCP23_CS_Pin MUX_S2_Pin MUX_S1_Pin
                           MUX_S0_Pin ADS_RESET_Pin MUX_EN1_Pin MUX_EN2_Pin
                           MCP23_RST1_Pin */
  GPIO_InitStruct.Pin = LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |LED8_Pin|MCP23_CS_Pin|MUX_S2_Pin|MUX_S1_Pin
                          |MUX_S0_Pin|ADS_RESET_Pin|MUX_EN1_Pin|MUX_EN2_Pin
                          |MCP23_RST1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MAX485_CTRL2_Pin */
  GPIO_InitStruct.Pin = MAX485_CTRL2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MAX485_CTRL2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
	uint32_t sum=0;

    if(get_is_configured()==SET){
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, SET);
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, SET);
		HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, SET);
		HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, SET);
		HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, SET);
		HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, SET);
		HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, SET);
		HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, SET);

    }
  /* Infinite loop */
  for(;;)
  {

		 uint8_t len=0;
		 (void)len;
		 (void)sum;
//		 slave_id=GPIO_readA_B();
//		 W_flash.SlaveID=slave_id;








		 memcpy(W_flash.channalmode,adc_conversion_mode,16);
		 memcpy(W_flash.MB_Zero_offset,MB_Zero_offset,16);
		 memcpy(W_flash.MB_Span_offset,MB_Span_offset,16);


/*save channal configuration in flash*/
		 if((memcmp(W_flash.channalmode,R_flash.channalmode,16)!=0)||(memcmp(W_flash.MB_Span_offset,R_flash.MB_Span_offset,16)!=0)||(memcmp(W_flash.MB_Zero_offset,R_flash.MB_Zero_offset,16)!=0)/*||get_is_configured()==0*/){


			 Flash_WriteSettings(&W_flash);
//			 set_is_configured();
		 }
		 Flash_ReadSettings(&R_flash);
		 memcpy(adc_conversion_mode,R_flash.channalmode,16);
		 memcpy(MB_Zero_offset,R_flash.MB_Zero_offset,16);
		 memcpy(MB_Span_offset,R_flash.MB_Span_offset,16);


		channal=0;
		for(int i=0;i<8;i++){
			set_adc_conversion_mode(adc_conversion_mode[i],i);
			osDelay(130);
			conversion_mode1=check_and_get_adc_conversion_mode((uint16_t*)adc_conversion_mode,channal_0+i);//check channal configuration mode value and get conversion mode

				if (HAL_GPIO_ReadPin(DRDY_PORT, DRDY_PIN) == GPIO_PIN_RESET){
					for(int i=0;i<ICNT;i++){
					 raw1=ADS1247_ReadData(P_AIN0);
					}
				}
			voltageReading0=ads1247_raw_to_voltage(raw1, 4.98, GAIN);
			supply[channal_0+i]=get_supply(voltageReading0,conversion_mode1);

			voltage_x[i]=supply[channal_0+i];

			modebus_regs_range[channal_0+i]=get_modbus_regs_range(voltage_x[i],conversion_mode1,i);

//			if(conversion_mode1==MV0_100){
//				voltage_x=supply[channal_0+i];
//				modebus_regs_range[channal_0]=MODBUS_REGS(voltage_x,100.0,-2000,20000);
//			}

				 memcpy(&Input_Registers_Database[channal_0],&supply[channal_0],4);
				 memcpy(&Input_Registers_Database[channal_1],&supply[channal_1],4);
				 memcpy(&Input_Registers_Database[channal_2],&supply[channal_2],4);
				 memcpy(&Input_Registers_Database[channal_3],&supply[channal_3],4);
				 memcpy(&Input_Registers_Database[channal_4],&supply[channal_4],4);
				 memcpy(&Input_Registers_Database[channal_5],&supply[channal_5],4);
				 memcpy(&Input_Registers_Database[channal_6],&supply[channal_6],4);
				 memcpy(&Input_Registers_Database[channal_7],&supply[channal_7],4);


//				 memcpy(&Input_Registers_Database[channal_0+20],&modebus_regs_range[channal_0],4);
//				 memcpy(&Input_Registers_Database[channal_1+20],&modebus_regs_range[channal_1],4);
//				 memcpy(&Input_Registers_Database[channal_2+20],&modebus_regs_range[channal_2],4);
//				 memcpy(&Input_Registers_Database[channal_3+20],&modebus_regs_range[channal_3],4);
//				 memcpy(&Input_Registers_Database[channal_4+20],&modebus_regs_range[channal_4],4);
//				 memcpy(&Input_Registers_Database[channal_5+20],&modebus_regs_range[channal_5],4);
//				 memcpy(&Input_Registers_Database[channal_6+20],&modebus_regs_range[channal_6],4);
//				 memcpy(&Input_Registers_Database[channal_7+20],&modebus_regs_range[channal_7],4);




				 memcpy(&Holding_Registers_Database[channal_0],&adc_conversion_mode[channal_0],2);
				 memcpy(&Holding_Registers_Database[channal_1],&adc_conversion_mode[channal_1],2);
				 memcpy(&Holding_Registers_Database[channal_2],&adc_conversion_mode[channal_2],2);
				 memcpy(&Holding_Registers_Database[channal_3],&adc_conversion_mode[channal_3],2);
				 memcpy(&Holding_Registers_Database[channal_4],&adc_conversion_mode[channal_4],2);
				 memcpy(&Holding_Registers_Database[channal_5],&adc_conversion_mode[channal_5],2);
				 memcpy(&Holding_Registers_Database[channal_6],&adc_conversion_mode[channal_6],2);
				 memcpy(&Holding_Registers_Database[channal_7],&adc_conversion_mode[channal_7],2);


				 memcpy(&Holding_Registers_Database[channal_0+20],&MB_Zero_offset[channal_0],2);
				 memcpy(&Holding_Registers_Database[channal_1+20],&MB_Zero_offset[channal_1],2);
				 memcpy(&Holding_Registers_Database[channal_2+20],&MB_Zero_offset[channal_2],2);
				 memcpy(&Holding_Registers_Database[channal_3+20],&MB_Zero_offset[channal_3],2);
				 memcpy(&Holding_Registers_Database[channal_4+20],&MB_Zero_offset[channal_4],2);
				 memcpy(&Holding_Registers_Database[channal_5+20],&MB_Zero_offset[channal_5],2);
				 memcpy(&Holding_Registers_Database[channal_6+20],&MB_Zero_offset[channal_6],2);
				 memcpy(&Holding_Registers_Database[channal_7+20],&MB_Zero_offset[channal_7],2);



				 memcpy(&Holding_Registers_Database[channal_0+30],&MB_Span_offset[channal_0],2);
				 memcpy(&Holding_Registers_Database[channal_1+30],&MB_Span_offset[channal_1],2);
				 memcpy(&Holding_Registers_Database[channal_2+30],&MB_Span_offset[channal_2],2);
				 memcpy(&Holding_Registers_Database[channal_3+30],&MB_Span_offset[channal_3],2);
				 memcpy(&Holding_Registers_Database[channal_4+30],&MB_Span_offset[channal_4],2);
				 memcpy(&Holding_Registers_Database[channal_5+30],&MB_Span_offset[channal_5],2);
				 memcpy(&Holding_Registers_Database[channal_6+30],&MB_Span_offset[channal_6],2);
				 memcpy(&Holding_Registers_Database[channal_7+30],&MB_Span_offset[channal_7],2);
		}
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_MODBUS_HANDLER */
/**
* @brief Function implementing the MODBUS_TASK thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MODBUS_HANDLER */
void MODBUS_HANDLER(void const * argument)
{
  /* USER CODE BEGIN MODBUS_HANDLER */
  /* Infinite loop */

  for(;;)
  {
	  slave_id=GPIO_readA_B();
	  W_flash.SlaveID=slave_id;
	  UpdateUartBaudRate();//configure baudrate for mb


/*********************************************************************************************************************/



		if (uart1_frame_ready){
		            uart1_frame_ready = 0;

		            uint16_t len = uart1_rx_size;
		            uart1len=len;

		            if (len < 6 || len > 64)
		            {
		                uart1_rx_size = 0;
		                continue;
		            }

		            uint16_t crc_calc = crc16(uart1_buf, len - 2);
		            uint16_t crc_rx   = uart1_buf[len-2] | (uart1_buf[len-1] << 8);

		            if (crc_calc != crc_rx) continue;
		            if (uart1_buf[1] != 0x03 && uart1_buf[1] != 0x04 &&uart1_buf[1] != 0x06) continue;


//		            startAddr1 = (uart1_buf[2] << 8) | uart1_buf[3];
//		            numCoils1  = (uart1_buf[4] << 8) | uart1_buf[5];


		    		switch (uart1_buf[1])
		    		{
		    			case 0x04:
		    			memcpy(tempuart1,uart1_buf,sizeof(tempuart1));
		    			startAddr = ((tempuart1[2]<<8)|tempuart1[3]);  // start Register Address
		   			    numRegs = ((tempuart1[4]<<8)|tempuart1[5]);   // number to registers master has requested
		    			if((startAddr+numRegs)<=16){
		    				Read_Input_Regs(UINT32_T,&huart1);
		    			}
		    			else{
		    				if((startAddr>19)&&((startAddr+numRegs)<29))
		    				 Read_Input_Regs(UINT16_T,&huart1);
		    				else
		    				 modbusException(ILLEGAL_DATA_ADDRESS,&huart1);
		    			}



		    			memset(tempuart1,0,sizeof(tempuart1));
		    			break;
		    			case 0x06:

		    			memcpy(tempuart1,uart1_buf,sizeof(tempuart1));
		    			Write_Holding_Regs(&huart1);
		    			memset(tempuart1,0,sizeof(tempuart1));
		    			break;
		    			case 0x03:
			    			memcpy(tempuart1,uart1_buf,sizeof(tempuart1));
			    			Read_Holding_Regs(UINT16_T,&huart1);
			    			memset(tempuart1,0,sizeof(tempuart1));
						break;
		    			default :
		    				modbusException(ILLEGAL_FUNCTION,&huart1);
		    				break;

		    		}

		   }
		   else{

			   UART1_Recover();
		   }







//		    // =====================================================
//		    // ================= UART3 PROCESS ======================
//		    // =====================================================
		    if (uart3_frame_ready){
	            uart3_frame_ready = 0;

	            uint16_t len = uart3_rx_size;
	            uart2len=len;

	            if (len < 6 || len > 64)
	            {
	                uart3_rx_size = 0;
	                continue;
	            }

	            uint16_t crc_calc = crc16(uart3_buf, len - 2);
	            uint16_t crc_rx   = uart3_buf[len-2] | (uart3_buf[len-1] << 8);

	            if (crc_calc != crc_rx) continue;
	            if (uart3_buf[1] != 0x03 && uart3_buf[1] != 0x04 &&uart3_buf[1] != 0x06) continue;


//		            startAddr1 = (uart1_buf[2] << 8) | uart1_buf[3];
//		            numCoils1  = (uart1_buf[4] << 8) | uart1_buf[5];


	    		switch (uart3_buf[1])
	    		{
	    			case 0x04:
	    			memcpy(tempuart2,uart3_buf,sizeof(tempuart2));
	    			startAddr = ((tempuart2[2]<<8)|tempuart2[3]);  // start Register Address
	   			    numRegs = ((tempuart2[4]<<8)|tempuart2[5]);   // number to registers master has requested
	    			if((startAddr+numRegs)<=16){
	    				Read_Input_Regs(UINT32_T,&huart2);
	    			}
	    			else{
	    				if((startAddr>19)&&((startAddr+numRegs)<29))
	    				 Read_Input_Regs(UINT16_T,&huart2);
	    				else
	    				 modbusException(ILLEGAL_DATA_ADDRESS,&huart2);
	    			}



	    			memset(tempuart2,0,sizeof(tempuart2));
	    			break;

	    			case 0x06:

	    			memcpy(tempuart2,uart3_buf,sizeof(tempuart2));
	    			Write_Holding_Regs(&huart2);
	    			memset(tempuart2,0,sizeof(tempuart2));
	    			break;

	    			case 0x03:

		    			memcpy(tempuart2,uart3_buf,sizeof(tempuart2));
		    			startAddr = ((tempuart2[2]<<8)|tempuart2[3]);  // start Register Address
		    			numRegs = ((tempuart2[4]<<8)|tempuart2[5]);   // number to registers master has requested
		    			if((startAddr+numRegs)<=8){
		    				Read_Holding_Regs(UINT16_T,&huart2);
		    			}
						else{
							if((startAddr>19)&&((startAddr+numRegs)<29))
							 Read_Holding_Regs(UINT16_T,&huart2);
							else if((startAddr>29)&&((startAddr+numRegs)<39)){
								Read_Holding_Regs(UINT16_T,&huart2);
							}
							else
							 modbusException(ILLEGAL_DATA_ADDRESS,&huart2);
						}
		    			memset(tempuart2,0,sizeof(tempuart2));
					break;

	    			default :
	    				modbusException(ILLEGAL_FUNCTION,&huart2);
	    				break;

	    		}

	   }
	   else{

		   UART3_Recover();
	   }

//






/**********************************************************************************************************************/





    osDelay(1);
  }
  /* USER CODE END MODBUS_HANDLER */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
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
