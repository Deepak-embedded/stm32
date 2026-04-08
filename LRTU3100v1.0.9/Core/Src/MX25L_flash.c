/*
 * flash_mx25.c
 *
 *  Created on: Nov 21, 2025
 *      Author: basavaraj
 */
#include"main.h"
#include"MX25L_flash.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;
extern QSPI_HandleTypeDef hqspi;
// MX25L Command Definitions
#define MX25L_DEVICE_ID         0xAF    // Read JEDEC ID          //OK
#define MX25L_READ_STATUS_REG   0x05    // Read Status Register  //OK
#define MX25L_READ_CONFIG_REG   0x15    // Read Configuration Register
#define MX25L_WRITE_ENABLE      0x06    // Write Enable           //OK
#define MX25L_WRITE_DISABLE     0x04    // Write Disable
#define MX25L_SECTOR_ERASE      0x20    // 4KB Sector Erase
#define MX25L_PAGE_PROGRAM      0x02    // Page Program
//#define MX25L_FAST_READ         0xEB    // Fast Read Quad I/O
#define MX25L_FAST_READ         0xEC    // Fast Read Quad I/O
#define MX25L_ENTER_QPI         0x35    // Enter QPI Mode          //OK
#define MX25L_EXIT_QPI          0xF5    // Exit QPI Mode

/**
 * @brief MX25L Write Enable in QPI mode
 */
MX25L_STATE MX25L_WriteEnable_QPI(void) {
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_WRITE_ENABLE;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
 //   sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.DataMode          = QSPI_DATA_NONE;      // <-- FIXED
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    return MX25L_OK;
}

/**
 * @brief MX25L Read Status Register in QPI mode
 */
/*uint8_t MX25L_ReadStatusReg_QPI(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t status = 0;

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_READ_STATUS_REG; // 0x05
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = 1;
    sCommand.DummyCycles       = 0;                     // ✔ no dummy cycles for RDSR
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return 0; // error code

    if (HAL_QSPI_Receive(&hqspi, &status, HAL_MAX_DELAY) != HAL_OK)
        return 0;

    return status;
}*/

uint8_t MX25L_ReadStatusReg_QPI(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t status = 0;

    sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction     = MX25L_READ_STATUS_REG; // 0x05
    sCommand.AddressMode     = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode        = QSPI_DATA_4_LINES;      // <-- IMPORTANT!!!
    sCommand.NbData          = 1;
    sCommand.DummyCycles     = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode        = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY);
    HAL_QSPI_Receive(&hqspi, &status, HAL_MAX_DELAY);

    return status;
}

/*uint8_t MX25L_ReadConfigReg_QPI(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t config = 0;

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_READ_CONFIG_REG;   // 0x15
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = 1;                        // 1-byte CR
    sCommand.DummyCycles       = 0;                        // NO dummy cycles
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
        return 0xFF;     // error code

    if (HAL_QSPI_Receive(&hqspi, &config, HAL_MAX_DELAY) != HAL_OK)
        return 0xFF;

    return config;
}*/

uint8_t MX25L_ReadConfigReg_QPI(void)
{
    QSPI_CommandTypeDef sCommand = {0};
    uint8_t config = 0;

    sCommand.InstructionMode = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction     = MX25L_READ_CONFIG_REG; // 0x15
    sCommand.AddressMode     = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode        = QSPI_DATA_4_LINES;      // <-- IMPORTANT!!!
    sCommand.NbData          = 1;
    sCommand.DummyCycles     = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY);
    HAL_QSPI_Receive(&hqspi, &config, HAL_MAX_DELAY);

    return config;
}
/**
 * @brief MX25L Check if device is busy
 */
MX25L_STATE MX25L_IsBusy(void) {
    uint8_t status = MX25L_ReadStatusReg_QPI();
    return (status & 0x01) ? MX25L_BUSY : MX25L_OK;  // WIP bit is bit 0
}

/**
 * @brief MX25L Wait for write/erase completion
 */
void MX25L_WaitForWriteEnd(void) {
    while (MX25L_IsBusy() == MX25L_BUSY) {
        HAL_Delay(1);
    }
}

/**
 * @brief MX25L Sector Erase (4KB) in QPI mode
 */
MX25L_STATE MX25L_EraseSector_QPI(uint32_t SectorAddress) {
    // Wait for any previous operation to complete
    MX25L_WaitForWriteEnd();

    // Enable write operations
    MX25L_STATE state = MX25L_WriteEnable_QPI();
    if (state != MX25L_OK) {
        return state;
    }

    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_SECTOR_ERASE;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = SectorAddress;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    // Wait for erase to complete
    MX25L_WaitForWriteEnd();

    return MX25L_OK;
}

/**
 * @brief MX25L Page Program in QPI mode
 */
MX25L_STATE MX25L_PageProgram_QPI(uint8_t *pBuffer, uint32_t Size,uint32_t WriteAddr) {
    if (Size > 256) {  // Page size is 256 bytes
        return MX25L_PARAM_ERR;
    }

    // Wait for any previous operation to complete
    MX25L_WaitForWriteEnd();

    // Enable write operations
    MX25L_STATE state = MX25L_WriteEnable_QPI();
    if (state != MX25L_OK) {
        return state;
    }

    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_PAGE_PROGRAM;
    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = WriteAddr;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = Size;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    // Send data
    if (HAL_QSPI_Transmit(&hqspi, pBuffer, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    // Wait for write to complete
    MX25L_WaitForWriteEnd();

    return MX25L_OK;
}

///**
// * @brief MX25L Read Data in QPI mode
// */
//MX25L_STATE MX25L_ReadRaw_QPI(uint8_t *buf, uint32_t data_len, uint32_t rawAddr) {
//    // Wait for any previous operation to complete
//    MX25L_WaitForWriteEnd();
//
//    QSPI_CommandTypeDef sCommand = {0};
//
//    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
//    sCommand.Instruction       = MX25L_FAST_READ;  // Fast Read Quad I/O
//    sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
//    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
//    sCommand.Address           = rawAddr;
//    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
//    sCommand.DataMode          = QSPI_DATA_4_LINES;
//    sCommand.NbData            = data_len;
//    sCommand.DummyCycles       = 6;  // Dummy cycles for Fast Read
//    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
//    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
//
//    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
//        return MX25L_SPI_ERR;
//    }
//
//    if (HAL_QSPI_Receive(&hqspi, buf, HAL_MAX_DELAY) != HAL_OK) {
//        return MX25L_SPI_ERR;
//    }
//
//    return MX25L_OK;
//}




/**
 * @brief MX25L Read ID in QPI mode
 */
MX25L_STATE MX25L_ReadID_QPI(uint8_t *buf) {
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
    sCommand.Instruction       = MX25L_DEVICE_ID;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_4_LINES;
    sCommand.NbData            = 3;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    if (HAL_QSPI_Receive(&hqspi, buf, HAL_MAX_DELAY) != HAL_OK) {
        return MX25L_SPI_ERR;
    }

    return MX25L_OK;
}





/**
 * @brief MX25L Enable QPI Mode
 */
void MX25L_EnableQPI(void) {
    QSPI_CommandTypeDef sCommand = {0};
    char logBuffer[100];

    // Now enter QPI mode
    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = MX25L_ENTER_QPI;
    sCommand.AddressMode       = QSPI_ADDRESS_NONE;
    sCommand.DataMode          = QSPI_DATA_NONE;
    sCommand.DummyCycles       = 0;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY);

    sprintf(logBuffer, "MX25L QPI mode enabled\r\n");
    WriteLog(1, logBuffer, 1);
}

/**
 * @brief MX25L Erase and Write One Sector (equivalent to W25Q function)
 */
MX25L_STATE MX25L_Erase_Write_One_Sector(uint8_t *buf, uint16_t len, uint32_t sectorAddr) {
    uint32_t end_addr, current_size, current_addr=0,last_addr = 0;
    MX25L_STATE status;

    // Erase the sector first
	current_addr = sectorAddr;
	last_addr = current_addr+len;

	while(current_addr < last_addr)
	{
		status = MX25L_EraseSector_QPI(sectorAddr);
		   if (status != MX25L_OK) {
		        return status;
		    }
		current_addr+=SECTOR_COUNT;
	}
    // Calculate the size between the write address and the end of the page
    current_size = 256 - (sectorAddr % 256);  // Page size is 256 bytes

    // Check if the size of the data is less than the remaining place in the page
    if (current_size > len) {
        current_size = len;
    }

    // Initialize the address variables
    current_addr = sectorAddr;
    end_addr = sectorAddr + len;

    // Perform the write page by page
    do {
        status = MX25L_PageProgram_QPI(buf, current_size,current_addr);
        if (status != MX25L_OK) {
            return status;
        }

        // Update the address and size variables for next page programming
        current_addr += current_size;
        buf += current_size;
        current_size = ((current_addr + 256) > end_addr) ? (end_addr - current_addr) : 256;

    } while (current_addr < end_addr);

    return MX25L_OK;
}

/**
 * @brief MX25L Initialization function (equivalent to W25Q_Init)
 */
MX25L_STATE MX25L_Init(void)
{
    MX25L_STATE state;
    uint8_t flash_id[3];
    char logBuffer[100];

    // Enable QPI mode
    MX25L_EnableQPI();
    HAL_Delay(10);


    // Read ID to verify communication
    state = MX25L_ReadID_QPI(flash_id);
    if (state != MX25L_OK) {
        WriteLog(1, "Failed to read MX25L ID\r\n", 1);
        return state;
    }

    sprintf(logBuffer, "MX25L ID: %02X %02X %02X\r\n", flash_id[0], flash_id[1], flash_id[2]);
    WriteLog(1, logBuffer, 1);

    // Verify it's the correct chip (MX25L25645G should return C2 20 19)
    if (flash_id[0] != 0xC2 || flash_id[1] != 0x20 || flash_id[2] != 0x19) {
        WriteLog(1, "Warning: Unexpected MX25L device ID\r\n", 1);
    }
    WriteLog(1, "MX25L initialization complete\r\n", 1);
    return MX25L_WriteEnable_QPI();
}

HAL_StatusTypeDef MX25L_ReadRaw(uint8_t *buf, uint16_t data_len, uint32_t rawAddr)
{
  QSPI_CommandTypeDef sCommand = {0};

  // Fast Read command
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  sCommand.Instruction       = MX25L_FAST_READ;  // Fast Read Quad I/O
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.Address           = rawAddr;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_4_LINES;
  sCommand.NbData            = data_len;
  sCommand.DummyCycles       = 6;  // Dummy cycles for Fast Read
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK)
  {
    return HAL_ERROR;
  }

  // Receive data
  if (HAL_QSPI_Receive(&hqspi, buf, HAL_MAX_DELAY) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}



