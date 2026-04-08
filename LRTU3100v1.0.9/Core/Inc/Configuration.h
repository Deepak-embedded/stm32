/*
 * Configuration.h
 *
 *  Created on: Jan 9, 2023
 *      Author: Shreyanss
 */

#ifndef INC_CONFIGURATION_H_
#define INC_CONFIGURATION_H_

/**************************************************************************//**
 * define
 *****************************************************************************/

#define DEFAULT_HW_VERSION					"1.0.0"
#define DEFAULT_FV_VERSION					"1.0.14"
#define DEFAULT_PLC_VERSION					"x.x.x"
#define DEFAULT_REC_VERSION					"x.x.x"
#define DEFAULT_LOGRATE						10
#define DEFAULT_MAX_LOGRATE_TIME_SLICE_DELAY_S	900
#define DEFAULT_RTUID						1
#define DEFAULT_SLOTID 						0

#define DEFAULT_CLIENTID					1
#define DEFAULT_READERID					1
#define DEFAULT_PROJECT_CODE  				"xyz"
#define DEFAULT_SITE_NAME	  				"xyz"
#define DEFAULT_TIME_ZONE	  				"+5:30"
#define DEFAULT_TIME_ZONE_SIGN				0
#define DEFAULT_TIME_ZONE_HOURS				5
#define DEFAULT_TIME_ZONE_MINUTES			30
#define DEFAULT_DAY_NIGHT_REBOOT			0
#define DEFAULT_LAT							23.04981//23.4511
#define DEFAULT_LOGITUDE					72.50129//72.5689

#define DEFAULT_DI							8	//16
#define DEFAULT_DO							22	//26
#define DEFAULT_AI							5
#define DEFAULT_AI_CH_TYPE					1  //1
#define DEFAULT_SCALE_LO					0//4
#define DEFAULT_SCALE_HI					16//20
#define DEFAULT_CALZ						0//4
#define DEFAULT_CALS						16//20

#define DEFAULT_MAILOWCAL_MA				1112	//1108
#define DEFAULT_MAILOWMIDCAL_MA				13544	//13503
#define DEFAULT_MAIMIDCAL_MA				32247	//32254
#define DEFAULT_MAIHIGHCAL_MA				63431	//63531


#define DEFAULT_MAILOWCAL_MA_POINT			0
#define DEFAULT_MAILOWMIDCAL_MA_POINT		4
#define DEFAULT_MAIMIDCAL_MA_POINT			10
#define DEFAULT_MAIHIGHCAL_MA_POINT			20

#define DEFAULT_MAILOWCAL_V					1126	//1135
#define DEFAULT_MAILOWMIDCAL_V				4166	//4250
#define DEFAULT_MAIMIDCAL_V					32996	//33000
#define DEFAULT_MAIHIGHCAL_V				65059	//64526

#define DEFAULT_MAILOWCAL_V_POINT			0
#define DEFAULT_MAILOWMIDCAL_V_POINT		1.5
#define DEFAULT_MAIMIDCAL_V_POINT			15		//5
#define DEFAULT_MAIHIGHCAL_V_POINT			30		//10

#define DEFAULT_RS232_1_ENABLE				1
#define DEFAULT_RS232_1_PROTCOL				1
#define DEFAULT_RS232_1_MASTER_SLAVE		3
#define DEFAULT_RS232_1_BAUDRATE			9600
#define DEFAULT_RS232_1_PORT_ID				COM_RS232_1
#define DEFAULT_RS232_1_POLL_FRQ			100 //ms

#define DEFAULT_MODEM_ENABLE 					1
#define DEFAULT_MODEM_SERIAL_USB 				0
#define DEFAULT_MODEM_PROTOCOL 					0
#define DEFAULT_MODEM_MQTT_BROKER_IP 			"203.88.128.141"
#define DEFAULT_MODEM_MQTT_BROKER_PORT 			1884
#define DEFAULT_MQTT_COMM_MODE					0
#define DEFAULT_MODEM_MQTT_USR_NAME 			"cimcon"
#define DEFAULT_MODEM_MQTT_USR_PASS 			"cimcon"
#define DEFAULT_MODEM_MQTT_CLIENTID 			"cimcon12345"
#define DEFAULT_MODEM_MQTT_PUB_TOPIC 			"12345678"
#define DEFAULT_MODEM_MQTT_SUB_TOPIC 			"12345678"
#define DEFAULT_MODEM_APN 						"www"
#define DEFAULT_MODEM_MQTT_LIVE_FRQ				5  //sec

#define DEFAULT_GPS_ENABLE 						1
#define DEFAULT_GPS_POLL_FRQ 					2 //min
#define DEFAULT_SD_ENABLE 						0
#define	DEFAULT_SD_SIZE 						0
#define DEFAULT_DO_MODE 						2
#define DEFAULT_DO_OFF 							0

#define DEFAULT_SCHEDULE_TOTAL_NO 				16
#define DEFAULT_SCHEDULE_ENABLE 				0
#define DEFAULT_SCHEDULE_START_HH 				0
#define DEFAULT_SCHEDULE_START_MIN 				0
#define DEFAULT_SCHEDULE_STOP_HH				0
#define DEFAULT_SCHEDULE_STOP_MIN				0


#define DEFAULT_LORA_DUTY_CYCLE_ENABLE			1
#define DEFAULT_LORA_BAUDRATE					115200
#define DEFAULT_RETRY_COUNT						3			// number of retries
#define DEFAULT_RETRY_DELAY 					35  		// seconds
#define DEFAULT_LOG_SEND_DELAY					15			// seconds
#define DEFAULT_READ_CMD_DELAY					2			// seconds
#define OTA_TIMEOUT								7			// seconds

#define DEFAULT_MODBUS_QUARY_DETAIL_TOTAL_NO  			0
#define DEFAULT_MODBUS_QUARY_DETAIL_RETRY_COUNT  		0
#define DEFAULT_MODBUS_QUARY_DETAIL_TOTAL_PARA_NO		0
#define DEFAULT_MAX_DATA_TAG							17
#define MAX_AI_CHANNEL   								5


/**************************************************************************//**
 * enum
 *****************************************************************************/

// ********************** configuration command Name (CMDDtate)****************************
typedef enum
{
	RTU_INFO = 1,
	CUST_INFO,
	AI_DI_DO_INFO,
	SERIAL_INFO,
	CELL_MODEM_INFO,
	GPS_INFO,
	SD_CARD_INFO,
	RTC_INFO,
	HISTORY_LOGRATE_INFO,
	DO_MODE_INFO,
	SCHEDULE_INFO,
	MODBUS_INFO,
	LORA_INFO,
} Configuration_command;

typedef enum
{
	WRITE_CMD = 1,
	READ_CMD  = 2,
} Read_write;

// ********************** RTU Details ****************************
struct RtuDetails //Command 1 for RTU basic Details
{
	int  RTUId ;
	char HW_Version[30];
	char PLC_Version[30];
	char REC_Version[30];
	char Hex_Version[30];
};

// ********************** Customer Details ***********************
struct CustDetails //Command 2 for Customer basic Details
{
	char Proj_Code[30];
	char Site_Name[30];
	char Time_zone[100];

	int  Client_Id;
	int  Reader_Id;

	double Lattitude;
	double Longitude;

	char reboot_day_night;
	char Tz_sign;
	char Tz_hours;
	char Tz_min;
};

// ********************** AI/DI/DO  Details ***********************
typedef enum
{
	MA4TO20 = 1,
	V0TO20 = 2,
//	V0TO10 = 2
}AI_chanel_types;

struct PulseDIDetail
{
	unsigned int Pulse_DI_count;
	float Pulse_DI_Freq;
	float Pulse_DI_Const;
	float Pulse_DI_Flow_Configured;
	float Pulse_DI_Flow_Calculated;
};

struct PulseDODetail
{
	unsigned char Pulse_DO_type;
	unsigned char Pulse_DO_polarity;
	unsigned char Pulse_DO_Width;
	unsigned char Pulse_DO_Count;
	unsigned char Pulse_DO_Width_scale;
};
struct AIDetails
{
	int Id;

	AI_chanel_types AI_ch_Type;
	float scaleLo;
	float scaleHi;
	float calZ;
	float calS;
	unsigned short int mAiHighCal_mA;  // = 0x0000;
	unsigned short int mAiMidCal_mA;  // = 0x7FFF;
	unsigned short int mAiLowMidCal_mA;   // = 0x0C80;
	unsigned short int mAiLowCal_mA; // = 0xFFFF;

	float mAiHighCal_mA_Point;  // = 0x0000;
	float mAiMidCal_mA_Point;  // = 0x7FFF;
	float mAiLowMidCal_mA_Point;   // = 0x0C80;
	float mAiLowCal_mA_Point; // = 0xFFFF;

	unsigned short int mAiHighCal_V;  // = 0x0000;
	unsigned short int mAiMidCal_V;  // = 0x7FFF;
	unsigned short int mAiLowMidCal_V;   // = 0x0C80;
	unsigned short int mAiLowCal_V; // = 0xFFFF;

	float mAiHighCal_V_Point;  // = 0x0000;
	float mAiMidCal_V_Point;  // = 0x7FFF;
	float mAiLowMidCal_V_Point;   // = 0x0C80;
	float mAiLowCal_V_Point; // = 0xFFFF;
};

struct AIDIDODetails
{
	int Total_Di;
	int Total_Do;
	int Total_Ai;
	struct AIDetails AI_Detail[MAX_AI_CHANNEL];
	unsigned char Sample_time_to_collect_AI;
};

struct PulseDODIDetails
{
	int Total_Pulse_Di;
	int Total_Pulse_Do;
	struct PulseDIDetail Pulse_DI_Detail[16];
	struct PulseDODetail Pulse_DO_Detail[DEFAULT_DO];
};
// **********************Communication  Details ***********************
typedef enum
{
	CONF_DISABLE = 0,
	CONF_ENABLE  = 1,
} Comm_En_Dis;

// **********************Serial Communication  Details ***********************
typedef enum
{
	S_MODBUS_RTU   = 1,
	S_MODBUS_ASCII = 2,
	S_SER_DEB      = 3,
} SComm_Proto;

struct Serial_Communication
{
	Comm_En_Dis S_Co_En_Di;
	SComm_Proto S_Protocol;
	mb_masterslave_t S_Ma_sl_Cu;
	unsigned int S_Baudrate;
	unsigned char S_Port_Id;
	int S_Poll_Freq;
};

struct SCommunication
{
	struct Serial_Communication Rs232_1_Info;
};

// ********************** cell Modem Communication Details ***********************
typedef enum
{
	MO_SERIAL = 0,
	MO_USB    = 1,
} MO_Comm_Interface;

typedef enum
{
	MO_MQTT   = 0,
	MO_UDP    = 1,
	MO_DNP3   = 2,
	MO_MQTTS  = 3,
	MO_CUSTOM = 4,
} MO_Comm_Protocol;

struct MQTT_Connection
{
	char MQTT_Bro_IP[20];
	char MQTT_Us_Name[20];
	char MQTT_Us_Pass[20];
	char MQTT_PUB_Topic[50];
	char MQTT_Sub_Topic[50];
	char MQTT_Cli_Id[20];
	int  MQTT_Bro_Port;
	char MQTT_Comm_Mode;
};

struct ModemCommunication
{
	Comm_En_Dis Mo_Co_En_Di;
	MO_Comm_Interface Mo_Com_Int;
	MO_Comm_Protocol Mo_Proto;
	struct MQTT_Connection MQTT_Conn;
	char Mo_APN[30];
	int MQTT_LiveFreq;
};

// ********************** BLE/GPS/SD card /Do mode Details ***********************
//typedef enum
//{
//	DO_AUTO = 0,
//	DO_NOT_APPLICABLE,
//	DO_MANUAL,
//} DO_Mode_AU_MA;


struct GPS_Details
{
	Comm_En_Dis GPS_Co_En_Di;
	int GPS_Poll_Freq;
};

struct SD_Card_Details
{
	Comm_En_Dis SD_Card_Co_En_Di;
	int SD_Card_Size;
};

struct DO_Mode_Details
{
	RtuDoMode_e Do_Mode;
	char DO_Value[35];
};

// **********************Schedule Configuration ***********************
struct Schedule_Data
{
	Comm_En_Dis Sch_En_Di;
	char Sch_Id;
	char Start_HH;
	char Start_Min;
	char Stop_HH;
	char Stop_Min;
};

// **********************Modbus Quary Configuration ***********************
typedef enum
{
	MOD_COIL_ST   = 0,
	MOD_INPUT_ST  = 1,
	MOD_HOLD_REG  = 2,
	MOD_INPUT_REG = 3,
} Mod_W_Types;

struct Modbus_Quary
{
	mb_functioncode_t mFunctionCode;  // Todo : sync Function code with prakshbhai,Harsh,Code logic,,modbus lib
	uint8_t mDataType;	// Todo : sync dataType with prakshbhai,Harsh,Code logic,,modbus lib
	char Mod_Quary_ID;
	int mPortSelection;
	unsigned char mSlaveId;
	int mRegStartAddr;
	unsigned char mNoOfRegister;
};

struct Modbus_WriteQuary
{
	Mod_W_Types mFunctionCode_write;  // Todo : sync Function code with prakshbhai,Harsh,Code logic,,modbus lib
	uint8_t mDataType_write;	// Todo : sync dataType with prakshbhai,Harsh,Code logic,,modbus lib
	char mWriteQueryNumber;
	int mPortSelection_write;
	unsigned char mSlaveId_write;
	int mRegStartAddr_write;
	unsigned char mNoOfRegister_write;
	unsigned int mValue_write; // for Modbus write to increase data value to 65535 || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-31
	unsigned char mWriteQueryAck;
};

// **********************RTV time and Historical and LogRate Configuration ***********************

struct Configuration
{
	char  CMD_State;
	char  CMD_Type;
	char  Res_Ack;
	char  RTC_D_T[30];
};

/**************************************************************************//**
 * struct
 *****************************************************************************/
struct Save_Para_Frequent
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	unsigned int DI1_Pulse;
	unsigned int DI2_Pulse;
	unsigned char Pulse_DI_Interrupt_Type;
	unsigned char Pulse_DI_frequency_Method;
	unsigned short int Pulse_DI_frequency_time;
};

struct Save_Para_General
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//4+20=24
	uint8_t slot_id;//2+2=4
	unsigned char LoraRetryCount;
	unsigned char LogSendDelay;
	unsigned char LoraRetryDelay;
	unsigned char readCmdDelay;
	unsigned int rebootCount;
	unsigned int LogRate;
	Comm_En_Dis History_En_Di;
	struct RtuDetails Rtu_Detail;
	struct CustDetails Cust_Detail;
	struct AIDIDODetails AI_DI_DO_Detail;
	struct SCommunication S_Comm;
	struct ModemCommunication Mo_Comm;
	struct GPS_Details gpsDetails;
	struct SD_Card_Details sdCardDetails;
	struct DO_Mode_Details DoModeDetails;
	unsigned char DeviceID[30];
	unsigned char pro_CheckByte;
	struct PulseDODIDetails Pulse_DO_DI_Detail;

	unsigned int Lora_Frequency ;
	uint8_t Lora_Spreading_Factor;
	uint8_t Lora_Bandwidth;
	uint8_t Lora_Code_Rate;
	uint16_t Lora_Preamble_Length;
	uint8_t Lora_TX_Power ;
	uint8_t Modem_EC200_presence;
	uint8_t LOta_MsgInterval;
};

struct Save_Para_AI_Calibration
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	struct AIDetails AI_Detail[MAX_AI_CHANNEL];
	unsigned char DeviceID[30];
};
struct Save_Para_Schedule_Configuration
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	int Total_No_Schedule;
	struct Schedule_Data Schedule[85];
};

struct Save_Para_Modbus_Configuration
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	char TotalQuery;
	char RetryCount;
	int TotalPara;
	unsigned int mMaxDataTagEnabled;
	struct Modbus_Quary Mod_Quary[100];
};

struct Save_Para_PCBPLC_General_Reg
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;
	float General_Reg[1000];
};

typedef struct {
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	unsigned int rebootCount;
} EPROM_Dync_t;

extern EPROM_Dync_t EPROM_Dync;

/**************************************************************************//**
 * extern
 *****************************************************************************/

extern struct Save_Para_General EPROM_General;
extern struct Save_Para_AI_Calibration EPROM_AI_Calibration;
extern struct Save_Para_Schedule_Configuration EPROM_Schedule;
extern struct Save_Para_Modbus_Configuration EPROM_Modbus_Quary_Detail;
extern struct Save_Para_PCBPLC_General_Reg EPROM_PCBPLC_General_Reg;
extern struct Modbus_WriteQuary MODBUS_Write[1];
extern struct Configuration Config;
extern struct OTA_hex_Data OTA_store_Data;
extern struct Save_Para_Frequent EPROM_Frequent;
extern unsigned char mWriteQueryAck[MAX_WRITE_QUERY_ACK];

void ExtFlash_update_EPROM_General();
void ExtFlash_update_EPROM_Schedule();
void ExtFlash_update_EPROM_Dync();
void ExtFlash_update_EPROM_Modbus_Quary_Detail();
void ExtFlash_Read_EPROM_General(unsigned char makeDefault);
void ExtFlash_Read_EPROM_Schedule(unsigned char makeDefault);
void ExtFlash_Read_EPROM_Dync(unsigned char makeDefault);
void ExtFlash_Read_EPROM_Modbus_Quary_Detail(unsigned char makeDefault);
void ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();
void ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(unsigned char makeDefault);
void ExtFlash_Read_gPlcRecFlash(unsigned char makeDefault);
void ExtFlash_update_gPlcRecFlash();
void ExtFlash_Read_OTA_Data();
void ExtFlash_update_OTA_Data();
void ExtFlash_Read_EPROM_AI_Calibration(unsigned char makeDefault);
void ExtFlash_Read_EPROM_Frequent(unsigned char makeDefault);
void ExtFlash_update_EPROM_Frequent();
void ExtFlash_update_EPROM_AI_Calibration();
void syncExtFlashVariableWithPCBPLCVariable();
JSON_ERROR_RESPONSE Response_Config_Read_JSON_frame(COM_TYPE com_mode , CMD_TYPE CMD, char cmdState, char file_CMD_type, RES_ACK iACK, char * ACK_Response);

#endif /* INC_CONFIGURATION_H_ */
