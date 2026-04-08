///*
// * storage.c
// *
// *  Created on: Dec 12, 2025
// *      Author: 91981
// */
//
//
//
//void SaveSettingsToFlash(void)
//{
//	Flash_WriteSettings(slaveId, currentBaud);
//}
//
//void LoadSettingsFromFlash(void)
//{
//	uint8_t id;
//	uint32_t baud;
//	Flash_ReadSettings(&id, &baud);
//	slaveId = id;
//	currentBaud = baud;
//}
//
#include<stdint.h>
#include"main.h"
#include"storage.h"
#include"string.h"

uint32_t PAGEError;
HAL_StatusTypeDef halstatus;
uint8_t is_configured;


void Flash_Read(uint32_t address, uint8_t *buffer, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        buffer[i] = *(volatile uint8_t*)(address + i);
    }
}

void Flash_WriteSettings(void* W_setting)
{
	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef eraseInit;
	uint32_t sectorError = 0;
	uint32_t address=FLASH_STORAGE_ADDRESS;
	uint8_t *ptr=(uint8_t*)W_setting;
//
	eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
	eraseInit.PageAddress = address;
	eraseInit.NbPages     = 1;

	if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return;
	}
//
	uint8_t size=sizeof(flash_t);
    uint16_t halfword;
    for (uint32_t i = 0; i < size; i += 2)
     {
         if (i + 1 < size)
             halfword = ptr[i] | (ptr[i + 1] << 8);
         else
             halfword = ptr[i];

         HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, halfword);
         address += 2;
     }


	HAL_FLASH_Lock();
}

void Flash_ReadSettings(flash_t *R_setting)
{

//	uint16_t i=0;
	uint32_t address=FLASH_STORAGE_ADDRESS;

    memcpy(R_setting,(void*)address,sizeof(flash_t));


}

uint8_t  get_is_configured(){
	uint32_t address=0x800FB70;
	is_configured=*(volatile uint8_t*)address;
	return is_configured;
}

void  set_is_configured(){
	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef eraseInit;
	uint32_t sectorError = 0;
	uint32_t address=0x800FB70;

	is_configured=1;
	eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
	eraseInit.PageAddress = address;
	eraseInit.NbPages     = 1;

	if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, (uint64_t)is_configured) != HAL_OK)
	{
		HAL_FLASH_Lock();
		return;
	}
	HAL_FLASH_Lock();
}

