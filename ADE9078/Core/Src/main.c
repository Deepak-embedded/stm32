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
#include <stdbool.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ade9078.h"

uint16_t ADE_version=0;
double AVrms=0.0f;
double BVrms=0.0f;
double CVrms=0.0f;
double AFreq=0.0f;
double angleab=0.0f;
double anglebc=0.0f;
double angleac=0.0f;
uint32_t raw_angleab=0,raw_anglebc=0,raw_angleac=0;
static Current_X Ix;
static CurrentRMSRegs XIrms;
AngleRegs_xx_xx raw_angle;
Angle_xx_xx angle;
Power_Factor_Regx raw_pf;
Power_Factorx pfx;
static Line_period_Regs period_x;
Active_pow_Regs raw_apow;
Reactive_pow_Regs raw_rpow={0};
Apparent_pow_Reg raw_appow={0};
static Apparent_pow  appow={0};
static Active_pow apow={0};
static Reactive_pow rpow={0};
uint32_t status_var=0;
bool phase_err=0;
bool DREADY=0;
Freq_X  LF;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint32_t a;
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */









	uint32_t outbuffer=0;
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
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  HAL_Delay(10);
  RESET_LOW;
  HAL_Delay(10);
  RESET_HIGH;
  HAL_Delay(10);
  CS_HIGH;
  HAL_Delay(100);
  uint32_t data=0x01;

  //RSTDONE interrupt indicates that the  ade has finished its power up sequence

  while(!RSTDONE());//set =finished its power up sequence
  ADE_version=get_version();
  while(ADE_version!=ADE_ID);//if ADE_ID is mismatch check spi transition check PM0 and PM1 pin is pull down


  ADE9078_wright_Regs_Data(AVGAIN_32,0x00000000 ,BYTE_32);//set volA channel gain to 1
  ADE9078_read_Regs_Data(AVGAIN_32,&outbuffer ,BYTE_32);
  ADE9078_wright_Regs_Data(AIGAIN_32,0x00000000 ,BYTE_32);//set currentA channel to 1
  ADE9078_read_Regs_Data(AIGAIN_32,&outbuffer ,BYTE_32);
  ADE9078_wright_Regs_Data(CONFIG1_16,(3<<2) ,BYTE_16);//CF4 pin output as DREADY
  ADE9078_read_Regs_Data(CONFIG1_16,&outbuffer ,BYTE_16);
//  ADE9078_wright_Regs_Data(CONFIG0_32,(1<<6) ,BYTE_32);//data going into the zero-crossing detection circuit come before the other component(hpf,integrater,phase compensation
//  ADE9078_read_Regs_Data(AIGAIN_32,&outbuffer ,BYTE_32);
//  ADE9078_wright_Regs_Data(ZX_LP_SEL_16,0x00000000 ,BYTE_16);//Line period measurement from phase A voltage
//  ADE9078_read_Regs_Data(ZX_LP_SEL_16,&outbuffer ,BYTE_16);
//  ADE9078_wright_Regs_Data(ACCMODE_16,(1<<8) ,BYTE_16);
//  ADE9078_read_Regs_Data(ACCMODE_16,&outbuffer ,BYTE_16);
  ADE9078_wright_Regs_Data(RUN_16,data ,BYTE_16);//run ade for mesurement
  ADE9078_read_Regs_Data(RUN_16,&outbuffer ,BYTE_16);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  HAL_Delay(100);
	  AVrms=getAVrms();
	  HAL_Delay(100);
	  BVrms=getBVrms();
	  HAL_Delay(100);
	  CVrms=getCVrms();
	  HAL_Delay(100);

	  ReadCurrentRMSRegs(&XIrms);//read RMS value
	  Ix=ReadCT(&XIrms);//read RMS current

	  //

//
//	  ReadFrequencyRegs(&period_x);//read frequency value
//	  LF=ReadFrequency(&period_x);//line frequency


//	  ADE9078_read_Regs_Data(STATUS0_32,&status_var ,BYTE_32);
//	  DREADY=(status_var>>15)&0x1;
////	  while(!DREADY);
//	  phase_err=(status_var>>18)&0x1;

	  ReadAngleRegs(&raw_angle);
	  angle=ReadAngle(&raw_angle);

//
//
	  ReadActivePowerReg(&raw_apow);
	  apow=ReadActivePower(&raw_apow);

//	  ReadReactivePowerReg(&raw_rpow);
//	  rpow=ReadReactivePower(&raw_rpow);
//
//	  ReadApparentPowerReg(&raw_appow);
//	  appow=ReadApparentPower(&raw_appow);
//
	  ReadPowerFactorReg(&raw_pf);//highest power factor 1 =0x07FFFFFF and -1=0xf8000000

	  pfx=ReadPowerFactor(&raw_pf);







//	  HAL_Delay(1000);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 80;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
