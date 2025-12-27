/*
 * HC595.c
 *
 *  Created on: Dec 16, 2025
 *      Author: 91981
 */

#include "stm32f0xx_hal.h"
#include "HC595.h"
void hc595(uint8_t data_bit)
{
	for(int j=7; j>=0; j--)
	{
		HAL_GPIO_WritePin(DATA_GPIO_Port, DATA_Pin, (data_bit>>j)&0x01);
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

//void input_low(HC595_OUTPUT_EN bit){
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(bit);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);//high plus for lathed data to output pin
//	HAL_Delay(200);
//}
//
//
//void TRIP_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X82);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
////HAL_Delay(200);
//}
//
//void TRIP_LED_off(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X80);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
////HAL_Delay(200);
//}




//void L1_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X80);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
//	HAL_Delay(200);
//}
//
//void L2_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X40);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
//	HAL_Delay(200);
//}
//void L3_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X40);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
//	HAL_Delay(200);
//}
//
//
//void E_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X01);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
//	HAL_Delay(200);
//}
//
//void PICKUP_LED(void) {
//
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_RESET);
//	hc595(0X14);
//	HAL_GPIO_WritePin(LTCH_GPIO_Port, LTCH_Pin, GPIO_PIN_SET);
//	HAL_Delay(200);
//}
