#ifndef __PCBPLC_H__
#define __PCBPLC_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/ioctl.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "define.h"

#define PLC_FILE_HEADER_LENGTH      	(89)
#define REC_FILE_ONE_CONTENT_LENGTH 	(33)

#define RECIPE_VAR_START_ARRAY_INDEX	(2435)
#define RECIPE_VAR_START_MODBUS_INDEX	(4871)
#define RECIPE_VAR_START_PCBPLC_INDEX	(6171)

#define RECIPE_VAR_END_ARRAY_INDEX		(2934)
#define RECIPE_VAR_END_MODBUS_INDEX		(5869)
#define RECIPE_VAR_END_PCBPLC_INDEX		(7169)

#define MODIFIED_RECIPE_FILE_PATH   	"M.BIN"
#define RECIPE_JSON_FILE_PATH       	"recipe.json"
#define CONFIG_FILE_PATH            	"/etc/service_pcbplc.conf"
#define ANALOG_JSON_FILE_PATH       	"debug.json"

#define TWO 							2
#define ONE 							1

#define	MAX_PLCSTRING       			100
#define MAX_PLCVAR          			(500)

#define MAX_RS485_PARA      			(300)

#define OPERATION_BOX					0xffff
#define CONDITION_BOX					0

#define MAX_CLOCKS      				(300)
#define PCBPLC_CLK_TCK   			  	(1)

#define GSM_MSG_SIZE 					240

#define MAX_AI 							(128)
#define MAX_DATA_TAG    				(128)

/** Maximum number of General purpose Analog parameters */
#define	MAX_GEN_ANA_PARA                (300)

/** Start index of General purpose Analog parameters Array - gFinalAnaValF */
#define START_IDX_GEN_ANA_PARA_TAG      (700)
/** Start Modbus address of general purpose analog array */
#define	START_IDX_GEN_ANA_PARA		(161)

/** Start index of Modbus parameters Array - gFinalAnaValF */
#define START_IDX_MODBUS_PARA_TAG       (300)
/** Start Modbus address of Modbus query*/
#define	START_IDX_MODBUS_ANA_PARA		(161)

typedef struct
{
	unsigned int sf_client_id;  					//4
	unsigned int sf_reader_id;  					//4+4=8
	unsigned char sf_Date;							//8+1=9
	unsigned char sf_Month;							//9+1=10
	unsigned char sf_Year;						//10+1=11
	unsigned char sf_Hour;							//11+1=12
	unsigned char sf_Min;							//12+1=13
	unsigned char sf_Sec;							//13+1=14
	unsigned char sf_NO_DI_LG;						//14+1=15
	unsigned char sf_NoofSMSTag;					//15+1=16
	unsigned int sf_Address; 						//16+4=20       //RTUD_ID address of this RTU stored in Address
	unsigned char sf_DI[2]; 						//20+2=22				//16 DI chennal in 2 byte
	float sf_AnalogValue[120];						//22+(120*4=480)=502
	unsigned char unused[6];
}flashDataSturct;

typedef struct
{
	unsigned short int s_ExtDataFlash_CheckByte;  							//	2
	unsigned short int s_ExtDataFlash_IsDataLogOverwritten;  		//	2
	unsigned int s_ExtDataFlash_PageCounter;  									//	4
	unsigned char s_temp_Counter;
	unsigned char unused[503];
}flashRunTimeParaSturct;

extern float gFinalAnaValF[MODMAX_PARA];
extern char dig_bit_array1[MAX_DI];
extern char dig_bit_array[MAX_DI];

extern short int seqStartNode,emrStartNode,intStartNode;
extern short int seqStep,emrStep,intStep;
extern short unsigned int totalBits, totalFloats;
extern short int *int_arr;
extern unsigned char overRideFlagArr[MAX_DIG_CHNS];
extern char plcFileRec;
extern unsigned char bAllDownload;
extern char firstTime;
extern unsigned short int memIndex;

extern unsigned char plcVarArrType[MAX_PLCVAR];
extern short int plcUserArr[];
extern float plcVarArr[MAX_PLCVAR];
extern float plcstringArr[MAX_PLCSTRING];
extern float signal_arr[MAX_GEN_ANA_PARA];
extern unsigned char flagWriteQueryAvailabe;
extern unsigned char PlcFlag;
extern unsigned short int maxAna,maxDig;
extern unsigned int outputStatus[MAX_DIG_CHNS];
extern unsigned char alarmToggleFlag,plcDisplayFlag;

extern char gPlcFile;
extern char gRecFile;

void pcbplc_BaseLoop();
short int pcbplc_process(void);
short int pcbplc_emr_seq(void);
int PcbPlc_FileReadAndParse(unsigned char pcbplc_logger, unsigned char pPlcFile, unsigned char pRecFile);

int ReadRecipeFile();
int WriteModifiedRecipeFile(const char *pFile);

float PcbPlc_Logic_Interpreter(unsigned char str[],short int execute_flag);
void pcbplc_memory_free();

short int PcbPlc_Confirm(void);
short int PcbPlc_SetPlcTimer(void);
short int PcbPlc_ResetPlcTimer(void);
short int PcbPlc_InitPlcTimer(void);
short int PcbPlc_HoldPlcTimer(void);
short int PcbPlc_ReleasePlcTimer(void);
short int PcbPlc_InitAllPlcTimers(void);
short int PcbPlc_HoldAllPlcTimers(void);
short int PcbPlc_ReleaseAllPlcTimers(void);
short int PcbPlc_ResetCycle(void);
short int PcbPlc_DummyHandler(void);
short int PcbPlc_SendAlarm(void);
short int PcbPlc_ChkSchedule(void);
short int PcbPlc_GetTrans(void);
short int PcbPlc_SetTrans(void);
short int PcbPlc_GetPlcTimer(void);
short int PcbPlc_ClearWaitState(void);
short int PcbPlc_GetHour(void);
short int PcbPlc_GetMinute(void);
short int PcbPlc_GetSecond(void);
short int PcbPlc_GetDay(void);
short int PcbPlc_GetMonth(void);
short int PcbPlc_GetYear(void);
short int PcbPlc_MakeTime(void);
short int PcbPlc_MakeDate(void);
short int PcbPlc_InPortB(void);
short int PcbPlc_OutPortB(void);
short int PcbPlc_UserGetBit(void);
short int PcbPlc_UserSetBit(void);
short int PcbPlc_UserClrBit(void);
short int PcbPlc_SetHiTimer(void);
short int PcbPlc_LoadMimicFile(void);
short int PcbPlc_PlaySoundFile(void);
short int PcbPlc_Plcabsolute(void);
short int PcbPlc_PlcCos(void);
short int PcbPlc_PlcSine(void);
short int PcbPlc_PlcTan(void);
short int PcbPlc_PlcAsine(void);
short int PcbPlc_PlcAcos(void);
short int PcbPlc_PlcAtan(void);
short int PcbPlc_PlcPower(void);
short int PcbPlc_PlcSqrt(void);
short int PcbPlc_PlcLog(void);
short int PcbPlc_PlcLog10(void);
short int PcbPlc_stringlength(void);
short int PcbPlc_Isnumeric(void);
short int PcbPlc_texttoval(void);
short int PcbPlc_assignstring(void);
short int PcbPlc_strmid(void);
short int PcbPlc_strinstring(void);
short int PcbPlc_stringsplit(void);
short int PcbPlc_BuildSms(void);
short int PcbPlc_Analog_output_Plc(void);
short int PcbPlc_DO_Key_Status(void);
short int PcbPlc_Get_sch_time(void);
short int PcbPlc_GetWeekDay(void);
short int PcbPlc_SetPValue(void);
short int PcbPlc_PlcDisplay(void);
short int PcbPlc_ClearPLCDisplay(void);
short int PcbPlc_PlcAlarmDisplay(void);        
short int PcbPlc_CheckAutoManual(void);
short int PcbPlc_SetAutoMode(void);
short int PcbPlc_SetManualMode(void);
short int PcbPlc_SendDataInSMS(void);
short int PcbPlc_SendData();
short int PcbPlc_SetDO(void);
short int PcbPlc_SendAlarmSMS(void);

short int mem_alloc(void);
short int getBit(unsigned short int);
void clrBit(unsigned short int);
void setBit(unsigned short int);

int ReadRecipeJsonFile();
int RecipeToJsonConversion();

void putInStack(short int execute_flag,short int stkPtr,short int offset);
void getStackVar(short int stkPtr,short int offset);
short int GetInt(unsigned char ch1,unsigned char ch2);
void makeAccessTable(void);
void *PlcAlloc(unsigned char pcbplc_logger, short int size);
void InitGlobalVar(void);
int Build_Data(char *pDataBuf);
int Build_Data_for_server();
int8_t LoraPublish();
void pcbplc_start();
unsigned long int clock_pcbplc(void);
void ExtFlash_WriteHistoricalData(flashDataSturct tflashDataSturct);
void MemMove(void);

extern unsigned long int plcTimerTicks;
extern unsigned char outputDOflag[],OldoutputDOflag[];
extern unsigned char PLC_RPOG_Flag;
extern unsigned char REC_RPOG_Flag;
extern short int maxSeqNode,maxEmrNode,maxIntNode,maxNodes;


#endif
