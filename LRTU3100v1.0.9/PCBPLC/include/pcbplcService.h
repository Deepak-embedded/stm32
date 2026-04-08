#ifndef RS485_SERVICE
#define RS485_SERVICE

#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#include  "pcbplc.h"

#if !defined(bool)
	#define bool int
	#define false 	0
	#define true 	1
#endif

#if !defined(NULL)
#define NULL 0
#endif

#define MAXSCH  		(84)
#define MAXALARM 		(10)

typedef enum
{
    RTU_DO_MODE_MANUAL          =   0,
    RTU_DO_MODE_PHOTO           =   1,
    RTU_DO_MODE_AUTO            =   2,
    RTU_DO_MODE_ASTROTIME_GEO   =   3,
    RTU_DO_MODE_TWILIGHT_GEO    =   4,
    RTU_DO_MODE_LOCAL           =   5
}RtuDoMode_e;

typedef struct Schedule
{
    unsigned char mIsSchEnabled;
    unsigned char mStartHour;
    unsigned char mStartMin;
    unsigned char mStopHour;
    unsigned char mStopMin;
}Schedule_t;

typedef struct smsAlarm
{
    unsigned char mType; //1-Alarm, 0 - Disable, 2 - Data
    unsigned char mobileNo[15];
}smsAlarm_t;

typedef struct sdkConfig
{
    char mDeviceName[256];
    char mParaName[256];
}sdkConfig_t;

typedef struct pcbplcCnfg
{
    char debug;
    char mIsEnable;
    unsigned int mClientAddr;
    unsigned int mGroupAddr;
    unsigned int mRtuAddr;
    unsigned char mRtuDoMode;
    unsigned int mMaxSchEnabled;
    Schedule_t sSch[MAXSCH];
    unsigned int mMaxAlarmEnabled;
    unsigned char mSiteName[128];
    unsigned int mLogRate; //in minutes
    smsAlarm_t mAlarm[MAXALARM];
    unsigned int mMaxDoEnabled; /* Valid value : 1 to 960 */ /* MAXDO */
    unsigned int mMaxDiEnabled; /* Valid value : 1 to 960 */ /* MAX_DI */
    unsigned int mMaxAiEnabled; /* Valid value : 1 to 128 */ /* MAX_AI */
    unsigned int mMaxAoEnabled; /* Valid value : 1 to 32 */ /* MAX_AO */
    unsigned char mDoStatus[MAXDO];
    unsigned char mOldDoStatus[MAXDO];
    double mtz;
    int mtimeoffset; /* This variable should be signed */
    unsigned int mMaxQuery;
    unsigned int mMaxDataTagEnabled; /* valid value 1 to 128 */
    unsigned int mDataTag[MAX_DATA_TAG];
    //unsigned char mScanRate;
    unsigned char mNumOfDest;
    sdkConfig_t *modbusConfig;
    unsigned char mNumOfModbusConfig;
}pcbplcCnfg_t;

typedef struct plcRecFlashInfo
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	unsigned int extract_receipe; // if new REC file recive than set this flash to one so on PLCPLC extrac rec variavle updated in modified REC Variable
    unsigned long int mPlcFileLength;
    unsigned long int mRecFileLength;
    unsigned long int mPlcFileCRC;
    unsigned long int mRecFileCRC;
    float plcVarArr[MAX_PLCVAR];  // add maulin for modified REC file
    unsigned char mPlcVarTypeArr[MAX_PLCVAR]; // add maulin for modified REC file
}plcRecFlashInfo_t;

typedef struct pcbplcInfo
{
    char mPlcFileError;
    char mRecFileError;
    unsigned long int mPlcFileLength;
    unsigned long int mRecFileLength;
    unsigned long int mMaxRecipeVar;
    //float mPlcVarArr[MAX_PLCVAR];
    char mPlcVarArrName[MAX_PLCVAR][16];
    float mPlcVarArr_1[MAX_PLCVAR];
    float mPlcVarArr_2[MAX_PLCVAR];
    float mPlcVarArr_3[MAX_PLCVAR];
    float mPlcVarArr_4[MAX_PLCVAR];
    float mPlcVarArr_5[MAX_PLCVAR];
    unsigned char mPlcVarTypeArr[MAX_PLCVAR];
    //unsigned char mReloadCnfg;
    //unsigned char mReloadReceipe;
}PcbplcInfo_t;

typedef struct timeInfo
{
    unsigned char mHour;    /* Hours (0-23) */
    unsigned char mMinute;  /* Minutes (0-59) */
    unsigned char mSecond;  /* Seconds (0-60) */
    unsigned char mDate;    /* Day of the month (1-31) */
    unsigned char mMonth;   /* Month (1-12) */
    unsigned int  mYear;     /* 1900 onwards */
    unsigned char mDayofWeek; /* Day of the week (0-6, Sunday = 0) */
}TimeInfo_t;

bool service_initialize(const char* config_file);
bool service_destroy();
bool service_start();
bool service_restart();
bool service_stop();

void read_rtu_datetime();
void timer_thread_start();
void send_data_string(char *pDataBuf);
void send_event_string(char *pDataBuf);
int IsLogRateMatched();
int update_do_status_key(int pin_no, int pin_value);
unsigned char checkFileAvibility(unsigned char file);

extern TimeInfo_t      gTimeInfo;
extern pcbplcCnfg_t    gpcbplcCnfg;
extern PcbplcInfo_t    gPcbplcInfo;
extern plcRecFlashInfo_t gPlcRecFlash;
extern char** destination_list;
#endif
