#ifndef LPC2300_STRUCTURE_H
#define LPC2300_STRUCTURE_H

struct sSendFrame
{
    char address;
    char info;
    unsigned short int dataSize;
    unsigned short int checkSum;
    unsigned char reply;
    unsigned char reserved;
};

struct sDateTime
{
    unsigned short int	iSec;
    unsigned short int iMin;
    unsigned short int iHour;
    unsigned short int iDay;
    unsigned short int iMonth;
    unsigned short int iYear;
    unsigned short int iDayOfWeek;
};

struct sScalling
{
    float highVal;
    float lowVal;
    unsigned char chnlType;
    unsigned char chnlNo;               // jk changed to unsigned
    unsigned char EMtr;
    unsigned char zeroVal;      // not required, only to match PC struct
};

struct sSetValue
{
    unsigned short int type;
    unsigned short int index;
    unsigned char value[4];
};

struct sHwDbInfo
{
    char AI,DI;
    char maxPid,maxProfile;
    char plcFile,rcpFile,limits,overRide,profile,anaSkip;
    char cardType[8];
};

struct pidStruct
{
    int startFlag,spFlag,outFlag;
    unsigned long timer;
};

struct circbuffst
{
    unsigned short int head;
    unsigned short int tail;
    unsigned short int cnt;
    unsigned short int tail2;
    unsigned short int tail3;
    unsigned short int cnt2;
    unsigned short int cnt3;
    unsigned short int nearfull;
    unsigned short int size;
    unsigned char Adata[BSIZE_E];
    unsigned char Gdata[GSIZE_E];
    unsigned char Ddata[DCU_SERIALBUFFER];
};

struct ch_data
{
    int from,to,type;
    float slope,const1;
};

union floattochar
{
    float tempfloat;
    unsigned int tempint;
    unsigned short int tempshortint[2];
    unsigned char tempchar[4];
};



struct smsindex
{
    unsigned char DD;
    unsigned char MM;
    unsigned char YY;
    unsigned char HH;
};

struct splcDataFile
{
    unsigned short int type;
    unsigned short int in;
    unsigned short int yes;
    unsigned short int no;            	// jk change to unsigned
    unsigned char *str;                     // jk change to unsigned
};

struct timer
{
    char active;
    long start;
    long time;
    long accu;
};
#endif


