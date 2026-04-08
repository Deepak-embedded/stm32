/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS485_H
#define __MODBUS485_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "device_configuration.h"

#define SID		   0//slave ID INDEX
#define FC		   1//Function Code INDEX
#define TOTAL_BYTE 2//TOTAL BYTE write INDEX
#define DATA_BYTE  3
#define TOTAL_REG_LEN 206
#define UART_RX_BUF_SIZE 64
#define CRC_LOW	6
#define CRC_HIGH	7
#define CMD_LEN   6 //CMD size excluding CRC of TWO byte
#define MB_ADDR_V0_5 0
#define MB_ADDR_V0_10   1
#define MB_ADDR_MV0_100 2
#define MB_ADDR_MV0_250 3
#define MB_ADDR_MA0_20	4
#define MB_ADDR_MA4_20  5
#define MB_WRITE_FC     0x10
#define MIN_CMD_LEN		25
#define ILLEGAL_FUNCTION       0x01
#define ILLEGAL_DATA_ADDRESS   0x02
#define ILLEGAL_DATA_VALUE     0x03

#define RS485_TX_ENABLE() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET)
#define RS485_RX_ENABLE() HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET)

typedef enum{
	UINT32_T,
	UINT16_T
}TYPE_t;

#define MODBUS_REGS(actual_v,max_v,min_offset,max_offset) ((int16_t)(((actual_v / max_v) * (max_offset-min_offset)) + min_offset))//linear scaling

typedef struct {
    uint8_t buffer[UART_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer_t;
extern int cnt;
extern uint16_t startAddr;
extern uint16_t numRegs;
extern uint8_t uart1RxData[UART_RX_BUF_SIZE];
extern uint8_t uart2RxData[UART_RX_BUF_SIZE];
extern uint8_t tempuart1[UART_RX_BUF_SIZE];
extern uint8_t tempuart2[64];
extern uint8_t uart1_buf[UART_RX_BUF_SIZE];
extern uint8_t uart3_buf[64];
extern uint8_t slave_id;
extern uint8_t data[UART_RX_BUF_SIZE];
extern int32_t Input_Registers_Database[50];
extern int16_t modebus_regs_range[10];
extern int16_t Holding_Registers_Database[50];
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

extern volatile uint16_t uart1len;
extern volatile uint16_t uart2len;
extern int16_t adc_conversion_mode[8];
extern float voltage_x[50];
extern int16_t MB_Zero_offset[8];
extern int16_t MB_Span_offset[8];

uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);
void Read_Input_Regs(TYPE_t datatype,UART_HandleTypeDef *huartx);
void Read_Holding_Regs(TYPE_t datatype,UART_HandleTypeDef *huartx);
bool Write_Holding_Regs(UART_HandleTypeDef *uartx);
void sendData (int8_t *data, int size,UART_HandleTypeDef *huartx );
void modbusException (uint8_t exceptioncode,UART_HandleTypeDef *huartx);
int16_t get_modbus_regs_range(float voltage_x,eMode_t conversion_mode,uint8_t channel);
#endif
