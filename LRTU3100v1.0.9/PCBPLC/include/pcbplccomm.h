
#ifndef LPC2300_PCBPLCCOMM_H
#define LPC2300_PCBPLCCOMM_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//#include <RTUCore/MessageBus.h>
//#include <RTUCore/Logger.h>
//#include <RTUCore/SignalHandler.h>
//#include <RTUCore/Directory.h>
//#include <RTUCore/StringEx.h>
//#include <RTUCore/Configuration.h>
//#include <RTUCore/Buffer.h>
//#include <RTUCore/jsonConfig.h>

//extern logger_t* logger;
extern unsigned char program_bit,ProgramMode;			// add by samir for program mode 
extern unsigned char bCommPriority;						// add by samir for bit communication priority.
extern short MsgCnt; 									//new message is startextern unsigned short int curStatus;			// add by samir for current status of communication.
extern unsigned char temparr[50];
extern unsigned char plcallocmemflag;
extern struct Buff ReadMsg;
extern unsigned char pcbplc_logger;
#define LOG_CRITICAL 	1
#define LOG_ERROR 		1
#define LOG_INFO  		1
//extern unsigned short int Max_Plc_var;

extern unsigned short int curStatus;
void RcvHandler(void);
void TransmitHandler(void);
unsigned short int cal_crc(unsigned char *ptr,unsigned short int count);
unsigned short int plc_cal_crc(unsigned short int ptr1,unsigned short int count);
short int  cal_crc1(unsigned char *ptr,short int count);
void MakeBlock(void);
void MakeBlockNew(void);
void putInt(short int intVal, char *buf);
char comin(void);
void SetupCircBuf(void);
void flushccb(void);
void WriteData(char *ch, short int len);
void pack(char *unpack,char *pack,short int no_chls);
void SetDigitalOutput(short int i);
void SetPlcVar(short int i);
void SetHwInfo(void);
short int ProcessCommands(void);
void SetDateTime(void);
void Ecrc(void);
//char checkcrc(void);
void assigndata(void);

void Extract_Alloc(void);

unsigned char set_plc_var(void);
void SendAckPlcVar(void);
extern void WriteLog(uint8_t LogEnable,const char *pData,uint8_t logType);
extern unsigned char transBuf[MAXTRANSBLOCK]; 

extern struct splcDataFile *plcDataFile;
#endif

