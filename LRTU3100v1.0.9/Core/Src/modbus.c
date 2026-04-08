
/*
 * mosbus.c
 *
 *  Created on: Nov 12, 2022
 *      Author: SanketP
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "modbus.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "pcbplccomm.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
extern UART_HandleTypeDef huart1;
uint8_t Digital_bit_Query_array[100];
uint8_t gNoofQueryStored;
uint16_t gTotalNumberOfReadDataUsingAllQuery;
modbus_t telegram[MODBUS_MASTER_MAX_TOTAL_QUERY];
modbusHandler_t ModbusH[COM_LORA+1]; // [0 : COM_RS232_1] [1 : COM_LORA]
uint16_t ModbusDATA[100]={11,22,33,44,55,66,77,88};
char RTCsync_flage=0;
const unsigned char fctsupported[] =
{
    MB_FC_READ_COILS,
    MB_FC_READ_DISCRETE_INPUT,
    MB_FC_READ_REGISTERS,
    MB_FC_READ_INPUT_REGISTER,
    MB_FC_WRITE_COIL,
    MB_FC_WRITE_REGISTER,
    MB_FC_WRITE_MULTIPLE_COILS,
    MB_FC_WRITE_MULTIPLE_REGISTERS
};
unsigned int tempLora_Frequency;
uint8_t tempLora_Spreading_Factor;
uint8_t tempLora_Bandwidth;
uint8_t tempLora_Code_Rate;
uint16_t tempLora_Preamble_Length;
uint8_t tempLora_TX_Power;
/**************************************************************************//**
 * Function name 	: ModbusInit
 * arguments		: 1) modH : ModbusPort Handler
 * 		 		 	: 2)
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

void ModbusInit(modbusHandler_t * modH)
{

	modH->port = &huart1; //HAL Serial Port handler
	modH->u8id = 1; //!< 0=master, 1..247=slave number
	//modH->u8Buffer[0] = '\0'; //Modbus buffer for communication
	modH->u8BufferSize = 0;
	modH->u16regs = 0;
	modH-> u16regsize = 0;

}

/**************************************************************************//**
 * Function name 	: ModbusInitData
 * arguments		: 1) telegram : Modbus Query Structure
 * 		 		 	: 2)
 * return			:
 * Note				: Feel telegram Structure with Query Data
 * 					:
 *****************************************************************************/

void ModbusInitData(modbus_t *telegram)
 {
 	telegram->uQueryNo = 0;
 	telegram->uPortNo = 1;
 	telegram->u8id = 1;          /*!< Slave address between 1 and 247. 0 means broadcast */
 	telegram->u8fct = 6;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
 	telegram->u16RegAdd = 1;    /*!< Address of the first register to access at slave/s */
 	telegram->u16CoilsNo = 1;   /*!< Number of coils or registers to access */
 	telegram->u16reg[0] = 123;
  }

 /**************************************************************************//**
  * Function name 	: validateAnswer
  * arguments		: 1) modH : ModbusPort Handler
  * 		 		: 2)
  * return			: 0 if OK, EXCEPTION if anything fails
  * Note			: This method validates slave incoming messages
  * 				:
  *****************************************************************************/

uint8_t validateAnswer(modbusHandler_t *modH)
 {
	 uint8_t isSupported =0;
     // check message crc vs calculated crc
 	uint16_t u16MsgCRC =
         ((modH->u8RxBuffer[modH->u8BufferSize - 2] << 8)
          | modH->u8RxBuffer[modH->u8BufferSize - 1]); // combine the crc Low & High bytes
     if ( calcCRC(modH->u8RxBuffer,  modH->u8BufferSize-2) != u16MsgCRC )
     {
     	//modH->u16errCnt ++;
         return 2;
     }

     // check exception
     if ((modH->u8Buffer[ FUNC ] & 0x80) != 0)
     {
     	//modH->u16errCnt ++;
         return 3;
     }


     for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
     {
         if (fctsupported[i] == modH->u8Buffer[FUNC])
         {
             isSupported = 1;
             break;
         }
     }
     if (!isSupported)
     {
     	//modH->u16errCnt ++;
         return EXC_FUNC_CODE;
     }

     return 0; // OK, no exception code thrown
 }

/**************************************************************************//**
 * Function name 	: validateAnswer (Not used)
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2)
 * return			: 0 if OK, EXCEPTION if anything fails
 * Note				: This method validates slave incoming messages
 * 					:
 *****************************************************************************/

//uint8_t validateRequest(modbusHandler_t *modH)
// {
// 	// check message crc vs calculated crc
//
//
// 	    uint16_t u16MsgCRC,isSupported;
// 	    u16MsgCRC= ((modH->u8Buffer[modH->u8BufferSize - 2] << 8)
// 	    		   	         | modH->u8Buffer[modH->u8BufferSize - 1]); // combine the crc Low & High bytes
//
//
// 	    if ( calcCRC( modH->u8Buffer,  modH->u8BufferSize-2 ) != u16MsgCRC )
// 	    {
// 	       		//modH->u16errCnt ++;
// 	       		return 2;
// 	    }
//
// 	    // check fct code
// 	    for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
// 	    {
// 	        if (fctsupported[i] == modH->u8Buffer[FUNC])
// 	        {
// 	            isSupported = 1;
// 	            break;
// 	        }
// 	    }
// 	    if (!isSupported)
// 	    {
// 	    	//modH->u16errCnt ++;
// 	        return EXC_FUNC_CODE;
// 	    }
//
// 	    // check start address & nb range
// 	    uint16_t u16AdRegs = 0;
// 	    uint16_t u16NRegs = 0;
//
// 	    //uint8_t u8regs;
// 	    switch ( modH->u8Buffer[ FUNC ] )
// 	    {
// 	    case MB_FC_READ_COILS:
// 	    case MB_FC_READ_DISCRETE_INPUT:
// 	    case MB_FC_WRITE_MULTIPLE_COILS:
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) / 16;
// 	    	u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) /16;
// 	    	if(word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) % 16) u16NRegs++; // check for incomplete words
// 	    	// verify address range
// 	    	if((u16AdRegs + u16NRegs) > modH->u16regsize) return EXC_ADDR_RANGE;
//
// 	    	//verify answer frame size in bytes
//
// 	    	u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) / 8;
// 	    	if(word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]) % 8) u16NRegs++;
// 	    	u16NRegs = u16NRegs + 5; // adding the header  and CRC ( Slave address + Function code  + number of data bytes to follow + 2-byte CRC )
// 	        if(u16NRegs > 256) return EXC_REGS_QUANT;
//
// 	        break;
// 	    case MB_FC_WRITE_COIL:
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) / 16;
// 	    	if(word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]) % 16) u16AdRegs++;	// check for incomplete words
// 	        if (u16AdRegs > modH->u16regsize) return EXC_ADDR_RANGE;
// 	        break;
// 	    case MB_FC_WRITE_REGISTER :
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]);
// 	        if (u16AdRegs > modH-> u16regsize) return EXC_ADDR_RANGE;
// 	        break;
// 	    case MB_FC_READ_REGISTERS :
// 	    case MB_FC_READ_INPUT_REGISTER :
// 	    case MB_FC_WRITE_MULTIPLE_REGISTERS :
// 	    	u16AdRegs = word( modH->u8Buffer[ ADD_HI ], modH->u8Buffer[ ADD_LO ]);
// 	        u16NRegs = word( modH->u8Buffer[ NB_HI ], modH->u8Buffer[ NB_LO ]);
// 	        if (( u16AdRegs + u16NRegs ) > modH->u16regsize) return EXC_ADDR_RANGE;
//
// 	        //verify answer frame size in bytes
// 	        u16NRegs = u16NRegs*2 + 5; // adding the header  and CRC
// 	        if ( u16NRegs > 256 ) return EXC_REGS_QUANT;
// 	        break;
// 	    }
// 	    return 0; // OK, no exception code thrown
//
// }

/**************************************************************************//**
 * Function name 	: Master_Parse_Modbus_Responce
 * arguments		: 1) argument : modH : ModbusPort Handler
 * 		 			: 2) argument2 : telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void Master_Parse_Modbus_Responce(void *argument,void *argument2)
{

	modbusHandler_t *modH =  (modbusHandler_t *)argument;
	modbus_t *telegram = (modbus_t *)argument2;

	// This is the case for implementations with only USART support
	// SendQuery(modH, telegram);
	//getRxBuffer(modH);

//	modH->u8BufferSize = modH->u8RxBuffer[2]+3;
//	telegram->u8rxdataValidation = validateAnswer(modH);
//	if (telegram->u8rxdataValidation != 0)
//	{
//
//	}
//	else
	{
		switch( modH->u8Buffer[ FUNC ] )
		{
			case MB_FC_READ_COILS:
			case MB_FC_READ_DISCRETE_INPUT:
			  //call get_FC1 to transfer the incoming message to u16regs buffer
			  get_FC1(modH,telegram);
			  break;
			case MB_FC_READ_INPUT_REGISTER:
			case MB_FC_READ_REGISTERS :
			  // call get_FC3 to transfer the incoming message to u16regs buffer
			  get_FC3(modH,telegram);
			  break;
			case MB_FC_WRITE_COIL:
			case MB_FC_WRITE_REGISTER :
			case MB_FC_WRITE_MULTIPLE_COILS:
			case MB_FC_WRITE_MULTIPLE_REGISTERS :
				get_FC5_FC6(modH,telegram);
			  // nothing to do
			  break;
			default:
			  break;
		}
	}

}

/**************************************************************************//**
 * Function name 	: Master_Send_Modbus_Query
 * arguments		: 1) argument : modH : ModbusPort Handler
 * 		 			: 2) telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

void Master_Send_Modbus_Query(void *argument, modbus_t *telegram)
 {
	 modbusHandler_t *modH =  (modbusHandler_t *)argument;
	 //uint32_t notification;

	 //modH->i8lastError = 0;
	 // validate message: CRC, FCT, address and size
	 //uint8_t u8exception = validateRequest(modH);
	// if (u8exception > 0)
	// {}
	// else
	 {
		 // process message
		 switch(telegram->u8fct )
		 {
 			case MB_FC_READ_COILS:
 			case MB_FC_READ_DISCRETE_INPUT:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_READ_INPUT_REGISTER:
 			case MB_FC_READ_REGISTERS :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_COIL:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_REGISTER :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_MULTIPLE_COILS:
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			case MB_FC_WRITE_MULTIPLE_REGISTERS :
 				modH->i8state = SendQuery(modH, telegram);
 				break;
 			default:
 				break;
		 }
	 }
 }

/**************************************************************************//**
 * Function name 	: SendQuery
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) telegram
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/

int8_t SendQuery(modbusHandler_t *modH ,  modbus_t *telegram )
{

	uint8_t u8regsno, u8bytesno;

	modH->u16regs = telegram->u16reg;

	// telegram header
	modH->u8Buffer[ ID ]         = telegram->u8id;
	modH->u8Buffer[ FUNC ]       = telegram->u8fct;
	modH->u8Buffer[ ADD_HI ]     = highByte(telegram->u16RegAdd );
	modH->u8Buffer[ ADD_LO ]     = lowByte( telegram->u16RegAdd );

	switch( telegram->u8fct )
	{
	case MB_FC_READ_COILS:
	case MB_FC_READ_DISCRETE_INPUT:
	case MB_FC_READ_REGISTERS:
	case MB_FC_READ_INPUT_REGISTER:
		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_COIL:
		modH->u8Buffer[ NB_HI ]      = (( telegram->u16reg[0]> 0) ? 0xff : 0);
		modH->u8Buffer[ NB_LO ]      = 0;
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_REGISTER:
		modH->u8Buffer[ NB_HI ]      = highByte( telegram->u16reg[0]);
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16reg[0]);
		modH->u8BufferSize = 6;
		break;
	case MB_FC_WRITE_MULTIPLE_COILS: // TODO: implement "sending coils"
		u8regsno = telegram->u16CoilsNo / 16;
		u8bytesno = u8regsno * 2;
		if ((telegram->u16CoilsNo % 16) != 0)
		{
			u8bytesno++;
			u8regsno++;
		}

		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8Buffer[ BYTE_CNT ]    = u8bytesno;
		modH->u8BufferSize = 7;

		for (uint16_t i = 0; i < u8bytesno; i++)
		{
			if(i%2)
			{
				modH->u8Buffer[ modH->u8BufferSize ] = lowByte( telegram->u16reg[ i/2 ] );
			}
			else
			{
				modH->u8Buffer[  modH->u8BufferSize ] = highByte( telegram->u16reg[ i/2 ] );

			}
			modH->u8BufferSize++;
		}
		break;

	case MB_FC_WRITE_MULTIPLE_REGISTERS:
		modH->u8Buffer[ NB_HI ]      = highByte(telegram->u16CoilsNo );
		modH->u8Buffer[ NB_LO ]      = lowByte( telegram->u16CoilsNo );
		modH->u8Buffer[ BYTE_CNT ]    = (uint8_t) ( telegram->u16CoilsNo * 2 );
		modH->u8BufferSize = 7;

		for (uint16_t i=0; i< telegram->u16CoilsNo; i++)
		{

			modH->u8Buffer[  modH->u8BufferSize ] = highByte(  telegram->u16reg[ i ] );
			modH->u8BufferSize++;
			modH->u8Buffer[  modH->u8BufferSize ] = lowByte( telegram->u16reg[ i ] );
			modH->u8BufferSize++;
		}
		break;
	}
	sendTxBuffer(modH);
	return 0;
}

/**************************************************************************//**
 * Function name 	: calcCRC
 * arguments		: 1) *Buffer
 * 		 			: 2) u8length
 * return			: CRC
 * Note				:
 * 					:
 *****************************************************************************/

uint16_t calcCRC(uint8_t *Buffer, uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;

}

/**************************************************************************//**
 * Function name 	: sendTxBuffer
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			:
 * Note				: send Query over selected port
 * 					:
 *****************************************************************************/

void sendTxBuffer(modbusHandler_t *modH)
{
    // append CRC to message

	uint16_t u16crc = calcCRC(modH->u8Buffer, modH->u8BufferSize);
	modH->u8Buffer[ modH->u8BufferSize ] = u16crc >> 8;
	modH->u8BufferSize++;
	modH->u8Buffer[ modH->u8BufferSize ] = u16crc & 0x00ff;
	modH->u8BufferSize++;

    if(modH->u8Port == COM_LORA)
    {
    	memset(lora_tx_buf,0,sizeof(lora_tx_buf));
    	asciiStringToHexString((char *)modH->u8Buffer,lora_tx_buf, modH->u8BufferSize);
    }
    else
	{
		HAL_UART_Transmit(modH->port, modH->u8Buffer,  modH->u8BufferSize,1000);
	}

    modH->u8BufferSize = 0;
    // increase message counter
}

/**************************************************************************//**
 * Function name 	: process_FC1
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 1
 * 					:
 *****************************************************************************/

int8_t process_FC1(modbusHandler_t *modH )
{
	uint8_t u8bytesno, u8bitsno;
    uint16_t u8CopyBufferSize;
    uint16_t u16currentCoil;

    // get the first and last coil from the message
	
    uint16_t u16StartCoil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16Coilno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
	
    // put the number of bytes in the outcoming message

    u8bytesno = (uint8_t) (u16Coilno / 8);
    if (u16Coilno % 8 != 0)
	{
    	u8bytesno ++;
	}

    modH->u8Buffer[ ADD_HI ]  = u8bytesno;
    modH->u8BufferSize         = ADD_LO;
    modH->u8Buffer[modH->u8BufferSize + u8bytesno - 1 ] = 0;

    // read each coil from the register map and put its value inside the outcoming message

    u8bitsno = 0;


	for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
	{
		if ((u16currentCoil + u16StartCoil + 1 >= 1)
				&& (u16currentCoil + u16StartCoil + 1 <= 960))  // for DI status
		{
			bitWrite(
				modH->u8Buffer[ modH->u8BufferSize ],
				u8bitsno,
				dig_bit_array[u16currentCoil + u16StartCoil]);
		}
		else
		{
			bitWrite(modH->u8Buffer[ modH->u8BufferSize ],u8bitsno,0);
		}

		u8bitsno ++;

		if (u8bitsno > 7)
		{
			u8bitsno = 0;
			modH->u8BufferSize++;
		}
	}

    // send outcoming message

    if (u16Coilno % 8 != 0)
	{
    	modH->u8BufferSize ++;
	}

    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);
    //stop :
    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC3
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 3 & 4
 * 					: This method reads a word array and transfers it to the master
 *****************************************************************************/

int8_t process_FC3(modbusHandler_t *modH)
{

    uint16_t u16StartAdd = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint8_t u8regsno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );
    uint16_t u8CopyBufferSize = 0;
    uint16_t i,index;
    union_Datatypes D1;

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];

    if(u8regsno % 2) // 4
    {
    	modH->u8Buffer[ 2 ]        = (u8regsno-1) * 2;
    }
    else
    {
    	modH->u8Buffer[ 2 ]        = u8regsno * 2;
    }
    modH->u8BufferSize         = 3;

    //for (i = u16StartAdd, index = 0; i < u16StartAdd + (u8regsno/2); ++i, ++index)
    for (i = u16StartAdd+1, index = 0; i < (u16StartAdd + u8regsno); i += 2, ++index)
    {
    	/* Physical DI*/
    	if((i >= 1) && (i < (1 + (16*2))))
		{
			D1.fl = gFinalAnaValF[1 + (i/2)];
		}
    	else if((i >= 67) && (i < (67 + (8*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else if((i >= 91) && (i < (91 + (6*2))))
    	{
    		D1.fl = gFinalAnaValF[i/2];
    	}
    	else if((i >= 103) && (i <= 113))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else if((i >= 115) && (i <= 129))
		{
			D1.fl = getFloatValueAddress(i);
		}
    	/* General purpose array */
    	else if((i >= START_IDX_MODBUS_ANA_PARA) && (i < (START_IDX_MODBUS_ANA_PARA + (MAX_GEN_ANA_PARA*2))))
    	{
    		D1.fl = gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+((i-START_IDX_MODBUS_ANA_PARA)/2)];
    	}
    	/* Modbus Slave parameters */
    	else if((i >= 761) && (i < (761 + (300*2))))
		{
			D1.fl = gFinalAnaValF[(i-160)/2];
		}
    	/* MISC parameters */
		else if((i >= 1361) && (i < (1361 + (98*2))))
		{
			D1.fl = gFinalAnaValF[(i-160)/2];
		}
    	/* Schedule */
    	else if((i >= 2971) && (i < (2971 + (420*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Modbus Read*/
    	else if((i >= 3891) && (i < (3891 + (300*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Modbus Write and Ack*/
    	else if((i >= 4491) && (i < (4491 + (40*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Scalling parameters */
    	else if((i >= 4571) && (i < (4571 + (150*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* Recipe parameters */
		else if((i >= RECIPE_VAR_START_MODBUS_INDEX) && (i <= RECIPE_VAR_END_MODBUS_INDEX))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* pulse DI parameters */
    	else if((i >=8701) && (i < (8701 + (80*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	/* pulse DO parameters */
    	else if((i >= 8861) && (i < (8861 + (130*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
		/* pulse Dual DO parameters */
    	else if((i >= 9201) && (i < (9201 + (26*2))))
		{
			D1.fl = gFinalAnaValF[i/2];
		}
    	else
    	{
    		D1.fl = 0;
    	}

    	modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[1];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[0];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[3];
		modH->u8BufferSize++;
		modH->u8Buffer[ modH->u8BufferSize ] = D1.s_ch[2];
		modH->u8BufferSize++;
    }
    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);

    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: getFloatValueAddress
 * arguments		: 1) registerAddress : uint16_t registerAddress
 * 		 		 	: 2)
 * return			: float value
 * Note				: This method is used to read the float values from the Modbus address
 * 					: This method returns the float value from the Modbus address
 *****************************************************************************/
float getFloatValueAddress(uint16_t registerAddress)
{

	switch (registerAddress)
	{
		case MODBUS_FLOAT_Lora_Frequency:return tempLora_Frequency;
		case MODBUS_FLOAT_Lora_Spreading_Factor:return tempLora_Spreading_Factor;
		case MODBUS_FLOAT_Lora_Bandwidth:return tempLora_Bandwidth;
		case MODBUS_FLOAT_Lora_Code_Rate  :return tempLora_Code_Rate;
		case MODBUS_FLOAT_Lora_Preamble_Length:return tempLora_Preamble_Length;
		case MODBUS_FLOAT_Lora_TX_Power :return tempLora_TX_Power;
		case MODBUS_FLOAT_Lora_P2P_PARAM_SET :return LoRa_Modem.lora_restart_request;
		case MODBUS_FLOAT_Modem_EC200_presence :return EPROM_General.Modem_EC200_presence;
		default: return 0;
	}
}

/**************************************************************************//**
 * Function name 	: process_FC5
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 5
 * 					: This method writes a value assigned by the master to a single bit
 *****************************************************************************/

int8_t process_FC5( modbusHandler_t *modH )
{
    uint16_t u16currentRegister;
    uint16_t u8CopyBufferSize;
    uint16_t u16coil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16val = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );

    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
    // point to the register and its bit
    u16currentRegister = u16coil;

    u16currentRegister++;

    if( (u16currentRegister >= 67) && (u16currentRegister <= (u16currentRegister + gpcbplcCnfg.mMaxDoEnabled)) )
	{
    	if(u16val == 0xFF00)
		{
			EPROM_General.DoModeDetails.DO_Value[u16currentRegister-67] = 1;
		}
		else
		{
			EPROM_General.DoModeDetails.DO_Value[u16currentRegister-67] = 0;
		}
	    flag_flashUpdateEPROM_General = 1;
	    flag_flashUpdateEPROM_General_WaitCounter=5;
	}

    // send answer to master
    modH->u8BufferSize = 6;
    u8CopyBufferSize =  modH->u8BufferSize +2;
    sendTxBuffer(modH);

    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC6
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 6
 * 					: This method writes a value assigned by the master to a single word
 *****************************************************************************/

int8_t process_FC6(modbusHandler_t *modH )
{

    uint16_t u16add = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u8CopyBufferSize;
    uint16_t k;
    uint16_t u16val = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );

    modH->u8Buffer[ID]     = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[FUNC]   = modH->u8RxBuffer[ FUNC ];
    modH->u8Buffer[ADD_HI] = (u16add >> 8) & 0x00FF;
    modH->u8Buffer[ADD_HI] = (u16add & 0x00FF);
    modH->u8Buffer[NB_HI]  = (u16val >> 8) & 0x00FF;
    modH->u8Buffer[NB_LO]  = (u16val & 0x00FF);
    modH->u8Buffer[CRC_HI] = (u16val >> 8) & 0x00FF; //@todo: calculate CRC
    modH->u8Buffer[CRC_LO] = (u16val & 0x00FF); //@todo: calculate CRC

    // keep the same header
    modH->u8BufferSize = RESPONSE_SIZE;

	k = u16add;
	switch(k+1)
	{
		case 5901:
		{

			break;
		}
		case 5902:
		{

			break;
		}
		case 5903:
		{

			break;
		}
		case 5904:
		{

			break;
		}
		case 5905:
		{

			break;
		}
		case 5906:
		{

			break;
		}
		case 5907:
		{

			break;
		}
		case 5908:
		{

			break;
		}
		case 5909:
		{

			break;
		}
		case 5910:
		{

			break;
		}
		case 5911:
		{

			break;
		}
		case 5912:
		{

			break;
		}
		default:
			break;
	}

    u8CopyBufferSize = modH->u8BufferSize + 2;
    sendTxBuffer(modH);

	return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC15
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 15
 * 					: This method writes a bit array assigned by the master
 *****************************************************************************/

int8_t process_FC15( modbusHandler_t *modH )
{
    uint8_t u8currentBit, u8frameByte, u8bitsno;
    uint16_t u16currentRegister;
    uint16_t u8CopyBufferSize;
    uint16_t u16currentCoil, u16coil;
    uint8_t bTemp;

    // get the first and last coil from the message
    uint16_t u16StartCoil = word( modH->u8RxBuffer[ ADD_HI ], modH->u8RxBuffer[ ADD_LO ] );
    uint16_t u16Coilno = word( modH->u8RxBuffer[ NB_HI ], modH->u8RxBuffer[ NB_LO ] );


    modH->u8Buffer[ ID ]       = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[ FUNC ]     = modH->u8RxBuffer[ FUNC ];
    // read each coil from the register map and put its value inside the outcoming message
    u8bitsno = 0;
    u8frameByte = 7;
    for (u16currentCoil = 0; u16currentCoil < u16Coilno; u16currentCoil++)
    {

        u16coil = u16StartCoil + u16currentCoil;
        u16currentRegister = (u16coil / 16);
        u8currentBit = (uint8_t) (u16coil % 16);

        bTemp = bitRead(
        			modH->u8Buffer[ u8frameByte ],
                    u8bitsno );

        bitWrite(
            modH->u16regs[ u16currentRegister ],
            u8currentBit,
            bTemp );

        u8bitsno ++;

        if (u8bitsno > 7)
        {
            u8bitsno = 0;
            u8frameByte++;
        }
    }

    // send outcoming message
    // it's just a copy of the incomping frame until 6th byte
    modH->u8BufferSize         = 6;
    u8CopyBufferSize = modH->u8BufferSize +2;
    sendTxBuffer(modH);
    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: process_FC16
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			:
 * return			: u8BufferSize Response to master length
 * Note				: This method processes functions 16
 * 					: This method writes a word array assigned by the master
 *****************************************************************************/

int8_t process_FC16(modbusHandler_t *modH )
{
    uint16_t u16StartAdd = modH->u8RxBuffer[ ADD_HI ] << 8 | modH->u8RxBuffer[ ADD_LO ];
    uint16_t u16regsno = modH->u8RxBuffer[ NB_HI ] << 8 | modH->u8RxBuffer[ NB_LO ];
    uint16_t u8CopyBufferSize = 0;
    uint16_t i, j, k;
    union_Datatypes D;
    float tValue = 0;

   if(ModbusH[COM_LORA].u8RxBuffer[4]==0 && ModbusH[COM_LORA].u8RxBuffer[5]==12)
   {
	   RTCsync_flage=1;
   }
    // build header
    modH->u8Buffer[ID]      = modH->u8RxBuffer[ ID ];
    modH->u8Buffer[FUNC]    = modH->u8RxBuffer[ FUNC ];
    modH->u8Buffer[ADD_HI]  = (u16StartAdd >> 8) & 0x00FF;
    modH->u8Buffer[ADD_LO]  = (u16StartAdd) & 0x00FF;
    modH->u8Buffer[NB_HI]   = (u16regsno >> 8) & 0x00FF;
    modH->u8Buffer[NB_LO]   = (u16regsno) & 0x00FF;
    //modH->u8Buffer[CRC_HI]  = 0;
    //modH->u8Buffer[CRC_LO]  = (uint8_t) u16regsno; // answer is always 256 or less bytes
    modH->u8BufferSize         = RESPONSE_SIZE;

    // write registers
//    for (i = 0; i < u16regsno; i++)
//    {
//    	temp = word(
//        		modH->u8Buffer[ (BYTE_CNT + 1) + i * 2 ],
//				modH->u8Buffer[ (BYTE_CNT + 2) + i * 2 ]);
//
//        modH->u16regs[u16StartAdd + i ] = temp;
//    }

    if(u16regsno <= 120)
    {
    	//FinalAnaValF[561]=16;
    	//FinalAnaValF[562]=u16regsno/2;
    	//FinalAnaValF[571]=u16StartAdd;
    	//k++;
		u16StartAdd++;
    	for(i = 0,j = 0, k = (u16StartAdd) ; j < (u16regsno/2) ; i = i+4,j++,k = k+2)
    	{
			D.ch[1] = modH->u8RxBuffer[7+i];
			D.ch[0] = modH->u8RxBuffer[7+i+1];
			D.ch[3] = modH->u8RxBuffer[7+i+2];
			D.ch[2] = modH->u8RxBuffer[7+i+3];
    		tValue = D.fl;

    		switch(k)
    		{
    			case 103: //Date
    			{
    				struct tm t, t1;
    				gFinalAnaValF[k/2] = tValue;

					t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
					t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
					t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
					//t.tm_wday = (gTimeInfo.WeekDay) - 1;
					if((tValue>0)&&(tValue<=31))
					{
						t.tm_mday = tValue;//bcdToDec(tValue);
					}
					else
					{
						t.tm_mday = gTimeInfo.mDate;
					}
					t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
					t.tm_year = gTimeInfo.mYear + 2000 - 1900;//bcdToDec(gTimeInfo.mYear) + 2000 - 1900;

					UTC = (uint64_t)mktime_new(&t);
    				set_time(t1);
    				break;
    			}
    			case 105: //month
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>0)&&(tValue<13))     //  An add for 1 to 12 month set
						{
							t.tm_mon = tValue-1;
						}
						else
						{
							t.tm_mon = gTimeInfo.mMonth-1;
						}
					}
					else
					{
						t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						if((tValue>0)&&(tValue<13))
						{
							t.tm_mon = tValue-1;
						}
						else
						{
							t.tm_mon = gTimeInfo.mMonth-1;
						}
						t.tm_mday = gTimeInfo.mDate;//bcdToDec(gTimeInfo.mDate);
						t.tm_year = gTimeInfo.mYear + 2000 - 1900;//bcdToDec(gTimeInfo.mYear) + 2000 - 1900;
					}
 						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 107://year
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>2000)&&(tValue<2100))
						{
							t.tm_year = tValue - 1900;
						}
						else
						{
							t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
						}
					}
					else
					{
						t.tm_hour = gTimeInfo.mHour;//bcdToDec(gTimeInfo.mHour);
						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
						t.tm_mday = gTimeInfo.mDate;//bcdToDec(gTimeInfo.mDate);
						if((tValue>2000)&&(tValue<2100))
						{
							t.tm_year = tValue - 1900;
						}
						else
						{
							t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
						}
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 109://hour
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>=0)&&(tValue<24))
						{
						t.tm_hour = tValue;//bcdToDec(tValue);
						}
						else
						{
						t.tm_hour = gTimeInfo.mHour;
						}

					}
					else
					{
						if((tValue>=0)&&(tValue<24))
						{
							t.tm_hour = tValue;//bcdToDec(tValue);
						}
						else
						{
							t.tm_hour = gTimeInfo.mHour;
						}

						t.tm_min = gTimeInfo.mMinute;//bcdToDec(gTimeInfo.mMinute);
						t.tm_sec = gTimeInfo.mSecond;//bcdToDec(gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = gTimeInfo.mMonth-1;//bcdToDec(gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 111://minute
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						if((tValue>=0)&&(tValue<60))
						{
						t.tm_min = (tValue);
						}
						else
						{
						t.tm_min = gTimeInfo.mMinute;
						}
					}
					else
					{
						t.tm_hour = (gTimeInfo.mHour);
						if((tValue>=0)&&(tValue<60))
						{
							t.tm_min = (tValue);
						}
						else
						{
							t.tm_min = gTimeInfo.mMinute;
						}

						t.tm_sec = (gTimeInfo.mSecond);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = (gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
    			case 113://second
				{
					struct tm t, t1;
					gFinalAnaValF[k/2] = tValue;
					if(RTCsync_flage==1)
					{
						RTCsync_flage=0;
						memset(ModbusH[COM_LORA].u8RxBuffer,0,300);
						if((tValue>=0)&&(tValue<60))
						{
						t.tm_sec = (tValue);
						}
						else
						{
						t.tm_sec = (gTimeInfo.mSecond);
						}
					}
					else
					{
						t.tm_hour = (gTimeInfo.mHour);
						t.tm_min = (gTimeInfo.mMinute);
						if((tValue>=0)&&(tValue<60))
						{
							t.tm_sec = (tValue);
						}
						else
						{
							t.tm_sec = (gTimeInfo.mSecond);
						}
						t.tm_sec = (tValue);
						//t.tm_wday = (gTimeInfo.WeekDay) - 1;
						t.tm_mon = (gTimeInfo.mMonth) - 1;
						t.tm_mday = (gTimeInfo.mDate);
						t.tm_year = (gTimeInfo.mYear) + 2000 - 1900;
					}
						UTC = (uint64_t)mktime_new(&t);

					set_time(t1);
					break;
				}
				// LoRA RF Parameters will only be saved after writing 1 on P2P_PARAM_SET
    			case MODBUS_FLOAT_Lora_Frequency:
				if ((tValue >= 865000000) && (tValue <= 867000000) )
				{
					tempLora_Frequency = (uint32_t)tValue;
				}
				break;
    			case MODBUS_FLOAT_Lora_Spreading_Factor:
    				if ((tValue >= 6) && (tValue < 13) )
    				{
    					tempLora_Spreading_Factor = (uint32_t)tValue;
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Bandwidth:
    				if ((tValue >= 0) && (tValue < 10) )
    				{
    					tempLora_Bandwidth = (uint32_t)tValue;
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Code_Rate:
    				if ((tValue >= 0) && (tValue < 4) )
    				{
    					tempLora_Code_Rate = (uint32_t)tValue;
    				}
    				break;
    			case MODBUS_FLOAT_Lora_Preamble_Length:
    				if ((tValue >= 2) && (tValue <= 65535) )
    				{
    					tempLora_Preamble_Length = (uint32_t)tValue;
    				}
    				break;
    			case MODBUS_FLOAT_Lora_TX_Power:
    				if ((tValue >= 5) && (tValue < 23) )
    				{
    					tempLora_TX_Power = (uint32_t)tValue;
    				}
    				break;
    			case MODBUS_FLOAT_Lora_P2P_PARAM_SET:
				if (tValue == 1)
				{
					LoRa_Modem.lora_restart_request = tValue;
					EPROM_General.Lora_Frequency = tempLora_Frequency;
					EPROM_General.Lora_Spreading_Factor = tempLora_Spreading_Factor;
					EPROM_General.Lora_Bandwidth = tempLora_Bandwidth;
					EPROM_General.Lora_Code_Rate = tempLora_Code_Rate;
					EPROM_General.Lora_Preamble_Length = tempLora_Preamble_Length;
					EPROM_General.Lora_TX_Power = tempLora_TX_Power;
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter=5;
					flagLORAPubLogData = 1;
				}
				else
				{
					tempLora_Frequency = EPROM_General.Lora_Frequency;
					tempLora_Spreading_Factor = EPROM_General.Lora_Spreading_Factor;
					tempLora_Bandwidth = EPROM_General.Lora_Bandwidth;
					tempLora_Code_Rate = EPROM_General.Lora_Code_Rate;
					tempLora_Preamble_Length = EPROM_General.Lora_Preamble_Length;
					tempLora_TX_Power = EPROM_General.Lora_TX_Power;
				}
				break;
    			case MODBUS_FLOAT_Modem_EC200_presence:
				if ((tValue >= 0) && (tValue < 2) )
				{
					EPROM_General.Modem_EC200_presence = (uint32_t)tValue;
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter=5;
					MODEM_GPIO_Define();
				}
    			break;
    			case 5901:
    			case 5902:
				{

					break;
				}
    			case 5903:
    			case 5904:
				{

					break;
				}
    			case 5905:
    			case 5906:
				{

					break;
				}
    			case 5907:
    			case 5908:
				{

					break;
				}
    			case 5909:
    			case 5910:
				{

					break;
				}
    			case 5911:
    			case 5912:
				{

					break;
				}
    			default:
    			{
    				if(k >= 1800 && k <= 9301 && (k%2) == 0)//if(k >= 1800 && k <= 9201 && (k%2) == 0)
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1800) && (k <= 9301) && ((k%2) == 1))//else if((k >= 1800) && (k <= 9201) && ((k%2) == 1))
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1201) && (k <= 1243) && ((k%2) == 1))
					{
						ControlLogic (k/2, tValue);
					}
    				else if((k >= 1361) && (k <= 1555) && ((k%2) == 1))
					{
						ControlLogic ((k-160)/2, tValue);
					}
    				else if(k >= 161 && k <= 759 && (k%2) == 1)
    				{
    					ControlLogic (START_IDX_GEN_ANA_PARA_TAG+((k-START_IDX_MODBUS_ANA_PARA)/2), tValue);
    				}
    				else if((k >= RECIPE_VAR_START_MODBUS_INDEX) &&
    						(k <= RECIPE_VAR_END_MODBUS_INDEX) &&
							((RECIPE_VAR_START_MODBUS_INDEX % 2) == 1))
					{
						ControlLogic(k/2, tValue);
					}
					else
					{
						//RTU_para_WriteMod(k/2,tValue);
					}
    				break;
    			}
    		}
    	}

//    	if((k/2)>=(785+220+(MAXSCH*3)) && (k/2)<=(785+220+(MAXSCH*3)+MAX3PARA))
//		{}
//		else
//		{	InsertIntoFlashBuff(__LINE__);;
//			SaveTOEprom=1;
//
//		}

    	u8CopyBufferSize = 8;
    	sendTxBuffer(modH);
    }


    return u8CopyBufferSize;
}

/**************************************************************************//**
 * Function name 	: get_FC1
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: modbus_t *telegram
 * return			:
 * Note				: This method processes functions 1 & 2 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/

void get_FC1(modbusHandler_t *modH,modbus_t *telegram )
{
    uint8_t  i,j,k;
     /*for (i=0; i< modH->u8Buffer[2]; i++) {

        if(i%2)
        {
        	modH->u16regs[i/2]= word(modH->u8Buffer[i+u8byte], lowByte(modH->u16regs[i/2]));
        }
        else
        {

        	modH->u16regs[i/2]= word(highByte(modH->u16regs[i/2]), modH->u8Buffer[i+u8byte]);
        }

     }*/

	modH->u8BufferSize = modH->u8RxBuffer[2]+3+2;
	telegram->u8rxdataValidation = validateAnswer(modH);
	if (telegram->u8rxdataValidation != 0)
	{

	}
	else
	{
		if(telegram->uDataType==DIGITAL_TYPE)
		{
			for(j = 3,i = 0 ; j < (modH->u8RxBuffer[2]+3) ; j++)
			{
				for(k = 0 ; k < 8 ; k++)
				{
					Digital_bit_Query_array[k] = ((modH->u8RxBuffer[j]>>k) & 0x01);
					//gFinalAnaValF[telegram->u16addressRegisterMap + i] = Digital_bit_Query_array[k];
					//FinalAnaValF[k]=Digital_bit_Query_array[k];
					dig_bit_array[telegram->u16addressRegisterMap + i] = Digital_bit_Query_array[k];
					i++;
					if(i >= telegram->u8noOfData)//Rx_data[3])
					{
						break;
					}
				}
			}
		}
	}
}

/**************************************************************************//**
 * Function name 	: get_FC3
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) modbus_t *telegram
 * return			:
 * Note				: This method processes functions 3 & 4 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/

void get_FC3(modbusHandler_t *modH,modbus_t *telegram)
{
    uint16_t  i,j;
    float LocalFloat=0.0;
    union_Datatypes D1;

	modH->u8BufferSize = modH->u8RxBuffer[2]+3+2;
	telegram->u8rxdataValidation = validateAnswer(modH);
	if (telegram->u8rxdataValidation == 0)
	{
		for(j = 3,i = 0 ; j < (modH->u8RxBuffer[2]+3) ; i++)
		{
			if(telegram->uDataType==FLOAT_TYPE)				// Float - Flow Computer
			{
				D1.ch[1] =  modH->u8RxBuffer[j+0];
				D1.ch[0] =  modH->u8RxBuffer[j+1];
				D1.ch[3] =  modH->u8RxBuffer[j+2];
				D1.ch[2] =  modH->u8RxBuffer[j+3];
				LocalFloat = D1.fl;
				j += 4;
			}
			else if(telegram->uDataType==SWFLOAT_TYPE)					//Swapped Float - Flow Computer
			{
				D1.ch[3] =  modH->u8RxBuffer[j+0];
				D1.ch[2] =  modH->u8RxBuffer[j+1];
				D1.ch[1] =  modH->u8RxBuffer[j+2];
				D1.ch[0] =  modH->u8RxBuffer[j+3];
				LocalFloat = D1.fl;
				j += 4;
			}
			else if(telegram->uDataType==INTEGER_TYPE || telegram->uDataType == UNSIGNED32_TYPE)					//Interger-32 bits - Flow Computer
			{
				D1.ch[0] =  modH->u8RxBuffer[j+3];
				D1.ch[1] =  modH->u8RxBuffer[j+2];
				D1.ch[2] =  modH->u8RxBuffer[j+1];
				D1.ch[3] =  modH->u8RxBuffer[j+0];
				LocalFloat = D1.uint;
				j += 4;
			}
			else if(telegram->uDataType == SIGNED32_TYPE)					//Interger-32 bits - Flow Computer
			{
				D1.ch[0] =  modH->u8RxBuffer[j+3];
				D1.ch[1] =  modH->u8RxBuffer[j+2];
				D1.ch[2] =  modH->u8RxBuffer[j+1];
				D1.ch[3] =  modH->u8RxBuffer[j+0];
				LocalFloat = D1.int_32;
				j += 4;
			}
			else if(telegram->uDataType == UNSIGNED16_TYPE)			//Interger-16 bits - Flow Computer
			{
				D1.ch[0] =  modH->u8RxBuffer[j+1];
				D1.ch[1] =  modH->u8RxBuffer[j+0];
				LocalFloat = D1.sh;
				j += 2;
			}
			else if(telegram->uDataType == SIGNED16_TYPE)			//Interger-16 bits - Flow Computer
			{
				D1.ch[0] =  modH->u8RxBuffer[j+1];
				D1.ch[1] =  modH->u8RxBuffer[j+0];
				LocalFloat = D1.sh_16;
				j += 2;
			}
			else if(telegram->uDataType == UNSIGN_CHAR_STRING_TYPE)			//Interger-16 bits - Flow Computer
			{
						D1.ch[0] =  modH->u8RxBuffer[j];
						//D1.ch[1] =  modH->u8RxBuffer[j+0];
						LocalFloat = D1.ch[0];
						j += 1;
			}
			else if(telegram->uDataType == CHAR_STRING_TYPE)			//Interger-16 bits - Flow Computer
			{
						D1.s_ch[0] =  modH->u8RxBuffer[j];
						//D1.ch[1] =  modH->u8RxBuffer[j+0];
						LocalFloat = D1.s_ch[0];
						j += 1;
			}
			else
			{
				break;
			}
			if(i < telegram->u8noOfData)  // RS port hand if above 255 value ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-20
			{
				gFinalAnaValF[telegram->u16addressRegisterMap+i] = LocalFloat;
			}
			else
			{
				break;
			}
		}
	}
}
/**************************************************************************//**
 * Function name 	: get_FC5_FC6
 * arguments		: 1) modH : ModbusPort Handler
 * 		 			: 2) modbus_t *telegram
 * return			:
 * Note				: This method processes functions 5 & 6 (for master)
 * 					: This method puts the slave answer into master data buffer
 *****************************************************************************/
void get_FC5_FC6(modbusHandler_t *modH,modbus_t *telegram )
{
	//uint8_t  i,j,k;
	//uint8_t  tSlave_id = modH->u8Buffer[0];
	uint8_t  tFunction_code = modH->u8Buffer[1];

	if((tFunction_code == 0x05) || (tFunction_code == 0x6) || (tFunction_code == 0xF) || (tFunction_code == 0x10))
	{
		gFinalAnaValF[MODBUS_WRITE_QUERY_ACK_gFinalAnaValF + ((telegram->uQueryNo)-1)] = 1;
	}
	else
	{
		gFinalAnaValF[MODBUS_WRITE_QUERY_ACK_gFinalAnaValF + ((telegram->uQueryNo)-1)] = 0;
	}
}
/**************************************************************************//**
 * Function name 	: BuildModbusMasterQueryTelegrams
 * arguments		: 1)
 * 		 			:
 * return			:
 * Note				: Make Basic Telegram From Flash Data / RegisterMap
 * 					: Fill Address of Register Map in each Query Telegram using previousQuery + PreviousQuerylength
 * 					: Validation check for Query
 *****************************************************************************/

void changeRTUIDinSlaveLogic()
{
	ModbusH[COM_RS232_1].u8id = EPROM_General.Rtu_Detail.RTUId; //slave ID
	ModbusH[COM_LORA].u8id = EPROM_General.Rtu_Detail.RTUId; 			//slave ID
}

void BuildModbusMasterQueryTelegrams()
{
	ModbusH[COM_RS232_1].uModbusType = EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu;//MB_DEBUG;
	ModbusH[COM_RS232_1].port =  &huart1; // This is the UART port connected to RS232
	ModbusH[COM_RS232_1].u8id = 1; //slave ID
	ModbusH[COM_RS232_1].u16regs = ModbusDATA;
	ModbusH[COM_RS232_1].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
	ModbusH[COM_RS232_1].u8Port = COM_RS232_1;

	ModbusH[COM_LORA].uModbusType = MB_SLAVE;//MB_SLAVE;
	ModbusH[COM_LORA].port = &huart1;; // This is the UART port connected to RS232
	ModbusH[COM_LORA].u8id = 1; //slave ID should be 1 as SCADA uses 1
	ModbusH[COM_LORA].u16regs = ModbusDATA;
	ModbusH[COM_LORA].u16regsize= sizeof(ModbusDATA)/sizeof(ModbusDATA[0]);
	ModbusH[COM_LORA].u8Port = COM_LORA; // maulin needs to change for lora

	if(Pro_Application_flag)
	{
		ModbusH[COM_RS232_1].uModbusType = MB_SLAVE;
	}

}
/**************************************************************************//**
 * Function name 	: ProcessModbusSlave
 * arguments		: 1) modbusHandler_t *modH
 * 		 			:
 * return			:
 * Note				: In Slave Mode Receive Modbus Query and Proccess Accordingly
 * 					: 
 *****************************************************************************/
void ProcessModbusSlave(uint8_t port)
{
	modbusHandler_t *modH = &ModbusH[port];

   // check slave id
	if ( modH->u8RxBuffer[ID] !=  modH->u8id)
	{
		return; // return from function if Slave ID is not matched
	}

	modH->u8BufferSize = (word(modH->u8RxBuffer[NB_HI],modH->u8RxBuffer[NB_LO])*2)+1+1+2+2+2;

	 // process message
	 switch(modH->u8RxBuffer[ FUNC ] )
	 {
			case MB_FC_READ_COILS:
			case MB_FC_READ_DISCRETE_INPUT:
				modH->i8state = process_FC1(modH);
				break;
			case MB_FC_READ_INPUT_REGISTER:
				modH->i8state = process_FC1(modH);//@todo
				break;
			case MB_FC_READ_REGISTERS :
				modH->i8state = process_FC3(modH);
				break;
			case MB_FC_WRITE_COIL:
				modH->i8state = process_FC5(modH);
				break;
			case MB_FC_WRITE_REGISTER :
				modH->i8state = process_FC6(modH);
				break;
			case MB_FC_WRITE_MULTIPLE_COILS:
				modH->i8state = process_FC15(modH);
				break;
			case MB_FC_WRITE_MULTIPLE_REGISTERS :
				modH->i8state = process_FC16(modH);
				break;
			default:

				break;
	 }
	// xSemaphoreGive(modH->ModBusSphrHandle); //Release the semaphore
}

float swap_float(float input)
{
    union
    {
        float f;
        uint8_t b[4];
    } in, out;

    in.f = input;
    out.f = input;
    // ABCD -> BADC
    out.b[0] = in.b[1]; // C
    out.b[1] = in.b[0]; // D
    out.b[2] = in.b[3]; // A
    out.b[3] = in.b[2]; // B

    return out.f;
}
