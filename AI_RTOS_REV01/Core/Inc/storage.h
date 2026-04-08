/*
 * storage.h
 *
 *  Created on: Dec 16, 2025
 *      Author: 91981
 */

#ifndef INC_STORAGE_H_
#define INC_STORAGE_H_
#pragma once
typedef struct{

	uint32_t BaudRate;
	uint32_t SlaveID;
	uint16_t channalmode[8];
	int16_t MB_Zero_offset[8];
	int16_t MB_Span_offset[8];
}flash_t;

void Flash_Write(uint32_t address, uint8_t *data, uint16_t length);
void Flash_Read(uint32_t address, uint8_t *buffer, uint16_t length);
void Flash_WriteSettings(void* W_setting);
void Flash_ReadSettings(flash_t *R_setting);
uint8_t  get_is_configured();
void  set_is_configured();
//#define FLASH_DATA_ADDR   0x0801FC00



#define FLASH_STORAGE_ADDRESS 0x0800F00C
//#define FLASH_CHANNAL_CONFIGURATION_ADDR	0x0801FC00
//#define FLASH_BAUD_RATE_ADDR	0x0801fc18
//#define FLASH_SID 0x0801fc30

extern uint8_t is_configured;//634,880

#endif /* INC_STORAGE_H_ */
