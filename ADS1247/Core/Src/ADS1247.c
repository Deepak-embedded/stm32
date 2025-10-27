/*
 * ADS1247.c
 *
 *  Created on: Oct 13, 2025
 *      Author: Nelumbo
 */
#include"ADS1247.h"
#include<stdint.h>
#include "stm32f4xx_hal.h"
#include "spi.h"
uint8_t read_cmd[10];
uint8_t writecmd[10] ;

/**
  * @brief  addr :-address of register
  * 		byte :-amount of byte need to read (read one byte)
  * 		return :- register value if sucess or -1 on fail
  */

 uint8_t ADS1247_read_register(uint8_t addr,uint8_t byte){

	  uint8_t cmd[3];
	  uint8_t val;
	  cmd[0] = CMD_RREG | addr;
	  cmd[1] = 0x00; //byte-1
	  cmd[2] = CMD_NOP;

	  CS_LOW;

	  HAL_Delay(10);
	  if(HAL_OK!=HAL_SPI_Transmit(&hspi2, cmd, 2, 1000))
		  return -1;
	  HAL_Delay(10);
	  if(HAL_OK!=HAL_SPI_TransmitReceive(&hspi2, &cmd[2], &val, 1, HAL_MAX_DELAY))
		  return -1;
	  HAL_Delay(10);

	  CS_HIGH;

	  return val;
}

 /**
   * @brief  addr :-address of register
   * 		byte :-amount of byte need to write (write one byte)
   * 		return :- HAL_OK if sucess or -1 on fail
   */

uint8_t ADS1247_write_register(uint8_t addr,uint8_t byte,uint8_t data){


	  uint8_t cmd[3];
	  cmd[0] = CMD_WREG | addr;
	  cmd[1] = 0x00;       // write 1 register
	  cmd[2] = data;

	  CS_LOW;

	  HAL_Delay(10);
	  if(HAL_OK!=HAL_SPI_Transmit(&hspi2, &cmd[0], 1, 1000))
		  return -1;
	  HAL_Delay(10);
	  if(HAL_OK!=HAL_SPI_Transmit(&hspi2, &cmd[1], 1, 1000))
		  return -1;
	  HAL_Delay(10);
	  if(HAL_OK!=HAL_SPI_Transmit(&hspi2, &cmd[2], 1, 1000))
	  	  return -1;
	  HAL_Delay(10);

	  CS_HIGH;

	  return HAL_OK;

}

/**
  * @brief  :- read the adc raw data
  *
  * 		return :- read adc conversion if sucess or -1 on fail
  */
int32_t ADS1247_ReadData(void){

  uint8_t cmd = CMD_RDATA;
  uint8_t rx[3];
  int32_t value = 0;
  uint8_t tx[4]={CMD_NOP,CMD_NOP,CMD_NOP};

  CS_LOW;

  if(HAL_OK!=HAL_SPI_Transmit(&hspi2, &cmd, 1, 5000))
	  return -1;
  HAL_Delay(10);
  if(HAL_OK!=HAL_SPI_TransmitReceive(&hspi2, tx, rx, 3, HAL_MAX_DELAY))
	  return -1;
  HAL_Delay(10);

  CS_HIGH;
  value = ((int32_t)rx[0] << 16) | ((int32_t)rx[1] << 8) | rx[2];
  // Sign extension for 24-bit data
//  if (value & 0x800000)
//	  value |= 0xFF000000;

  return value;
}

/**
  * @brief  :- init the adc with register
  *
  * 		return :- read adc conversion if sucess or -1 on fail
  */

void ADS1247_begin(void)
{
	  // Reset and Start
	  RESET_LOW;
	  HAL_Delay(10);
	  RESET_HIGH;
	  HAL_Delay(10);

	  START_HIGH;
	  HAL_Delay(10);

	  // Stop continuous read
	  uint8_t cmd = CMD_SDATAC;
	  CS_LOW;
	  HAL_Delay(10);
	  HAL_SPI_Transmit(&hspi2, &cmd, 1, 2000);
	  HAL_Delay(10);
	  CS_HIGH;
	  HAL_Delay(1000);

	  // Configure basic registers

	  ADS1247_write_register(REG_MUX0,1, P_AIN0|N_AINCOM);
	  HAL_Delay(10);
//	  uint8_t var=ADS1247_read_register(REG_MUX0,1);//optional for sanity check
	  HAL_Delay(10);

	  ADS1247_write_register(REG_MUX1,1,VREFCON1_ON | REFSELT1_ON);
	  HAL_Delay(10);
//	  var=ADS1247_read_register(REG_MUX1,1);////optional for sanity check
	  HAL_Delay(10);

	  ADS1247_write_register(REG_MUX1,1,DOR3_20 | PGA2_0);
	  HAL_Delay(10);


}

/**
  * @brief  :- write the adc cmd see @brief1
  *			cmd :- cmd to be write
  * 		return :- read adc conversion if sucess or -1 on fail
  */

uint8_t write_cmd(uint8_t cmd){
	CS_LOW;
	HAL_SPI_Transmit(&hspi2, &cmd,1, 2000);
	CS_HIGH;
	HAL_Delay(2);

	return 0;
}

float ads1247_raw_to_voltage(int32_t raw, float vref, uint8_t pga) {
    const int32_t FULL_SCALE =16777216 ;//8388608;  // 2^23
    return (float)raw * vref / (FULL_SCALE * pga);
}

