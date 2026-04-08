/*
 * flash_mx25.h
 *
 *  Created on: Nov 21, 2025
 *      Author: basavaraj
 */

#ifndef INC_FLASH_MX25_H_
#define INC_FLASH_MX25_H_

#include <stdint.h>
#include <stdbool.h>

// MX25L Status definitions
typedef enum {
    MX25L_OK = 0,
    MX25L_SPI_ERR,
    MX25L_PARAM_ERR,
    MX25L_BUSY,
    MX25L_CHIP_ERR
} MX25L_STATE;

// Function prototypes
MX25L_STATE MX25L_Init(void);
MX25L_STATE MX25L_ReadID_QPI(uint8_t *buf);
MX25L_STATE MX25L_WriteEnable_QPI(void);
uint8_t MX25L_ReadStatusReg_QPI(void);
MX25L_STATE MX25L_IsBusy(void);
void MX25L_WaitForWriteEnd(void);
MX25L_STATE MX25L_EraseSector_QPI(uint32_t SectorAddress);
MX25L_STATE MX25L_PageProgram_QPI(uint8_t *pBuffer, uint32_t Size,uint32_t WriteAddr);
//MX25L_STATE MX25L_ReadRaw_QPI(uint8_t *buf, uint32_t data_len, uint32_t rawAddr);
MX25L_STATE MX25L_Erase_Write_One_Sector(uint8_t *buf, uint16_t len, uint32_t sectorAddr);
void MX25L_EnableQPI(void);



HAL_StatusTypeDef MX25L_ReadRaw(uint8_t *buf, uint16_t data_len, uint32_t rawAddr);

#endif /* INC_FLASH_MX25_H_ */

