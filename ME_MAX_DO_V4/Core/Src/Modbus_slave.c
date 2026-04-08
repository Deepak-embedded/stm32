/*
 * Modbus_slave.c
 *
 *  Created on: Dec 2, 2025
 *      Author: Dell
 */


#include "Modbus_slave.h"
#include "string.h"

#define MAX_COILS 16
#define MAX_COILS1 16

extern uint8_t uart1_buf[64];
extern uint8_t uart3_buf[64];
extern uint16_t coils_value;

extern uint8_t TxData1[256];
extern uint8_t TxData2[16];
extern uint8_t TxData[16];

extern uint16_t startAddr;
extern uint16_t startAddr1;
extern uint16_t numCoils1;
extern uint16_t numCoils;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

uint8_t coilByte;
uint8_t coilBit;
uint8_t cnt;
extern uint8_t slave_id;

//#define SLAVE_ID  1


void sendData1(uint8_t *data, uint16_t size)
{
	uint16_t crc = crc16(data, size);
	data[size] = crc & 0xFF;
	data[size+1] = (crc >> 8) & 0xFF;

	/* Enable RS485 Driver */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	HAL_Delay(10);
	/* Blocking transmit */
	HAL_UART_Transmit(&huart3, data, size + 2, HAL_MAX_DELAY);
	memset(uart3_buf,0,sizeof(uart3_buf));
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

	//HAL_UART_Transmit_DMA(&huart3, data, size + 2);

	/* Wait until Transmission Complete (shift register empty) */

}


void sendData(uint8_t *data, uint16_t size)
{
	uint16_t crc = crc16_1(data, size);
	data[size] = crc & 0xFF;
	data[size+1] = (crc >> 8) & 0xFF;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_UART_Transmit(&huart1, data, size + 2, HAL_MAX_DELAY);
	memset(uart1_buf,0,sizeof(uart1_buf));
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);

//	HAL_UART_Transmit_DMA(&huart1, data, size + 2);

}



//void modbusException (uint8_t exceptioncode)
//{
//	//| SLAVE_ID | FUNCTION_CODE | Exception code | CRC     |
//	//| 1 BYTE   |  1 BYTE       |    1 BYTE      | 2 BYTES |
//
//
//	TxData[0] = RxData[0];       // slave ID
//	TxData[1] = RxData[1]|0x80;  // adding 1 to the MSB of the function code
//	TxData[2] = exceptioncode;   // Load the Exception code
//	sendData(TxData, 3);         // send Data... CRC will be calculated in the function
//}






uint8_t writeMultiCoils(void)
{

    uint16_t  byteCount = uart1_buf[6];                      // number of data bytes




    //if(startAddr+numCoils>16)return 0;//only 16 register permited
    if((startAddr1+numCoils1)>16)return 0;
    int coilIndex = 0;
    for (int byteIndex = 0; byteIndex < byteCount; byteIndex++)
    {
        uint8_t dataByte = uart1_buf[7 + byteIndex];

      //  memset(&TxData, 0, byteCount);

        for (int bit = 0; bit < 8; bit++)
        {
            if (coilIndex < numCoils1)
            {
                int coilPos = startAddr1 + coilIndex;

                if ((dataByte >> bit) & 0x01)
                    Coils_Database[coilPos / 8] |=  (1 << (coilPos % 8));
                else
                    Coils_Database[coilPos / 8] &= ~(1 << (coilPos % 8));

                coilIndex++;
            }
        }
    }

         // --- Drive GPIOs based on 16-bit coil value ---

                uint16_t coils_value = (Coils_Database[1] << 8) | Coils_Database[0];
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (coils_value & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, (coils_value & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, (coils_value & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (coils_value & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, (coils_value & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (coils_value & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (coils_value & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (coils_value & (1 << 7)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,(coils_value & (1 << 8)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (coils_value & (1 << 9)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,(coils_value & (1 << 10)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,(coils_value & (1 << 11)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,(coils_value & (1 << 12)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,(coils_value & (1 << 13)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,(coils_value & (1 << 14)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,(coils_value & (1 << 15)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // --- Build Response (6 bytes + CRC) ---
    uint8_t TxData1[8];
    TxData1[0] = slave_id;
    TxData1[1] = 0x0f;          // function code
    TxData1[2] = uart1_buf[2];     // startAddr Hi
    TxData1[3] = uart1_buf[3];     // startAddr Lo
    TxData1[4] = uart1_buf[4];     // quantity Hi
    TxData1[5] = uart1_buf[5];     // quantity Lo


    sendData(TxData1,6);
//   HAL_UART_Receive_IT(&huart3, uart3_buf,10);    //multicoil should be RxData[10]
    return 1;
}


uint8_t readCoils(void)
{



    // --- Validation ---


    uint8_t byteCount = (numCoils1 + 7) / 8;
	if((startAddr1+numCoils1)>16)return 0;//only 16 register permited
    TxData[0] = slave_id;     // Slave ID
    TxData[1] = 0x01;         // Function Code (Read Coils)
    TxData[2] = byteCount;    // Number of bytes that follow

    memset(&TxData[3], 0, byteCount);

    // --- Pack coil bits into bytes ---
    uint16_t coilIndex = startAddr1;
    for (int i = 0; i < byteCount; i++) {
        uint8_t dataByte = 0;

        for (int b = 0; b < 8; b++) {
            if (coilIndex < (startAddr1 + numCoils1)) {
                uint8_t coilByte = coilIndex / 8;
                uint8_t coilBit  = coilIndex % 8;

                if (Coils_Database[coilByte] & (1 << coilBit)) {
                    dataByte |= (1 << b);
                }
                coilIndex++;
            }
        }

        TxData[3 + i] = dataByte;
    }

    sendData(TxData, 3 + byteCount);



    return 1;
}


uint8_t writeMultiCoils2(void)
{

    uint16_t  byteCount = uart3_buf[6];                      // number of data bytes




    //if(startAddr+numCoils>16)return 0;//only 16 register permited
    if((startAddr+numCoils)>16)return 0;
    int coilIndex = 0;
    for (int byteIndex = 0; byteIndex < byteCount; byteIndex++)
    {
        uint8_t dataByte = uart3_buf[7 + byteIndex];

       // memset(&TxData, 0, byteCount);

        for (int bit = 0; bit < 8; bit++)
        {
            if (coilIndex < numCoils)
            {
                int coilPos = startAddr + coilIndex;

                if ((dataByte >> bit) & 0x01)
                    Coils_Database[coilPos / 8] |=  (1 << (coilPos % 8));
                else
                    Coils_Database[coilPos / 8] &= ~(1 << (coilPos % 8));

                coilIndex++;
            }
        }
    }

         // --- Drive GPIOs based on 16-bit coil value ---

                uint16_t coils_value = (Coils_Database[1] << 8) | Coils_Database[0];
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (coils_value & (1 << 0)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, (coils_value & (1 << 1)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, (coils_value & (1 << 2)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, (coils_value & (1 << 3)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, (coils_value & (1 << 4)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, (coils_value & (1 << 5)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (coils_value & (1 << 6)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, (coils_value & (1 << 7)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14,(coils_value & (1 << 8)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, (coils_value & (1 << 9)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8,(coils_value & (1 << 10)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,(coils_value & (1 << 11)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15,(coils_value & (1 << 12)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3,(coils_value & (1 << 13)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,(coils_value & (1 << 14)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,(coils_value & (1 << 15)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // --- Build Response (6 bytes + CRC) ---
    uint8_t TxData2[8];
    TxData2[0] = slave_id;
    TxData2[1] = 0x0f;          // function code
    TxData2[2] = uart3_buf[2];     // startAddr Hi
    TxData2[3] = uart3_buf[3];     // startAddr Lo
    TxData2[4] = uart3_buf[4];     // quantity Hi
    TxData2[5] = uart3_buf[5];     // quantity Lo


    sendData1(TxData2,6);
//   HAL_UART_Receive_IT(&huart3, uart3_buf,10);    //multicoil should be RxData[10]
    return 1;
}


uint8_t readCoils2(void)
{


	if((startAddr+numCoils)>16)return 0;//only 16 register permited
    uint8_t byteCount = (numCoils + 7) / 8;

    TxData2[0] = slave_id;     // Slave ID
    TxData2[1] = 0x01;         // Function Code (Read Coils)
    TxData2[2] = byteCount;    // Number of bytes that follow

    memset(&TxData2[3], 0, byteCount);

    // --- Pack coil bits into bytes ---
    uint16_t coilIndex = startAddr;
    for (int i = 0; i < byteCount; i++) {
        uint8_t dataByte = 0;

        for (int b = 0; b < 8; b++) {
            if (coilIndex < (startAddr + numCoils)) {
                uint8_t coilByte = coilIndex / 8;
                uint8_t coilBit  = coilIndex % 8;

                if (Coils_Database[coilByte] & (1 << coilBit)) {
                    dataByte |= (1 << b);
                }
                coilIndex++;
            }
        }

        TxData2[3 + i] = dataByte;
    }

    sendData1(TxData2, 3 + byteCount);
   //
//    HAL_UART_Receive_IT(&huart3, uart3_buf,10);  // Prepare for next request
    return 1;
}



