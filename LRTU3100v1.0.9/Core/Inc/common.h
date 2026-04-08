/*
 * common.h
 *
 *  Created on: Nov 21, 2022
 *      Author: maulin
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

/**************************************************************************//**
 * Constant
 *****************************************************************************/
#define MAX_NODES		30
#define UPLINK_BUF_LEN 	2*MAX_NODES
#define MAC_LEN 		16	// 8 bytes
#define MSG_ID_LEN		2 	// 1 byte
#define MSG_TYPE_LEN 	2 	// 1 byte
#define MAGIC_TAIL   	"AC1F0905"
#define MIN_MS			60000
#define HOUR_MS			3600000	// 1 hour

//RegisterMap Address
#define REG_MAP_START_ADDRESS_TO_STORE_ANALOG_DATA 				45
#define REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA_DIGITAL 16 // 0 to 15 physical Di  and 16 to ... Modbus DI
#define REG_MAP_START_ADDRESS_TO_STORE_READ_MODBUS_DATA 		300
#define REG_MAP_START_ADDRESS_TO_STORE_AI_TAG					300

#define PRODUCTION_TEST_BIT_DO			0
#define PRODUCTION_TEST_BIT_DI			1
#define PRODUCTION_TEST_BIT_RTC			2
#define PRODUCTION_TEST_BIT_GPS			3
#define PRODUCTION_TEST_BIT_BLE			4
#define PRODUCTION_TEST_BIT_4G			5
#define PRODUCTION_TEST_BIT_FLASH		6
#define PRODUCTION_TEST_BIT_RS485_1		7
#define PRODUCTION_TEST_BIT_RS485_2		8
#define PRODUCTION_TEST_BIT_RS232_1		9
#define PRODUCTION_TEST_BIT_RS232_2		10
#define PRODUCTION_TEST_BIT_SD_CARD		11
#define PRODUCTION_TEST_BIT_KEY_LED		12
#define PRODUCTION_TEST_BIT_WATCHDOG	13

/**************************************************************************//**
 * TYPE Def
 *****************************************************************************/
typedef union
{
	unsigned char ch[4];
	signed char s_ch[4];
	unsigned short sh;
	unsigned int uint;
	float fl;
	int int_32;
	short sh_16;
} union_Datatypes ;

typedef union {
	uint8_t  u8[4];
	uint16_t u16[2];
	uint32_t u32;

} bytesFields ;


// A structure with forced alignment
typedef struct
{
	uint8_t variable0 	: 1;
	uint8_t variable1 	: 1;
	uint8_t variable2 	: 1;
	uint8_t variable3 	: 1;
	uint8_t variable4 	: 1;
	uint8_t variable5 	: 1;
	uint8_t variable6 	: 1;
	uint8_t variable7 	: 1;
}flag_ChangeRequired;

typedef enum {
    LORA_LOGRATE = 1,
    LORA_CMD,
    LORA_ACK,
	LORA_DL_ACK,	// ACK for downlink
    LORA_EVENT,
	LORA_RTC_SYNC,
	LORA_OTA_START_MSG,
	LORA_OTA_CHNK_MSG,
	LORA_OTA_NACK_MSG,
	LORA_OTA_ACK_MSG,
	LORA_OTA_VERIFY_MSG
} LoraMessageType;

typedef struct
{
	uint8_t	 ota_type;
	uint8_t	 ota_state;
    uint16_t totalChunks;
    uint16_t curentChunk;
    uint8_t	 chunkSize;
    uint8_t  sessionId;
    uint16_t forVer;
    char	 bin[330];
    uint16_t index;
    uint32_t fileSize;
    uint16_t chunkCrc;
    uint8_t  fileCrc;
    bool	 ready;
} LoraOta_t;

typedef enum {
    LORA_OTA_BEGIN = 1,
	LORA_OTA_PUSHING,
	LORA_OTA_DOWNLOADING,
	LORA_OTA_VERIFY,
	LORA_OTA_COMPLETE,
	LORA_OTA_FAILED
} OtaState_t;

typedef enum {
    OTA_DELTA_H7 = 1,
    OTA_DELTA_G0
} OtaType_t;

typedef struct {
    char macId[MAC_LEN+1];
    uint32_t timeoutTick;
} nodeEntry_t;

/**************************************************************************//**
 * Macro
 *****************************************************************************/
/**
 * \brief           Calculate length of statically allocated array
 */
#define ARRAY_LEN(x)            (sizeof(x) / sizeof((x)[0]))

///**
// * \brief           Assert an input parameter if in valid range
// * \note            Since this is a macro, it may only be used on a functions where return status is of type \ref lwgsmr_t enumeration
// * \param[in]       c: Condition to test
// */
/*
#define LWGSM_ASSERT(c)                                                                                           		\
    do {                                                                                                               \
        if (!(c)) {                                                                                                    \
            LWGSM_DEBUGF(LWGSM_CFG_DBG_ASSERT, "Assert failed in file %s on line %d: %s\r\n", __FILE__, (int)__LINE__, \
                         #c);                                                                                          \
            return lwgsmERRPAR;                                                                                        \
        }                                                                                                             \
    } while (0)
*/
/**
 * \brief           Align `x` value to specific number of bytes, provided by \ref LWGSM_CFG_MEM_ALIGNMENT configuration
 * \param[in]       x: Input value to align
 * \return          Input value aligned to specific number of bytes
 * \hideinitializer
 */
#define LWGSM_MEM_ALIGN(x)                ((x + (LWGSM_CFG_MEM_ALIGNMENT - 1)) & ~(LWGSM_CFG_MEM_ALIGNMENT - 1))

/**
 * \brief           Get minimal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Minimal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWGSM_MIN(x, y)                   ((x) < (y) ? (x) : (y))

/**
 * \brief           Get maximal value between `x` and `y` inputs
 * \param[in]       x: First input to test
 * \param[in]       y: Second input to test
 * \return          Maximal value between `x` and `y` parameters
 * \hideinitializer
 */
#define LWGSM_MAX(x, y)                   ((x) > (y) ? (x) : (y))

/**
 * \brief           Get size of statically declared array
 * \param[in]       x: Input array
 * \return          Number of array elements
 * \hideinitializer
 */
#define LWGSM_ARRAYSIZE(x)                (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           Unused argument in a function call
 * \note            Use this on all parameters in a function which are not used to prevent
 *                  compiler warnings complaining about "unused variables"
 * \param[in]       x: Variable which is not used
 * \hideinitializer
 */
#define LWGSM_UNUSED(x)                   ((void)(x))

/**
 * \brief           Get input value casted to `unsigned 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U32(x)                      ((uint32_t)(x))

/**
 * \brief           Get input value casted to `unsigned 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U16(x)                      ((uint16_t)(x))

/**
 * \brief           Get input value casted to `unsigned 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_U8(x)                       ((uint8_t)(x))

/**
 * \brief           Get input value casted to `signed 32-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I32(x)                      ((int32_t)(x))

/**
 * \brief           Get input value casted to `signed 16-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I16(x)                      ((int16_t)(x))

/**
 * \brief           Get input value casted to `signed 8-bit` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_I8(x)                       ((int8_t)(x))

/**
 * \brief           Get input value casted to `size_t` value
 * \param[in]       x: Input value
 * \hideinitializer
 */
#define LWGSM_SZ(x)                       ((size_t)(x))

/**
 * \brief           Convert `unsigned 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u32_to_str(num, out)        lwgsm_u32_to_gen_str(LWGSM_U32(num), (out), 0, 0)

/**
 * \brief           Convert `unsigned 32-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u32_to_hex_str(num, out, w) lwgsm_u32_to_gen_str(LWGSM_U32(num), (out), 1, (w))

/**
 * \brief           Convert `signed 32-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i32_to_str(num, out)        lwgsm_i32_to_gen_str(LWGSM_I32(num), (out))

/**
 * \brief           Convert `unsigned 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u16_to_str(num, out)        lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U16(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u16_to_hex_str(num, out, w) lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U16(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 16-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i16_to_str(num, out)        lwgsm_i32_to_gen_str(LWGSM_I32(LWGSM_I16(num)), (out))

/**
 * \brief           Convert `unsigned 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u8_to_str(num, out)         lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U8(num)), (out), 0, 0)

/**
 * \brief           Convert `unsigned 16-bit` number to HEX string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \param[in]       w: Width of output string.
 *                      When number is shorter than width, leading `0` characters will apply.
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_u8_to_hex_str(num, out, w)  lwgsm_u32_to_gen_str(LWGSM_U32(LWGSM_U8(num)), (out), 1, (w))

/**
 * \brief           Convert `signed 8-bit` number to string
 * \param[in]       num: Number to convert
 * \param[out]      out: Output variable to save string
 * \return          Pointer to output variable
 * \hideinitializer
 */
#define lwgsm_i8_to_str(num, out)         lwgsm_i32_to_gen_str(LWGSM_I32(LWGSM_I8(num)), (out))


/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned char proCheck,checkAgain;
extern unsigned int SlotNo_RS232_1,SlotNo_RS232_2,SlotNo;
extern unsigned char pro_MQTT_Broker_IP[30];
extern unsigned int pro_MQTT_Broker_Port;
extern unsigned char pro_MQTT_Client_ID[30];
extern unsigned short int proTestRequest;
extern uint8_t Lora_RX_Buff[525];        	//LoRA buffer to fill from Rx interrupt
extern lwrb_t lora_rx_rb;  					//LoRA Ring buffer instance for RX data
extern uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA
extern volatile uint8_t sleep_enable;
extern LoraOta_t LoraOTA;
extern uint8_t NoConnNodes, maxConnNode;
extern nodeEntry_t nodesCon[MAX_NODES];

extern unsigned char Pro_Application_flag,pro_DO_DI_TestFinish,UART_OTA_ACKflag;
extern volatile unsigned char UART_OTAflag, AI_CALflag, History_READflag, beforeRTOS;
extern unsigned char pro_RS232_1_state,pro_RS232_2_state,pro_RS485_1_state,pro_RS485_2_state;
extern unsigned char pro_DO_State[26],pro_DI_State[8];
extern unsigned char pro_key1_status,pro_key2_status,pro_Flash_State,pro_I2C_State;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/
uint16_t word(uint8_t H, uint8_t L);
char* lwgsm_u32_to_gen_str(uint32_t num, char* out, uint8_t is_hex, uint8_t padding);
char* lwgsm_i32_to_gen_str(int32_t num, char* out);
void lwgsm_str_to_asciiStr(const char * _in_Str,unsigned int _in_Str_len,const char * _out_asciiStr,unsigned int _in_Str_out);
char HexChartoHexByte(unsigned char a);
unsigned char convertHextoAsciiString(char* i_HexString,char* O_AsciiString,unsigned int _length);
int FindSubstr(char *listPointer, char *itemPointer);
void checkProductionMode();
void setProductionModePara();
int atoi_new(const char* str, int len);
void asciiStringToHexString(char* str, char* hexStr, int len);
void calculateLograteTimeSliceDelayS();
void HexToAscii(const void *value, char *out, uint8_t numBytes);
void AsciiToHex(const char *in, void *out, uint8_t numBytes);
void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);
#endif /* INC_COMMON_H_ */
