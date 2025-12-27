/*
 * HC595.c
 *
 *  Created on: Dec 16, 2025
 *      Author: 91981
 */

#include "stm32f1xx_hal.h"
#include "HC595.h"
void hc595(uint8_t data_bit)
{
	for(int j=0; j<8; j++)
	{
		HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, (data_bit>>j)&1);
		HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, GPIO_PIN_SET);//high to low transition to store data to shift register
		HAL_GPIO_WritePin(CLK_GPIO_Port, CLK_Pin, GPIO_PIN_RESET);
	}
}


void input_high(uint8_t  bit ) {


	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
	hc595(bit);
	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);//high plus for lathed data to output pin
	HAL_Delay(200);
}


