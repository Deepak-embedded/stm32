/*
 * modbus.h
 *
 *  Created on: Nov 15, 2022
 *      Author: SanketP
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "stdbool.h"
/**************************************************************************//**
 * Constant
 *****************************************************************************/
//#define MAX_QUERY  			(50)
#define MAX_WRITE_QUERY		(1)
#define MAX_WRITE_QUERY_ACK	(32)

#define T35  				5              // Timer T35 period (in ticks) for end frame detection.
#define MAX_BUFFER  		300	    // Maximum size for the communication buffer in bytes.
#define TIMEOUT_MODBUS 		1000 // Timeout for master query (in ticks)
#define MAX_M_HANDLERS 		2    //Maximum number of modbus handlers that can work concurrently
#define MAX_TELEGRAMS 		2     //Max number of Telegrams in master queue

// Values copied from the ModbusF429TCP example project.
#define NUMBERTCPCONN   4   // Maximum number of simultaneous client connections, it should be equal or less than LWIP configuration
#define TCPAGINGCYCLES	20000 //200 // Number of times the master will check for a incoming request before closing the connection for inactivity
/* Note: the total aging time for a connection is approximately NUMBERTCPCONN*TCPAGINGCYCLES*u16timeOut ticks
 * for the values selected in this example it is approximately 40 seconds
*/

//#define	MAX_BUFFER		40
#define RESPONSE_SIZE 		6
#define	ID					0 	//!< ID field
#define	FUNC				1 	//!< Function code position
#define	ADD_HI				2 	//!< Address high byte
#define	ADD_LO 				3 	//!< Address low byte
#define	NB_HI 				4 	//!< Number of coils or registers high byte
#define	NB_LO 				5 	//!< Number of coils or registers low byte
#define CRC_HI				6
#define CRC_LO				7
#define	BYTE_CNT			6 	//!< byte counter

#define COM_RS232_1 		0
#define COM_LORA  			1

#define DIGITAL_TYPE			0
#define UNSIGNED1_TYPE			1 ///
#define UNSIGNED16_TYPE			2
#define INTEGER_TYPE			3
#define SIGNED32_TYPE			4
#define FLOAT_TYPE				5
#define SWFLOAT_TYPE			6
#define UNSIGNED32_TYPE			7
#define SINGED24_TYPE			8
#define SIGNED16_TYPE			9
#define CHAR_STRING_TYPE		10
#define UNSIGN_CHAR_STRING_TYPE 11

//#define DIGITAL_TYPE	0
//#define UNSIGNED1_TYPE	1 ///
//#define UNSIGNED16_TYPE	2
//#define INTEGER_TYPE	3
//#define SIGNED32_TYPE	4
//#define FLOAT_TYPE		5
//#define SWFLOAT_TYPE	6
//#define UNSIGNED32_TYPE	7
//#define SINGED24_TYPE	8
//#define SIGNED16_TYPE	9

#define MODBUS_MASTER_MAX_DATA_IN_ONE_QUERY							128
#define MODBUS_MASTER_MAX_SLAVE_ID_IN_ONE_COM_PORT					10
#define MODBUS_MASTER_MAX_QUERY_ON_ONE_COM_PORT						32
#define MODBUS_MASTER_MAX_TOTAL_QUERY								50
#define MODBUS_MASTER_MAX_MAX_TOTAL_DATA_DIGITAL					50//2560
#define MODBUS_MASTER_MAX_MAX_TOTAL_DATA_ANALOG						300//2560

#define MODBUS_QUERY_VALIDATION_OK                  				0
#define MODBUS_QUERY_VALIDATION_DATA_TYPE_MISMATCH  				1
#define MODBUS_QUERY_VALIDATION_MAX_SLAVE_ID_LIMIT_EXCEED 			2
#define MODBUS_QUERY_VALIDATION_MAX_TOTAL_DATA_LIMIT_EXCEED 		4
#define MODBUS_QUERY_VALIDATION_MAX_DATA_IN_ONE_QUERY_LIMIT_EXCEED 	8
#define MODBUS_QUERY_VALIDATION_SLAVE_ID_NOT_IN_RANGE			 	16
#define MODBUS_QUERY_VALIDATION_WRONG_FUNCTION_CODE					32
#define MODBUS_QUERY_VALIDATION_DATA_TYPE_NOT_VALID					64

#define MODBUS_FLOAT_Lora_Frequency  								115
#define MODBUS_FLOAT_Lora_Spreading_Factor  						117
#define MODBUS_FLOAT_Lora_Bandwidth  								119
#define MODBUS_FLOAT_Lora_Code_Rate  								121
#define MODBUS_FLOAT_Lora_Preamble_Length  							123
#define MODBUS_FLOAT_Lora_TX_Power  								125
#define MODBUS_FLOAT_Lora_P2P_PARAM_SET								127
#define MODBUS_FLOAT_Modem_EC200_presence  							129
/**************************************************************************//**
 * Enum / union / Structure
 *****************************************************************************/
enum
{
    EXC_FUNC_CODE = 1,
    EXC_ADDR_RANGE = 2,
    EXC_REGS_QUANT = 3,
    EXC_EXECUTE = 4
};

typedef enum
{
    USART_HW = 1,
    USB_CDC_HW = 2,
    TCP_HW = 3,
	USART_HW_DMA = 4,
}mb_hardware_t ;

typedef enum
{
    MB_SLAVE = 3,
    MB_MASTER = 4,
	MB_DEBUG = 5
}mb_masterslave_t ;

typedef struct
{
	struct netconn *conn;
	uint32_t aging;
}
tcpclients_t;

typedef struct
{
	mb_masterslave_t uModbusType;
	UART_HandleTypeDef *port; //HAL Serial Port handler
	uint8_t u8id; //!< 0=master, 1..247=slave number
	uint8_t u8Buffer[MAX_BUFFER]; //Modbus buffer for communication
	uint8_t u8RxBuffer[MAX_BUFFER]; //Modbus buffer for communication
	uint16_t u8BufferSize;
	uint16_t *u16regs;
	uint8_t u8Datatype;
	uint16_t u16regsize;
	int8_t i8state;
	uint8_t u8FunctionCode;
	uint8_t u8Port; // communication port [COM_RS232_1 : 0]	[COM_LORA : 1]

	uint16_t u16TransactionID;
	uint16_t u16timeOut;
}
modbusHandler_t;

typedef enum MB_FC
{
    MB_FC_READ_COILS               = 1,	 /*!< FCT=1 -> read coils or digital outputs */
    MB_FC_READ_DISCRETE_INPUT      = 2,	 /*!< FCT=2 -> read digital inputs */
    MB_FC_READ_REGISTERS           = 3,	 /*!< FCT=3 -> read registers or analog outputs */
    MB_FC_READ_INPUT_REGISTER      = 4,	 /*!< FCT=4 -> read analog inputs */
    MB_FC_WRITE_COIL               = 5,	 /*!< FCT=5 -> write single coil or output */
    MB_FC_WRITE_REGISTER           = 6,	 /*!< FCT=6 -> write single register */
    MB_FC_WRITE_MULTIPLE_COILS     = 15, /*!< FCT=15 -> write multiple coils or outputs */
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16	 /*!< FCT=16 -> write multiple registers */
}mb_functioncode_t;

typedef struct
{
	uint8_t uQueryNo;
	uint8_t uPortNo;
    uint8_t u8id;          				/*!< Slave address between 1 and 247. 0 means broadcast */
    mb_functioncode_t u8fct;         	/*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
    uint16_t u16RegAdd;    				/*!< Address of the first register to access at slave/s */
    uint16_t u16CoilsNo;   				/*!< Number of coils or registers to access */
    uint16_t *u16reg;     				/*!< Pointer to memory image in master */
    uint32_t *u32CurrentTask; 			/*!< Pointer to the task that will receive notifications from Modbus */
    uint16_t u16addressRegisterMap;
    uint8_t uDataType;
    uint8_t u8Validation;
    uint8_t u8noOfData;
    uint8_t u8rxdataValidation;
}modbus_t;

/**************************************************************************//**
 * Macro
 *****************************************************************************/

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

#define lowByte(w) ((w) & 0xff)
#define highByte(w) ((w) >> 8)

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern uint8_t Digital_bit_Query_array[];
extern uint8_t gNoofQueryStored;
extern uint16_t gTotalNumberOfReadDataUsingAllQuery;
extern modbus_t telegram[];
extern modbusHandler_t ModbusH[];
extern uint16_t ModbusDATA[100];
extern char Lora_Modem_Ascii_String_app_eui[],Lora_Modem_Ascii_String_app_key[],Lora_Modem_Ascii_String_dev_eui[];

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void ModbusInit(modbusHandler_t * modH);
void ModbusInitData(modbus_t*);
uint16_t calcCRC(uint8_t *Buffer, uint8_t u8length);
void sendTxBuffer(modbusHandler_t *modH);
int8_t SendQuery(modbusHandler_t *modH ,  modbus_t *telegram );
void Master_Send_Modbus_Query(void *argument,modbus_t *telegram); //slave
void Master_Parse_Modbus_Responce(void *argument,void *argument2);
uint16_t calcCRC(uint8_t *Buffer, uint8_t u8length);
int8_t process_FC1(modbusHandler_t *modH);
int8_t process_FC3(modbusHandler_t *modH);
int8_t process_FC5(modbusHandler_t *modH);
int8_t process_FC6(modbusHandler_t *modH);
int8_t process_FC15(modbusHandler_t *modH);
int8_t process_FC16(modbusHandler_t *modH);
void get_FC1(modbusHandler_t *modH,modbus_t *telegram );
void get_FC3(modbusHandler_t *modH,modbus_t *telegram );
void get_FC5_FC6(modbusHandler_t *modH,modbus_t *telegram );
uint8_t validateAnswer(modbusHandler_t *modH);
void changeRTUIDinSlaveLogic();
void BuildModbusMasterQueryTelegrams();
void ProcessModbusSlave(uint8_t port);	//modbusHandler_t *modH);
void  TCPinitserver(modbusHandler_t *modH);
bool TCPwaitConnData(modbusHandler_t *modH);
void ModbusCloseConn(struct netconn *conn); //close the TCP connection
void ModbusCloseConnNull(modbusHandler_t * modH); //close the TCP connection and cleans the modbus handler
float getFloatValueAddress(uint16_t registerAddress);
float swap_float(float input);

extern void ControlLogic(unsigned int Mod_Address, float Mod_Value);

#endif /* INC_MODBUS_H_ */
