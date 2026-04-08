/*
 * modbus_slave.c
 *
 *  Created on: Jun 24, 2024
 *      Author: Dell
 */




#include "string.h"
#include "main.h"
#include "modbus485.h"


#include<stdbool.h>
#include "cmsis_os.h"

#define SLAVE_ID  1
extern osSemaphoreId txCompleteSem;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE END Includes */

uint16_t startAddr;
uint16_t numRegs;
int8_t TxData[TOTAL_REG_LEN]={0};
int16_t MB_Zero_offset[8]={-2000,-2000,-2000,-2000,-2000,-2000,-2000,-2000};
int16_t MB_Span_offset[8]={20000,20000,20000,20000,20000,20000,20000,20000};


static const uint8_t table_crc_hi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
    0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
    0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
    0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
    0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
    0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
    0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
    0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
    0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
    0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
    0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
    0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
    0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
    0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
    0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
    0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

uint16_t crc16(uint8_t *buffer, uint16_t buffer_length){

    uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
    uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
    unsigned int i; /* will index into CRC lookup */

    /* pass through message buffer */
    while (buffer_length--) {
        i = crc_lo ^ *buffer++; /* calculate the CRC  */
        crc_lo = crc_hi ^ table_crc_hi[i];
        crc_hi = table_crc_lo[i];
    }

    return (crc_hi << 8 | crc_lo);
}


void sendData (int8_t *data, int size,UART_HandleTypeDef *huartx )
{


	uint16_t crc = crc16((uint8_t*)data, size);
	data[size] = crc&0xFF;   // CRC LOW
	data[size+1] =(crc>>8)&0xFF;  // CRC HIGH

	(huartx->Instance==USART2)?HAL_GPIO_WritePin(MAX485_CTRL2_GPIO_Port, MAX485_CTRL2_Pin, GPIO_PIN_SET):HAL_GPIO_WritePin(MAX485_CTRL1_GPIO_Port, MAX485_CTRL1_Pin, GPIO_PIN_SET);      // DE_RE_EN pin high
	HAL_UART_Transmit(huartx, (uint8_t*)data,size+2,HAL_MAX_DELAY);
	memset(uart1_buf,0,sizeof(uart1_buf));
	memset(uart3_buf,0,sizeof(uart3_buf));
    (huartx->Instance==USART2)?HAL_GPIO_WritePin(MAX485_CTRL2_GPIO_Port, MAX485_CTRL2_Pin, GPIO_PIN_RESET):HAL_GPIO_WritePin(MAX485_CTRL1_GPIO_Port, MAX485_CTRL1_Pin, GPIO_PIN_RESET);      // DE_RE_EN pin high


}



//void readHoldingRegs (uint16_t addr)
//{
//	static uint8_t TxData[8];
//	startAddr = addr;//((RxData[2]<<8)|RxData[3]);  // start Register Address
//	TxData[0] = slave_id;  // slave ID
//	TxData[1] = 0x03;//RxData[1];  // function code
//	TxData[2] = (addr>>8)&0xff;
//	TxData[3] = addr&0xff;
//	TxData[4] = 0x00;
//	TxData[5] =0x08;
//	sendData(TxData, 6);  // send data... CRC will be calculated in the function itself
//}
int16_t get_modbus_regs_range(float voltage_x,eMode_t conversion_mode,uint8_t channel){
	int16_t retval;
	int16_t temp;


	switch(conversion_mode){

	case V0_5 :
		voltage_x = ((int)(voltage_x * 100))/100.0;
		retval=MODBUS_REGS(voltage_x,5.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;

	case V0_10 :
		voltage_x = ((int)(voltage_x * 10))/10.0;
		//voltage_x=roundf(voltage_x);
		retval=MODBUS_REGS(voltage_x,10.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;

	case MV0_100 :

		temp=(int)(voltage_x);
		retval=MODBUS_REGS(voltage_x,100.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;
	case MV0_250 :

		temp=(int)(voltage_x);
		retval=MODBUS_REGS(temp,250.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;

	case MV0_50 :

		temp=(int)(voltage_x);
		retval=MODBUS_REGS(voltage_x,50.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;

	case MA0_20 :
		retval=MODBUS_REGS(voltage_x,20.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;

	case MA4_20 :

		retval=MODBUS_REGS((voltage_x-4),(20.0-4.0),MB_Zero_offset[channel],MB_Span_offset[channel]);

//		I=(I-Imin)*((Regmax-Regmin)/Imax-Imin)+Regmin
		break;
	case V0_N10 :
		retval=MODBUS_REGS(voltage_x,-10.0,MB_Zero_offset[channel],MB_Span_offset[channel]);
		break;
	case VN10_N50 :
		retval=MODBUS_REGS((voltage_x -(-10)),(50.0-(-10.0)),MB_Zero_offset[channel],MB_Span_offset[channel]);

//		I=(I-Imin)*((Regmax-Regmin)/Imax-Imin)+Regmin
		break;

	}


	return retval;
}
void Read_Input_Regs(TYPE_t datatype,UART_HandleTypeDef *huartx){
	  	 memset(TxData,0,TOTAL_REG_LEN);
	  	 uint8_t indx;
	  	 if(huartx->Instance==USART1){
			 startAddr = ((tempuart1[2]<<8)|tempuart1[3]);  // start Register Address

			 numRegs = ((tempuart1[4]<<8)|tempuart1[5]);   // number to registers master has requested
//			 if((startAddr+numRegs)>16){
//				 modbusException(ILLEGAL_DATA_ADDRESS);
//				 return;
//			 }
			 TxData[SID] = slave_id;  // slave ID
			 		TxData[FC] = tempuart1[1];  // function code
			 		TxData[TOTAL_BYTE] = numRegs*2;  // Byte count
			 		 indx = DATA_BYTE;  // we need to keep track of how many bytes has been stored in TxData Buffer

			 //Data are store in big endianess manar

			 		if(datatype==UINT32_T){//for 32 bit 2 register will use for write
			 			//reglen=((numRegs&0x01)?numRegs:(numRegs/2));
			 			for (uint8_t i=0; i<((numRegs&0x01)?numRegs:(numRegs/2)); i++)   // Load the actual data into TxData buffer
			 			{
			 					//first register

			 					TxData[indx++] = (Input_Registers_Database[i]>>24)&0xFF;  // extract the higher byte
			 					TxData[indx++] = (uint8_t)(Input_Registers_Database[i]>>16)&0xFF;   // extract the lower byte
			 					//second register
			 					if((numRegs&0x01)==0){
			 						TxData[indx++] = (uint8_t)(Input_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 						TxData[indx++] = (uint8_t)(Input_Registers_Database[i])&0xFF;   // extract the lower byte
			 					}

			 			}
			 		}
			 		else if(datatype==UINT16_T){
			 			for (uint8_t i=0; i<numRegs; i++)   // Load the actual data into TxData buffer
			 			{
			 				TxData[indx++] = (int8_t)(modebus_regs_range[i]>>8)&0xFF;  // extract the higher byte
			 				TxData[indx++] = (int8_t)(modebus_regs_range[i])&0xFF;   // extract the lower byte
			 			}

			 		}

	  	 }
	  	 else if(huartx->Instance==USART2){
			 startAddr = ((tempuart2[2]<<8)|tempuart2[3]);  // start Register Address

			 numRegs = ((tempuart2[4]<<8)|tempuart2[5]);   // number to registers master has requested
//			 if((startAddr+numRegs)>16){
//				 modbusException(ILLEGAL_DATA_ADDRESS);
//				 return;
//			 }
			 TxData[SID] = slave_id;  // slave ID
			 TxData[FC] = tempuart2[1];  // function code
			 TxData[TOTAL_BYTE] = numRegs*2;  // Byte count
			 indx = DATA_BYTE;  // we need to keep track of how many bytes has been stored in TxData Buffer

			 //Data are store in big endianess manar

			 		if(datatype==UINT32_T){//for 32 bit 2 register will use for write
			 			//reglen=((numRegs&0x01)?numRegs:(numRegs/2));
			 			for (uint8_t i=0; i<((numRegs&0x01)?numRegs:(numRegs/2)); i++)   // Load the actual data into TxData buffer
			 			{
			 					//first register

			 					TxData[indx++] = (Input_Registers_Database[i]>>24)&0xFF;  // extract the higher byte
			 					TxData[indx++] = (uint8_t)(Input_Registers_Database[i]>>16)&0xFF;   // extract the lower byte
			 					//second register
			 					if((numRegs&0x01)==0){
			 						TxData[indx++] = (uint8_t)(Input_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 						TxData[indx++] = (uint8_t)(Input_Registers_Database[i])&0xFF;   // extract the lower byte
			 					}

			 			}
			 		}
			 		else if(datatype==UINT16_T){

			 			for (uint8_t i=0; i<numRegs; i++)   // Load the actual data into TxData buffer
			 			{
			 				TxData[indx++] = (int8_t)(modebus_regs_range[i]>>8)&0xFF;  // extract the higher byte
			 				TxData[indx++] = (int8_t)(modebus_regs_range[i])&0xFF;   // extract the lower byte
			 			}


			 		}

	  	 }
		// Prepare TxData buffer


		sendData(TxData,indx,huartx);  // send data... CRC will be calculated in the function itself

}

void Read_Holding_Regs(TYPE_t datatype,UART_HandleTypeDef *huartx){
	  	 memset(TxData,0,TOTAL_REG_LEN);
	  	 uint8_t indx;
	  	 if(huartx->Instance==USART1){
			 startAddr = ((tempuart1[2]<<8)|tempuart1[3]);  // start Register Address

			 numRegs = ((tempuart1[4]<<8)|tempuart1[5]);   // number to registers master has requested
//			 if((startAddr+numRegs)>8){
//				 modbusException(ILLEGAL_DATA_ADDRESS,&huart1);
//				 return;
//			 }
			 TxData[SID] = slave_id;  // slave ID
			 		TxData[FC] = tempuart1[1];  // function code
			 		TxData[TOTAL_BYTE] = numRegs*2;  // Byte count
			 		 indx = DATA_BYTE;  // we need to keep track of how many bytes has been stored in TxData Buffer

			 //Data are store in big endianess manar

			 		if(datatype==UINT32_T){//for 32 bit 2 register will use for write
			 			//reglen=((numRegs&0x01)?numRegs:(numRegs/2));
			 			for (uint8_t i=0; i<((numRegs&0x01)?numRegs:(numRegs/2)); i++)   // Load the actual data into TxData buffer
			 			{
			 					//first register

			 					TxData[indx++] = (Holding_Registers_Database[i]>>24)&0xFF;  // extract the higher byte
			 					TxData[indx++] = (uint8_t)(Holding_Registers_Database[i]>>16)&0xFF;   // extract the lower byte
			 					//second register
			 					if((numRegs&0x01)==0){
			 						TxData[indx++] = (uint8_t)(Holding_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 						TxData[indx++] = (uint8_t)(Holding_Registers_Database[i])&0xFF;   // extract the lower byte
			 					}

			 			}
			 		}
			 		else if(datatype==UINT16_T){
			 			for (uint16_t i=startAddr; i<(numRegs+startAddr); i++)   // Load the actual data into TxData buffer
			 			{

			 				TxData[indx++] = (int8_t)(Holding_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 				TxData[indx++] = (int8_t)(Holding_Registers_Database[i])&0xFF;   // extract the lower byte
			 			}

			 		}

	  	 }
	  	 else if(huartx->Instance==USART2){
			 startAddr = ((tempuart2[2]<<8)|tempuart2[3]);  // start Register Address

			 numRegs = ((tempuart2[4]<<8)|tempuart2[5]);   // number to registers master has requested
//			 if((startAddr+numRegs)>8){
//				 modbusException(ILLEGAL_DATA_ADDRESS,&huart2);
//				 return;
//			 }
			 TxData[SID] = slave_id;  // slave ID
			 		TxData[FC] = tempuart2[1];  // function code
			 		TxData[TOTAL_BYTE] = numRegs*2;  // Byte count
			 		 indx = DATA_BYTE;  // we need to keep track of how many bytes has been stored in TxData Buffer

			 //Data are store in big endianess manar

			 		if(datatype==UINT32_T){//for 32 bit 2 register will use for write
			 			//reglen=((numRegs&0x01)?numRegs:(numRegs/2));
			 			for (uint8_t i=0; i<((numRegs&0x01)?numRegs:(numRegs/2)); i++)   // Load the actual data into TxData buffer
			 			{
			 					//first register

			 					TxData[indx++] = (Holding_Registers_Database[i]>>24)&0xFF;  // extract the higher byte
			 					TxData[indx++] = (uint8_t)(Holding_Registers_Database[i]>>16)&0xFF;   // extract the lower byte
			 					//second register
			 					if((numRegs&0x01)==0){
			 						TxData[indx++] = (uint8_t)(Holding_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 						TxData[indx++] = (uint8_t)(Holding_Registers_Database[i])&0xFF;   // extract the lower byte
			 					}

			 			}
			 		}
			 		else if(datatype==UINT16_T){

			 			for (uint16_t i=startAddr; i<(numRegs+startAddr); i++)   // Load the actual data into TxData buffer
			 			{

			 				TxData[indx++] = (int8_t)(Holding_Registers_Database[i]>>8)&0xFF;  // extract the higher byte
			 				TxData[indx++] = (int8_t)(Holding_Registers_Database[i])&0xFF;   // extract the lower byte
			 			}

			 		}

	  	 }
		// Prepare TxData buffer


		sendData(TxData,indx,huartx);  // send data... CRC will be calculated in the function itself

}

bool Write_Holding_Regs(UART_HandleTypeDef *uartx){

	    // Minimum required bytes
	if(uartx->Instance==USART1){
	    if(uart1len < 8)
	        return false;

	    // Extract CRC from last two bytes
	    uint16_t crc_recv = (tempuart1[uart1len-1]<<8)|tempuart1[uart1len-2];
	    uint16_t crc_calc = crc16(tempuart1, uart1len - 2);

	    // Validate Modbus frame
	    if(tempuart1[0] != slave_id)    return false;
	    if(tempuart1[1] != 0x06) return false;//function code
	    if(crc_recv != crc_calc)     return false;
	    if(tempuart1[5]>8 && tempuart1[3]<=8){
	    	modbusException(ILLEGAL_DATA_VALUE,&huart1);
	    	return false;
	    }

	    int8_t tx[8]={0};

	    tx[0]=tempuart1[0];
	    tx[1]=tempuart1[1];
	    tx[2]=tempuart1[2];
	    tx[3]=tempuart1[3];
	    tx[4]=tempuart1[4];
	    tx[5]=tempuart1[5];

	    sendData(tx,6,uartx);
	    if(tempuart1[3]>=0&&tempuart1[3]<=8){
	    	static uint8_t all_set=0;
	    	adc_conversion_mode[tempuart1[3]-0] = tempuart1[5];
			if(tempuart1[3]==0){
				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, SET);
				//HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, SET);
				all_set|=(1<<0);
			}
			else if(tempuart1[3]==1){

				HAL_GPIO_WritePin(LED2_GPIO_Port, LED1_Pin, SET);
				all_set|=(1<<1);
			}
			else if(tempuart1[3]==2){
				HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, SET);
				all_set|=(1<<2);
			}
			else if(tempuart1[3]==3){
				HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, SET);

				all_set|=(1<<3);
			}
			else if(tempuart1[3]==4){

				HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, SET);
				all_set|=(1<<4);
			}
			else if(tempuart1[3]==5){

				HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, SET);
				all_set|=(1<<5);
			}
			else if(tempuart1[3]==6){
				HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, SET);


				all_set|=(1<<6);
			}
			else if(tempuart1[3]==7){
				HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, SET);
				all_set|=(1<<7);
			}
			if(all_set==0xFF)set_is_configured();

	    }
	    else if(tempuart1[3]>=20&&tempuart1[3]<=28){
	    	MB_Zero_offset[tempuart1[3]-20] = (tempuart1[5]|(tempuart1[4]<<8));
	    }
	    else if(tempuart1[3]>=30&&tempuart1[3]<=38){
	    	MB_Span_offset[tempuart1[3]-30] = (tempuart1[5]|(tempuart1[4]<<8));
	    }
	    else{
	    	modbusException(ILLEGAL_DATA_VALUE,&huart1);
	    }

	}
	else if(uartx->Instance==USART2){
		    if(uart2len < 8)
		        return false;

		    // Extract CRC from last two bytes
		    uint16_t crc_recv = (tempuart2[uart2len-1]<<8)|tempuart2[uart2len-2];
		    uint16_t crc_calc = crc16((uint8_t*)tempuart2, uart2len - 2);

		    // Validate Modbus frame
		    if(tempuart2[0] != slave_id)    return false;
		    if(tempuart2[1] != 0x06) return false;//function code
		    if(crc_recv != crc_calc)     return false;
		    if(tempuart2[5]>8 && tempuart2[3]<=8){
		    	modbusException(ILLEGAL_DATA_VALUE,&huart2);
		    	return false;
		    }

		    int8_t tx[8]={0};

		    tx[0]=tempuart2[0];
		    tx[1]=tempuart2[1];
		    tx[2]=tempuart2[2];
		    tx[3]=tempuart2[3];
		    tx[4]=tempuart2[4];
		    tx[5]=tempuart2[5];

		    sendData(tx,6,uartx);
		    if(tempuart2[3]>=0&&tempuart2[3]<=8){
		    	static uint8_t all_set=0;
		    	adc_conversion_mode[tempuart2[3]-0] = tempuart2[5];
				if(tempuart2[3]==0){
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, SET);
					all_set|=(1<<0);
				}
				else if(tempuart2[3]==1){
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED1_Pin, SET);
					all_set|=(1<<1);
				}
				else if(tempuart2[3]==2){
					HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, SET);
					all_set|=(1<<2);
				}
				else if(tempuart2[3]==3){
					HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, SET);
					all_set|=(1<<3);
				}
				else if(tempuart2[3]==4){
					HAL_GPIO_WritePin(LED6_GPIO_Port, LED6_Pin, SET);
					all_set|=(1<<4);
				}
				else if(tempuart2[3]==5){
					HAL_GPIO_WritePin(LED5_GPIO_Port, LED5_Pin, SET);
					all_set|=(1<<5);
				}
				else if(tempuart2[3]==6){
					HAL_GPIO_WritePin(LED8_GPIO_Port, LED8_Pin, SET);

					all_set|=(1<<6);
				}
				else if(tempuart2[3]==7){
					HAL_GPIO_WritePin(LED7_GPIO_Port, LED7_Pin, SET);
					all_set|=(1<<7);
				}
				if(all_set==0xFF)set_is_configured();

		    }
		    else if(tempuart2[3]>=20&&tempuart2[3]<=28){
		    	MB_Zero_offset[tempuart2[3]-20] = (tempuart2[5]|(tempuart2[4]<<8));
		    }
		    else if(tempuart2[3]>=30&&tempuart2[3]<=38){
		    	MB_Span_offset[tempuart2[3]-30] = (tempuart2[5]|(tempuart2[4]<<8));
		    }
		    else{
		    	modbusException(ILLEGAL_DATA_VALUE,&huart2);
		    }
	}
	    return true;
}


void modbusException (uint8_t exceptioncode,UART_HandleTypeDef *huartx)
{
	//| SLAVE_ID | FUNCTION_CODE | Exception code | CRC     |
	//| 1 BYTE   |  1 BYTE       |    1 BYTE      | 2 BYTES |
	if(huartx->Instance==USART1){
		TxData[0] = tempuart1[0];       // slave ID
		TxData[1] = tempuart1[1]|0x80;  // adding 1 to the MSB of the function code
		TxData[2] = exceptioncode;   // Load the Exception code
	}
	else if(huartx->Instance==USART2){
		TxData[0] = tempuart2[0];       // slave ID
		TxData[1] = tempuart2[1]|0x80;  // adding 1 to the MSB of the function code
		TxData[2] = exceptioncode;   // Load the Exception code
	}
	sendData(TxData,3,huartx);
}





