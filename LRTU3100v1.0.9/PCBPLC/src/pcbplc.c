/**********************************************************************
        RTU PROGRAM Project : 12-3-08
        AUTHOR : samir B. Malvi
        File name : Comm.c
        Description : This file is having communication related routines.
**********************************************************************/
#include "main.h"
#include "define.h"
#include "structure.h"
#include "pcbplc.h"
#include "pcbplccomm.h"
#include "pcbplcService.h"
#include "pcbplcInterface.h"
#include "stdlib.h"

union Flt
{
    float tempfloat;
    unsigned int tempint;
    unsigned short int tempshortint[2];
    unsigned char tempchar[4];
}fpara;

union DataFreg
{
    float fl;
    short sh[2];
    unsigned char ch[4];
}Dat;

PcbplcInfo_t    gPcbplcInfo;
plcRecFlashInfo_t gPlcRecFlash;

flashRunTimeParaSturct g_flashRunTimeParaSturct;

struct timer clocks[(MAX_CLOCKS*3)+1] = {0,} ;
TimeInfo_t gTimeInfo = {0,};

unsigned char buffer[BUFFER_SIZE];
unsigned char flagWriteQueryAvailabe = 0;
unsigned char stringbuff[50][50];
float plcStack[300];
float paramStack[300];
unsigned short int Rcp_var_index = 0;
unsigned char Plc_type = 0;
unsigned char  plc_var_name[20] = {0, };
float Plc_var_1,Plc_var_2,Plc_var_3,Plc_var_4,Plc_var_5;
char csStrList[8][GSM_MSG_SIZE];
unsigned char PLC_RPOG_Flag = 0;
unsigned char REC_RPOG_Flag = 0;
unsigned short int RECindex[MAX_PLCVAR];

extern unsigned char gStopHistoricalDataStoreCounter;
extern unsigned char gStopHistoricalDataStore;

extern uint8_t pcbplcfile[12*1024];
extern uint8_t RecFile[1*1024];

unsigned char temparr[50];
unsigned char program_bit=0,ProgramMode = 0;//Temp1;
unsigned char bCommPriority;
unsigned short int curStatus;

unsigned char outputDOflag[MAX_DIG_CHNS],OldoutputDOflag[MAX_DIG_CHNS]; /* Write this buffer to fire DO output */
float gFinalAnaValF[MODMAX_PARA];
float signal_arr[MAX_GEN_ANA_PARA]; /* For General purpose use, Analog (float) array */

float plcVarArr[MAX_PLCVAR];
float plcstringArr[MAX_PLCSTRING];

char dig_bit_array[MAX_DI];
char dig_bit_array1[MAX_DI];

float gAoValue[MAX_AO];

unsigned char plcallocmemflag = 0;
short int maxSeqNode,maxEmrNode,maxIntNode,maxNodes;
short int curStep,emrLastStep;
short int executing,waitingFlag,maxPlcVar;
short int paramSp,stackIndex;
short unsigned int totalPid;

unsigned char *falseInterlock;
unsigned int outputStatus[MAX_DIG_CHNS];
unsigned char outBits[MAX_DIG_CHNS];
unsigned char accessTable[MAX_DIG_CHNS];
unsigned char feedbackArr[MAX_DIG_CHNS];
unsigned char waitingFor[MAX_DIG_CHNS];
unsigned char modifiedArr[MAX_DIG_CHNS];
unsigned char conflictArr[MAX_DIG_CHNS];
short int transFlagMode;
unsigned char string[200],string1[200];
static unsigned char gsLastCommand = 0;
char bPCBFlag=0;

unsigned char str[500];
char AnaScan[8];
unsigned char alarmToggleFlag = 0,plcDisplayFlag = 0;
unsigned char str_flag = 0; 
unsigned char reset_flag = 0;
unsigned char ucB[500];

char gPlcFile = 1;
char gRecFile = 2;

extern int makeConflictTable(void);

static void AnaOutLogic(short int index, float val);
void ControlLogic(unsigned int Mod_Address, float Mod_Value);

extern int gPlcFileLength;
extern int gRecFileLength;

void GetStringArryRec(unsigned char *pFile,unsigned long address,unsigned char *Arr,unsigned short len);
float Readfloatrec(unsigned char *pFile,unsigned long address);
unsigned char ReadByterec(unsigned char *pFile,unsigned long address);
short int ReadShortrec(unsigned char *pFile,unsigned long address);
float ConvertFloatRec(unsigned char *bytes);

extern TimerHandle_t sendTimer, readCmdTimer, ackTimer;

short int (*FnPtr[])(void) =
{
        PcbPlc_Confirm,                     /* 0 */
        PcbPlc_PlcDisplay,                  /* * 1 - DISPLAY */
        PcbPlc_ResetPlcTimer,               /* 2 - TIMER */
        PcbPlc_SetPlcTimer,                 /* 3 - TIMER*/
        PcbPlc_InitPlcTimer,                /* 4 - TIMER*/
        PcbPlc_HoldPlcTimer,                /* 5 - TIMER*/
        PcbPlc_ReleasePlcTimer,             /* 6 - TIMER*/
        PcbPlc_InitAllPlcTimers,            /* 7 - TIMER*/
        PcbPlc_ResetCycle,                  /* 8 */
        PcbPlc_GetTrans,                    /* 9 */
        PcbPlc_SetTrans,                    /* 10 */
        PcbPlc_Plcabsolute,                 /* 11 */
        PcbPlc_GetPlcTimer,                 /* 12 - TIMER */
        PcbPlc_PlcAlarmDisplay,             /* 13 - DISPLAY  */
        PcbPlc_ClearWaitState,              /* 14 */
        PcbPlc_HoldAllPlcTimers,            /* 15 - TIMER */
        PcbPlc_ReleaseAllPlcTimers,         /* 16 - TIMER */
        PcbPlc_GetHour,                     /* 17 - RTC */
        PcbPlc_GetMinute,                   /* 18 - RTC */
        PcbPlc_GetSecond,                   /* 19 - RTC */
        PcbPlc_GetDay,                      /* 20 - RTC */
        PcbPlc_GetMonth,                    /* 21 - RTC */
        PcbPlc_GetYear,                     /* 22 - RTC */
        PcbPlc_MakeTime,                    /* 23 - RTC */
        PcbPlc_MakeDate,                    /* 24 - RTC */
        PcbPlc_InPortB,                     /* 25 */
        PcbPlc_OutPortB,                    /* 26 */
        PcbPlc_UserGetBit,                  /* 27 */
        PcbPlc_UserSetBit,                  /* 28 */
        PcbPlc_UserClrBit,                  /* 29 */
        PcbPlc_SetHiTimer,                  /* 30 */
        PcbPlc_LoadMimicFile,               /* 31 */
        PcbPlc_PlaySoundFile,               /* 32 */
        PcbPlc_PlcCos,                      /* 33 - MATH */
        PcbPlc_PlcSine,                     /* 34 - MATH  */
        PcbPlc_PlcTan,                      /* 35 - MATH  */
        PcbPlc_PlcAsine,                    /* 36 - MATH  */
        PcbPlc_PlcAcos,                     /* 37 - MATH  */
        PcbPlc_PlcAtan,                     /* 38 - MATH  */
        PcbPlc_PlcPower,                    /* 39 */
        PcbPlc_PlcSqrt,                     /* 40 - MATH  */
        PcbPlc_PlcLog,                      /* 41 */
        PcbPlc_PlcLog10,                    /* 42 - MATH  */
        PcbPlc_DummyHandler,                /* 43 - DUMMY */
        PcbPlc_SendAlarm,                   /* 44 */
        PcbPlc_DummyHandler,                /* 45 - DUMMY  */
        PcbPlc_ChkSchedule,                 /* 46 - SCHEDULE  */
        PcbPlc_DummyHandler,                /* 47 - DUMMY  */
        PcbPlc_SetDO,                       /* 48 */
        PcbPlc_DummyHandler,                /* 49 - DUMMY  */
        PcbPlc_GetWeekDay,                  /* 50 - RTC */
        PcbPlc_SetPValue,                   /* 51 */
        PcbPlc_SendData,                    /* 52 - DUMMY  */
        PcbPlc_DummyHandler,                /* 53 - DUMMY  */
        PcbPlc_stringlength,                /* 54 - STRING */
        PcbPlc_strmid,                      /* 55 - STRING */
        PcbPlc_strinstring,                 /* 56 - STRING */
        PcbPlc_stringsplit,                 /* 57 - STRING */
        PcbPlc_assignstring,                /* 58 - STRING */
        PcbPlc_ClearPLCDisplay,             /* 59 - DISPLAY */
        PcbPlc_ClearPLCDisplay,             /* 60 - DISPLAY */
        PcbPlc_Isnumeric,                   /* 61 - STRING */
        PcbPlc_texttoval,                   /* 62 - STRING */
        PcbPlc_SendAlarmSMS,                /* 63 - SMS */
        PcbPlc_ClearPLCDisplay,             /* 64 - DISPLAY  */
        PcbPlc_ClearPLCDisplay,             /* 65 - DISPLAY  */
        PcbPlc_ClearPLCDisplay,             /* 66 - DISPLAY  */
        PcbPlc_ClearPLCDisplay,             /* 67 - DISPLAY  */
        PcbPlc_ClearPLCDisplay,             /* 68 - DISPLAY  */
        PcbPlc_ClearPLCDisplay,             /* 69 - DISPLAY  */
        PcbPlc_BuildSms,                    /* 70 - SMS */
        PcbPlc_ChkSchedule,                 /* * 71 - SCHEDULE */
        PcbPlc_CheckAutoManual,             /* * 72 - MODE  */
        PcbPlc_SetAutoMode,                 /* 73 - MODE  */
        PcbPlc_SetManualMode,               /* 74 - MODE  */
        PcbPlc_SendDataInSMS,               /* 75 - SMS */
        PcbPlc_Analog_output_Plc,           /* 76 - AO */
        PcbPlc_DO_Key_Status,               /* 77 - DO STATUS */
        PcbPlc_Get_sch_time,                /* * 78 - SCHEDULE  */
        };

/*****************************************************************************/
/* Main function to execute plc logic sequenc                                */
/* If PlcFlag is on, this function is being called from baseloop for
 * plc logic execution.												 */
/*****************************************************************************/
void pcbplc_BaseLoop()
{
    if(seqStep>=0)
    {
        /** bPCBFlag will be use full when we load logic through web interface */
        bPCBFlag = 1;
        pcbplc_process();
        bPCBFlag = 0;
    }
}

/*****************************************************************************/
/*      mem_alloc(): Allocate memory for logic execution arrays.			 */
/* 		This function called,when logic execution starts.					 */
/*****************************************************************************/
short int mem_alloc()
{
    memset(outBits, 0, sizeof (outBits));
    memset(accessTable, 0, sizeof (accessTable));
    memset(feedbackArr, 0, sizeof (feedbackArr));
    memset(waitingFor, 0, sizeof (waitingFor));
    memset(modifiedArr, 0, sizeof (modifiedArr));
    memset(conflictArr, 0, sizeof (conflictArr));

    falseInterlock = (unsigned char *)PlcAlloc(pcbplc_logger, maxIntNode*sizeof(unsigned char));
    if (!falseInterlock)
    {
        return(0);
    }


    return(1);
}


void pcbplc_memory_free()
{
    if(plcDataFile)
    {
        for(int tIdx = 0; tIdx < maxNodes; ++tIdx)
        {
            if(plcDataFile[tIdx].str)
            {
                free(plcDataFile[tIdx].str);
                plcDataFile[tIdx].str = NULL;
            }
        }

        free(plcDataFile);
        plcDataFile = NULL;
    }
}
/*****************************************************************************/
/*      clock_pcbplc(): returns number of seconds elapsed after starting RTU		 */
/*****************************************************************************/
unsigned long int clock_pcbplc(void)
{
    return (plcTimerTicks);
}
/*****************************************************************************/
/*      copy the main variables to temp. variables	 */
/* 		All the logic execution related calculation takes place in temp.
                variables. So before start the current execution it copies all the
                main variables set to temp. variables.								 */
/*****************************************************************************/
void copy_to_temp_table(void)
{
    short int i;

    memmove(outBits, dig_bit_array, totalBits);
    memset(modifiedArr, 0, sizeof(unsigned char)*totalBits);

    for(i = 0 ; i < totalBits ; i++)
    {
        if(feedbackArr[i])
        {
            if(dig_bit_array[i] == waitingFor[i])
            {
                feedbackArr[i] = 0;
            }
            else
            {
                feedbackArr[i]++;
            }
        }
    }
}

/*****************************************************************************/
/*      copy_to_main_table(): copy the temp. variables to main variables.	 */
/*		This function copies the temp. variables to main variables. After
                all calculations takes place, the result is copied in temp. variables.
                At the end of the current baseloop, this temp. variables are copied
                into the main variables set.										 */
/*****************************************************************************/
short int copy_to_main_table(void)
{
    int i,flag=1;

    for(i = 0 ; i < totalBits ; ++i)
    {
        if( (modifiedArr[i]) && (dig_bit_array[i] != outBits[i]) )
        {
            if(accessTable[i])
            {
                if(outBits[i])
                {
                    outputDOflag[i] = 1;
                }
                else
                {
                    outputDOflag[i] = 0;
                }

                waitingFor[i] = outBits[i];

                if(overRideFlagArr[i])
                {
                    dig_bit_array[i] = outBits[i];
                }
                else
                {
                    feedbackArr[i]++;
                }
            }
            else
            {
                flag = 0;
            }
        }
    }

    return(flag);
}

/*****************************************************************************
 * processes a plc file.
 * This function processes the plce file.
 ******************************************************************************/
short int pcbplc_process(void)
{
    short int i;

    paramSp = 0;                               // changed by samir for subroutine not work properly.
    curStep = seqStep;

    if ((maxSeqNode <= 0) || (seqStep < 0))
    {
        PlcFlag = 0;
        return(0);
    }

    /* Energency logic execution */
    copy_to_temp_table();

    /* Function executing emergency steps */
    emrLastStep = pcbplc_emr_seq();
    if(emrLastStep == -1)
    {
        WriteLog(pcbplc_logger, "Emergency steps execution not started\r\n", LOG_ERROR);
        PlcFlag = 0;
    }

    /* Sequence logic execution */
    executing = SEQ;
    memset(conflictArr, 0, sizeof(unsigned char)*totalBits);

    i = PcbPlc_Logic_Interpreter(plcDataFile[seqStep].str, 0);

    if(plcDataFile[seqStep].type == OPERATION_BOX)
    {
        seqStep = plcDataFile[seqStep].yes;
    }
    else if(plcDataFile[seqStep].type == CONDITION_BOX)
    {
        if(i)
        {
            seqStep = plcDataFile[seqStep].yes;
        }
        else
        {
            seqStep = plcDataFile[seqStep].no;
        }
    }

    /* TODO : Need to understand below logic */
    if( (seqStep < 0 ) && (paramStack[0]))
    {
        seqStep = paramStack[--paramSp];
        paramSp -= paramStack[--paramSp];
        paramStack[0]--;
    }

    makeAccessTable();
    copy_to_main_table();

    for(i = 0 ; i < totalBits ; ++i)
    {
        if( ( (modifiedArr[i]==1) && (!accessTable[i]) ) || (conflictArr[i]) )
        {
            seqStep = curStep;
            break;
        }
    }
    return(1);
}

/*****************************************************************************/
/*      MakeAccessTable(): Makes the access table 					 		*/
/*****************************************************************************/
void makeAccessTable(void)
{
    unsigned char *atr;
    short int k,ret,i;

    memset(accessTable, 1, sizeof(unsigned char)*totalBits);
    memset(falseInterlock, 0, sizeof(unsigned char)*maxIntNode);

    if(maxIntNode > 0)
    {
        intStep = intStartNode;

        while( (intStep >= 0) && (intStep != 65535) )
        {
            atr=strchr((const char*)string,(int)';');
            if(atr)
            {
                (*atr) = '\0';
            }

            executing = INT;
            k = atoi((const char*)string);

            ret = PcbPlc_Logic_Interpreter(string,0);
            if(ret)
            {
                PcbPlc_Logic_Interpreter(atr+2,1);         /* Interlock not satisfied */
                falseInterlock[intStep-intStartNode] = 1;
            }

            executing = SEQ;
            waitingFlag = 0;
            ret = PcbPlc_Logic_Interpreter(string,0);
            if(atr)
            {
                k = atoi((const char*)atr+2);
            }

            if(waitingFlag)
            {
                accessTable[k] = 0;
            }
            else
            {
                accessTable[k] &= (!ret);
            }

            intStep = plcDataFile[intStep].yes;
        }
    }

    for(i = 0 ; i < totalBits ; ++i)
    {
        if(feedbackArr[i])
        {
            accessTable[i] = 0;
        }
    }
}

extern unsigned short fnInterruptMessage(signed char Task, unsigned char ucIntEvent);

/*****************************************************************************/
/*      pcbplc_emr_seq(): Emergency sequence get executed from this routine	 */
/*****************************************************************************/
short int pcbplc_emr_seq(void)
{
    short int k,i;
	//short int j = 0;
    //char TP[128] = {0,};
    char tBuffer[256] = {0, };

    /* if no emergency node then return -1 */
    if(maxEmrNode <= 0)
    {
        sprintf(tBuffer, "maxEmrNode(%d) is less than zero\r\n", maxEmrNode);
        WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
        return(-1);
    }

    emrStep = emrStartNode;
    executing = EMR;                      // Indicating emrgency sequence is executed

    while( (emrStep >= 0) && (emrStep != 65535) )
    {
        k = emrStep;

        #if 0
        j++;

        if(j >= 100)
        {
            sprintf(tBuffer, "PCBPLC: j = %d\r\n", j);
            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
            j = 0;
        }
        #endif

        i = PcbPlc_Logic_Interpreter(plcDataFile[emrStep].str, 1);

        if (plcDataFile[emrStep].type == OPERATION_BOX)
        {
            emrStep = plcDataFile[emrStep].yes;
        }
        else if (plcDataFile[emrStep].type == CONDITION_BOX)
        {
            if(i)
            {
                emrStep = plcDataFile[emrStep].yes;
            }
            else
            {
                emrStep = plcDataFile[emrStep].no;
            }
        }

        /* TODO: Need to understand below logic */
        if( (emrStep < 0 ) && (paramStack[299]) )
        {
            emrStep = paramStack[--paramSp];
            paramSp -= paramStack[--paramSp];
            paramStack[299]--;
        }

        if(reset_flag == 1)
        {
            reset_flag = 0;
            break;
        }
    }

    return(k);
}

/*********************************************************************************************************/
/* Reads PlcFile array and decodes it.		 							 */
/* This function reads the downloaded plcfile, decodes it and initializes all the buffers and variables.							 */
/*********************************************************************************************************/

int PcbPlc_FileReadAndParse(unsigned char pcbplc_logger, unsigned char pPlcFile, unsigned char pRecFile)
{
	short int len = 0;
    unsigned char tBuffer[1024] = {0,};
    //unsigned char tBuffer1[2048] = {0,};
    char tDebug[1200] = {0,};
    int i = 0;
    int tRetVal = -1;
    unsigned int rawAddr=0;

    do
    {
    	gPcbplcInfo.mPlcFileLength = gPlcRecFlash.mPlcFileLength;
    	gPcbplcInfo.mRecFileLength = gPlcRecFlash.mRecFileLength;

    	sprintf(tDebug, "Plc file(%d) Length(%ld)\r\n", pPlcFile, gPcbplcInfo.mPlcFileLength);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        if(0 == gPcbplcInfo.mPlcFileLength)
        {
            gPcbplcInfo.mPlcFileError = 1;
            break;
        }

        gPcbplcInfo.mPlcFileError = 0;

        /* Skipping 89 bytes - Which are frame header bytes and other information data.
         * Frame header : PLC_HEADER_LEN + 1 + (sizeof(short int)*2) + sizeof(long);
         * */
        rawAddr = PUB_FILE_START_ADDRESS;  // location in flash for PLC file as shreyans

        memset(tBuffer, 0 ,sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, PLC_FILE_HEADER_LENGTH, rawAddr) ;rawAddr += PLC_FILE_HEADER_LENGTH;

        // todo : check file name to identify pCBPLC file only

        if(FindSubstr((char *)tBuffer,"Personnel Computer Based Programmable Logic Controller")==-1)
        {
        	WriteLog(pcbplc_logger, "PCBPLC header Not match", LOG_ERROR);
        	break;
        }

        /* Get Next two bytes : Which represent MaxNodes */
        memset(tBuffer, 0 ,sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        maxNodes = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "PBCPLC: Max Nodes(%d)\r\n", maxNodes);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        /* Get Next two bytes : Which represent MaxSeqNodes */
        memset(tBuffer, 0 ,sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        maxSeqNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "PBCPLC: Max Sequence Node(%d)\r\n", maxSeqNode);
        WriteLog(pcbplc_logger,tDebug, LOG_INFO);

        /* Get Next two bytes : Which represent MaxDigitalNodes */
        memset(tBuffer, 0 ,sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        totalBits = GetInt(tBuffer[0],tBuffer[1]);

        maxDig = totalBits;
        sprintf(tDebug, "PBCPLC: Max Digital Nodes(%d)\r\n", maxDig);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        /* Get Next two bytes : Which represent MaxAnaNodes */
        memset(tBuffer, 0 ,sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        totalFloats = GetInt(tBuffer[0],tBuffer[1]);

        maxAna = totalFloats;
        sprintf(tDebug, "PBCPLC: Max Analog Nodes(%d)\r\n", maxAna);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        /* Allocate memory for MaxNodes */
        plcDataFile = (struct splcDataFile *) PlcAlloc(pcbplc_logger, maxNodes*sizeof(struct splcDataFile));
        if (NULL == plcDataFile)   // if function returns 0 means data allocation
        {
            sprintf(tDebug, "Memory allocation failed for Max Nodes(%d)\r\n", maxNodes);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);
            break;
        }

        /* Extract information of SeqNodes */
        for(i = 0 ; i < maxSeqNode ; ++i)
        {
            /* Type */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].type = GetInt(tBuffer[0], tBuffer[1]);

            /* IN */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].in = GetInt(tBuffer[0],tBuffer[1]);

            /* YES */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].yes = GetInt(tBuffer[0],tBuffer[1]);

            /* NO */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].no = GetInt(tBuffer[0],tBuffer[1]);

            /* Length of Data String */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            len = GetInt(tBuffer[0],tBuffer[1]);

            /* Allocate memory for data string */
            plcDataFile[i].str = (unsigned char *) PlcAlloc(pcbplc_logger, (len+1)*sizeof(unsigned char));
            if(!plcDataFile[i].str)
            {
                sprintf(tDebug, "Error at plc data-string allocation failed\r\n");
                WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
                break;
            }

            /* Data String */
            memset(tBuffer,0,sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, len, rawAddr);
            rawAddr += len;
            strcpy((char *)plcDataFile[i].str,(const char *)tBuffer);

            plcDataFile[i].str[len] = '\0';

            sprintf(tDebug, "STRING:%s\r\n", plcDataFile[i].str);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);
        }

        if(i != maxSeqNode)
        {
            sprintf(tDebug, "PCBPLC Error i(%d), maxSeqNode(%d)\r\n", i, maxSeqNode);
            WriteLog(pcbplc_logger,tDebug, LOG_ERROR);
            break;
        }

        /* Get Max PLC Variable */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        maxPlcVar = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "MAX PLC VARIABLE(%d)\r\n", maxPlcVar);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        if(maxPlcVar > 0)
        {
            if(maxPlcVar > 500)
            {
                // max 100 variables allowed
                sprintf(tDebug, "Error: Max PLC Var(%d) not allowed, only 500 PLC variable allowed\r\n",maxPlcVar);
                WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
                break;
            }

            for(i = 0 ; i < maxPlcVar ; i++)
            {
                /* Get Length of PLC variables */
                memset(tBuffer, 0, sizeof(tBuffer));
                MX25L_ReadRaw(tBuffer, TWO, rawAddr);
                rawAddr += TWO;
                len = GetInt(tBuffer[0],tBuffer[1]);

                memset(tBuffer, 0, sizeof(tBuffer));
                MX25L_ReadRaw(tBuffer, len, rawAddr);
                rawAddr += len;

                sprintf(tDebug, "PLC VAR NAME_%02d : %s\r\n", i, tBuffer);
                WriteLog(pcbplc_logger, tDebug, LOG_INFO);
            }

            if(i != maxPlcVar)
            {
                sprintf(tDebug, "i(%d), maxPlcVar(%d)\r\n", i, maxPlcVar);
                WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
                break;
            }
        }

        /* Get Start seq Node */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        seqStartNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "PBCPLC: seqStartNode=%d\r\n", seqStartNode);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        /* Get Number of emergency node */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        maxEmrNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "PBCPLC: maxEmrNode=%d\r\n", maxEmrNode);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        for(i = maxSeqNode ; i < (maxSeqNode+maxEmrNode) ; ++i)
        {
            /* Type */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;

            plcDataFile[i].type = GetInt(tBuffer[0],tBuffer[1]);

            /* IN */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;

            plcDataFile[i].in = GetInt(tBuffer[0],tBuffer[1]);

            /* YES */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;

            plcDataFile[i].yes = GetInt(tBuffer[0],tBuffer[1]);

            /* NO */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;

            plcDataFile[i].no = GetInt(tBuffer[0],tBuffer[1]);

            /* LEN */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;

            len = GetInt(tBuffer[0],tBuffer[1]);

            /* Allocate memory for data string */
            plcDataFile[i].str = (unsigned char *) PlcAlloc(pcbplc_logger, (len+1)*sizeof(unsigned char));
            if (!plcDataFile[i].str)
            {
                WriteLog(pcbplc_logger,"PCBPLC Memory allocation Error\r\n", LOG_ERROR);
                break;
            }

            /* Get Data string */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, len, rawAddr);
            rawAddr += len;

            strcpy((char *)plcDataFile[i].str,(const char *)tBuffer);

            plcDataFile[i].str[len] = '\0';

            //sprintf(tDebug, "Emr Node(%02d): %s\r\n", i-maxSeqNode+1, tBuffer);
            //WriteLog(pcbplc_logger,tDebug, LOG_INFO);
        }

        if(i != (maxSeqNode+maxEmrNode))
        {
            sprintf(tDebug, "i(%02d), (maxSeqNode+maxEmrNode)(%d)\r\n", i, (maxSeqNode+maxEmrNode));
            WriteLog(pcbplc_logger,tDebug, LOG_ERROR);
            break;
        }

        /* Get Emergency start node */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        emrStartNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "Emr Start Node: %d\r\n", emrStartNode);
        WriteLog(pcbplc_logger,tDebug, LOG_INFO);

        /* Maximun Int nodes */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        maxIntNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "PBCPLC: maxIntNode=%d\r\n", maxIntNode);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        for(i = (maxSeqNode+maxEmrNode); i < (maxSeqNode+maxEmrNode+maxIntNode) ; ++i)
        {
            /* TYPE */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].type = GetInt(tBuffer[0],tBuffer[1]);

            /* IN */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].in = GetInt(tBuffer[0],tBuffer[1]);

            /* YES */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].yes = GetInt(tBuffer[0],tBuffer[1]);

            /* NO */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            plcDataFile[i].no = GetInt(tBuffer[0],tBuffer[1]);

            /* Length of data string */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, TWO, rawAddr);
            rawAddr += TWO;
            len = GetInt(tBuffer[0],tBuffer[1]);

            /* Allocate memory for data string */
            plcDataFile[i].str = (unsigned char *) PlcAlloc(pcbplc_logger, (len+1)*sizeof(unsigned char));
            if(!plcDataFile[i].str)
            {
                WriteLog(pcbplc_logger,"PCBPLC memory allocation Error\r\n", LOG_ERROR);
                break;
            }

            /* Get Data */
            memset(tBuffer, 0, sizeof(tBuffer));
            MX25L_ReadRaw(tBuffer, len, rawAddr);
            rawAddr += len;
            strcpy((char *)plcDataFile[i].str,(const char *)tBuffer);

            plcDataFile[i].str[len] = '\0';

            sprintf(tDebug, "Interlock Node(%02d): %s\r\n", i-(maxSeqNode+maxEmrNode)+1, tBuffer);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);
        }

        if(i != (maxSeqNode+maxEmrNode+maxIntNode))
        {
            sprintf(tDebug, "i(%d), (maxSeqNode+maxEmrNode+maxIntNode)(%d)\r\n",i,
                                    (maxSeqNode+maxEmrNode+maxIntNode));
            WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
            break;
        }

        /* Get Int Start node */
        memset(tBuffer, 0, sizeof(tBuffer));
        MX25L_ReadRaw(tBuffer, TWO, rawAddr);
        rawAddr += TWO;
        intStartNode = GetInt(tBuffer[0],tBuffer[1]);

        sprintf(tDebug, "intStartNode:%d\r\n", intStartNode);
        WriteLog(pcbplc_logger,tDebug, LOG_INFO);

        WriteLog(pcbplc_logger,"PLC FILE READ END\r\n", LOG_INFO);

    }while(0);

//    if(gPlcRecFlash.extract_receipe == 0)
//    {
//        tRetVal = 0;
//        return (tRetVal);
//    }

    do
    {
    	gPcbplcInfo.mRecFileLength = gPlcRecFlash.mRecFileLength;
    	if(0 == gPcbplcInfo.mRecFileLength)
    	{
    		gPcbplcInfo.mRecFileError = 1;
            break;
        }

    	rawAddr = REC_FILE_START_ADDRESS;

        gPcbplcInfo.mRecFileError = 0;

        sprintf(tDebug, "RECIPE FILE: Length=%ld\r\n",gPcbplcInfo.mRecFileLength);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        gPcbplcInfo.mMaxRecipeVar = gPcbplcInfo.mRecFileLength/REC_FILE_ONE_CONTENT_LENGTH;

        sprintf(tDebug, "MAX RECIPE VAR:%02ld\r\n", gPcbplcInfo.mMaxRecipeVar);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        for(i = 0 ; i < gPcbplcInfo.mMaxRecipeVar ; ++i)
        {
            char temp[12] = {0,};
            unsigned int tRcpVarIndex = 0;

            memset(tBuffer, 0 ,sizeof(tBuffer));

            MX25L_ReadRaw(tBuffer, REC_FILE_ONE_CONTENT_LENGTH, rawAddr) ;
            rawAddr += REC_FILE_ONE_CONTENT_LENGTH;

            /* * 1. plc_type: what are the types and its meaning.
                    plc_type = 1 ==> Constant Variable.
                    plc_type = 2 ==> List variable, it has a list of value, among of its need to select.
                    plc_type = 3 ==> Range Variable, it has low and high limit.

                    2. Rcp_var_index : What is a range of index.
                    Its a index, where that variable need to store in service.

                    3. what are the meaning of Plc_var_1, Plc_var_2, Plc_var_3, Plc_var_4, Plc_var_5.
                    Its values depend on the plc_type.

                    4. Receipe variable can be changed from keyboard which have a plc_type = 3 (range Variable)
                */

            /* Derived points: For Received bytes it creates U.BIN file.
                 * After reading the file, service decode it and create M.BIN file .
                 * MAX variable it store in M.BIN is 500.
                */

            tRcpVarIndex = (tBuffer[1]*256) + tBuffer[0];

            RECindex[i] = tRcpVarIndex;

            sprintf(tDebug, "Recipe Var Index:%02hd, \r\n", tRcpVarIndex);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            Plc_type = tBuffer[2];

            sprintf(tDebug, "Plc_type:%d, \r\n", Plc_type);
            WriteLog(pcbplc_logger,tDebug, LOG_INFO);

            memcpy(temp, &tBuffer[3], 10);
            temp[10] = 0;

			#if 1
            strcpy(&gPcbplcInfo.mPlcVarArrName[tRcpVarIndex][0], temp);
            sprintf(tDebug, "REC_VAR NAME:%s,\r\n", gPcbplcInfo.mPlcVarArrName[tRcpVarIndex]);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);
			#else
            strcpy(&gPcbplcInfo.mPlcVarArrName[i][0], temp);
			sprintf(tDebug, "REC_VAR NAME:%s,\r\n", gPcbplcInfo.mPlcVarArrName[i]);
			WriteLog(pcbplc_logger, tDebug, LOG_INFO);
			#endif
            Plc_var_1 = ConvertFloatRec(&tBuffer[13]);
            Plc_var_2 = ConvertFloatRec(&tBuffer[17]);
            Plc_var_3 = ConvertFloatRec(&tBuffer[21]);
            Plc_var_4 = ConvertFloatRec(&tBuffer[25]);
            Plc_var_5 = ConvertFloatRec(&tBuffer[29]);

            sprintf(tDebug, "Var1 = %3.2f, Var2 = %3.2f, Var3 = %3.2f, Var4 = %3.2f, Var5 = %3.2f\r\n",
                    Plc_var_1, Plc_var_2, Plc_var_3, Plc_var_4, Plc_var_5);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

			#if 1
            plcVarArr[tRcpVarIndex] = Plc_var_1;
            gPcbplcInfo.mPlcVarArr_2[tRcpVarIndex] = Plc_var_2;
            gPcbplcInfo.mPlcVarArr_3[tRcpVarIndex] = Plc_var_3;
            gPcbplcInfo.mPlcVarArr_4[tRcpVarIndex] = Plc_var_4;
            gPcbplcInfo.mPlcVarArr_5[tRcpVarIndex] = Plc_var_5;
			#else
            plcVarArr[i] = Plc_var_1;
			gPcbplcInfo.mPlcVarArr_2[i] = Plc_var_2;
			gPcbplcInfo.mPlcVarArr_3[i] = Plc_var_3;
			gPcbplcInfo.mPlcVarArr_4[i] = Plc_var_4;
			gPcbplcInfo.mPlcVarArr_5[i] = Plc_var_5;
			#endif

            if(gPlcRecFlash.extract_receipe == 1)
            {
				#if 1
            	gPcbplcInfo.mPlcVarArr_1[tRcpVarIndex] = Plc_var_1;
            	//gPcbplcInfo.mPlcVarArr[tRcpVarIndex] = plcVarArr[tRcpVarIndex];
            	gPcbplcInfo.mPlcVarTypeArr[tRcpVarIndex] = Plc_type;
            	gPlcRecFlash.mPlcVarTypeArr[tRcpVarIndex] = gPcbplcInfo.mPlcVarTypeArr[tRcpVarIndex];
            	gPlcRecFlash.plcVarArr[tRcpVarIndex] = gPcbplcInfo.mPlcVarArr_1[tRcpVarIndex];
            	//gFinalAnaValF[RECIPE_VAR_START_ARRAY_INDEX + tRcpVarIndex] = gPcbplcInfo.mPlcVarArr_1[tRcpVarIndex];
            	gFinalAnaValF[RECIPE_VAR_START_ARRAY_INDEX + i] = gPcbplcInfo.mPlcVarArr_1[RECindex[i]];
				#else

            	gPcbplcInfo.mPlcVarArr_1[i] = Plc_var_1;
				//gPcbplcInfo.mPlcVarArr[i] = plcVarArr[i];
				gPcbplcInfo.mPlcVarTypeArr[i] = Plc_type;
				gPlcRecFlash.mPlcVarTypeArr[i] = gPcbplcInfo.mPlcVarTypeArr[i];
				gPlcRecFlash.plcVarArr[i] = gPcbplcInfo.mPlcVarArr_1[i];
				gFinalAnaValF[RECIPE_VAR_START_ARRAY_INDEX + i ] = gPcbplcInfo.mPlcVarArr_1[i];
				#endif
            }
        }

        if(i != gPcbplcInfo.mMaxRecipeVar)
        {
            break;
        }

        tRetVal = 0;

    }while(0);

    if(gPlcRecFlash.extract_receipe == 1)
    {
    	gPlcRecFlash.extract_receipe = 0;
    	ExtFlash_update_gPlcRecFlash();
    	//ReadRecipeOrigionalFile(pRecFile);

    	/* Recipe file creation using .REC file received from user */
    	WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
    }

    return(tRetVal);
}

int ReadRecipeJsonFile()
{
//    FILE *tFp = NULL;
//    char tBuffer[256] = {0,};
//    char tFileName[256] = RECIPE_JSON_FILE_PATH;
//    struct stat st = {0,};
//    int tRetVal = -1;
//    json_object *tMainObj = NULL;
//    int tReceipeArrLen = 0;
//    int tIdx = 0;
//
//    errno = 0;
//
//    do
//    {
//        errno = 0;
//        tMainObj = json_object_from_file(RECIPE_JSON_FILE_PATH);
//        if(NULL == tMainObj)
//        {
//            sprintf(tBuffer, "Error tMainObj(%p) is null at json_tokener_parse\r\n", tMainObj);
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//            break;
//        }
//
//        tReceipeArrLen = json_object_array_length(tMainObj);
//
//        for (tIdx = 0; tIdx < tReceipeArrLen; ++tIdx)
//        {
//            json_object *tSubObj = json_object_array_get_idx(tMainObj, tIdx);
//            if (NULL != tSubObj)
//            {
//                int tIndex = json_object_get_int(json_object_object_get(tSubObj, "index"));
//                strcpy(gPcbplcInfo.mPlcVarArrName[tIndex], json_object_get_string(json_object_object_get(tSubObj, "name")));
//                gPcbplcInfo.mPlcVarTypeArr[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "type"));
//                gPcbplcInfo.mPlcVarArr_1[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "var1"));
//                plcVarArr[tIndex] = gPcbplcInfo.mPlcVarArr_1[tIndex];
//                gPcbplcInfo.mPlcVarArr_2[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "var2"));
//                gPcbplcInfo.mPlcVarArr_3[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "var3"));
//                gPcbplcInfo.mPlcVarArr_4[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "var4"));
//                gPcbplcInfo.mPlcVarArr_5[tIndex] = json_object_get_double(json_object_object_get(tSubObj, "var5"));
//
//                printf("Idx:%d , Name:%s, Type:%d, Var1:%f, Var2:%f, Var3:%f, Var4:%f, Var5:%f\n", tIndex, gPcbplcInfo.mPlcVarArrName[tIndex], gPcbplcInfo.mPlcVarTypeArr[tIndex],
//                       gPcbplcInfo.mPlcVarArr_1[tIndex], gPcbplcInfo.mPlcVarArr_2[tIndex] , gPcbplcInfo.mPlcVarArr_3[tIndex],
//                       gPcbplcInfo.mPlcVarArr_4[tIndex], gPcbplcInfo.mPlcVarArr_5[tIndex]);
//            }
//        }
//
//        json_object_put(tMainObj);
//
//    }while(0);
//
//    for(int tDoIdx = 0 ; tDoIdx < gpcbplcCnfg.mMaxDoEnabled ; ++tDoIdx)
//    {
//            OldoutputDOflag[tDoIdx] = 0xFF;
//    }

    return 0;
}

#if 0
static int ReadRecipeOrigionalFile(const char *pRecFile)
{
    FILE *tFp = NULL;
    unsigned char tBuffer[128] = {0,};
    char tDebug[256] = {0,};
    int tCnt = 0, i = 0;
    int tRetVal = -1;

    if(NULL == pRecFile)
    {
        return (tRetVal);
    }

    tFp = fopen(pRecFile, "rb");
    if(NULL == tFp)
    {
        gPcbplcInfo.mRecFileError = 1;

        sprintf(tDebug, "Error at to open %s file", pRecFile);
        WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
        return (tRetVal);
    }

    do
    {
        fseek(tFp, 0L, SEEK_END);
        gPcbplcInfo.mRecFileLength = ftell(tFp);
        rewind(tFp);

        if(0 == gPcbplcInfo.mRecFileLength)
        {
            gPcbplcInfo.mRecFileError = 1;
            break;
        }

        gPcbplcInfo.mRecFileError = 0;

        sprintf(tDebug, "RECIPE FILE: Length=%d\r\n",gPcbplcInfo.mRecFileLength);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        gPcbplcInfo.mMaxRecipeVar = gPcbplcInfo.mRecFileLength/REC_FILE_ONE_CONTENT_LENGTH;

        sprintf(tDebug, "MAX RECIPE VAR:%02d\r\n", gPcbplcInfo.mMaxRecipeVar);
        WriteLog(pcbplc_logger, tDebug, LOG_INFO);

        for(i = 0 ; i < gPcbplcInfo.mMaxRecipeVar ; ++i)
        {
            char temp[12] = {0,};
            unsigned int tCnt = 0 ;
            unsigned int tRcpVarIndex = 0;

            memset(tBuffer, 0 ,sizeof(tBuffer));

            tCnt = fread(tBuffer,1 ,REC_FILE_ONE_CONTENT_LENGTH,tFp);
            if(REC_FILE_ONE_CONTENT_LENGTH != tCnt)
            {
                WriteLog(pcbplc_logger,"RECIPE File Read Error\r\n", LOG_ERROR);
                break;
            }

            /* *    1. plc_type: what are the types and its meaning.
                    2. Rcp_var_index : What is a range of index.
                    3. what are the meaning of Plc_var_1, Plc_var_2, Plc_var_3, Plc_var_4, Plc_var_5.
                */
            /* Derived points: For Received bytes it creates U.BIN file.
                 * After reading the file, service decode it and create M.BIN file .
                 * MAX variable it store in M.BIN is 500.
                */
            tRcpVarIndex = (tBuffer[1]*256) + tBuffer[0];

            sprintf(tDebug, "Recipe Var Index:%02hd, \r\n", tRcpVarIndex);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            Plc_type = tBuffer[2];

            sprintf(tDebug, "Plc_type:%d, \r\n", Plc_type);
            WriteLog(pcbplc_logger,tDebug, LOG_INFO);

            memcpy(temp, &tBuffer[3], 10);
            temp[10] = 0;

            sprintf(tDebug, "VAR:%s,\r\n", temp);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            Plc_var_1 = ConvertFloatRec(&tBuffer[13]);
            Plc_var_2 = ConvertFloatRec(&tBuffer[17]);
            Plc_var_3 = ConvertFloatRec(&tBuffer[21]);
            Plc_var_4 = ConvertFloatRec(&tBuffer[25]);
            Plc_var_5 = ConvertFloatRec(&tBuffer[29]);

            sprintf(tDebug, "Var1 = %3.2f, Var2 = %3.2f, Var3 = %3.2f, Var4 = %3.2f, Var5 = %3.2f\r\n",
                    Plc_var_1, Plc_var_2, Plc_var_3, Plc_var_4, Plc_var_5);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            gPcbplcInfo.mPlcVarArr[tRcpVarIndex] = Plc_var_1;
            gPcbplcInfo.mPlcVarTypeArr[tRcpVarIndex] = 1 ; //As per the RTU 9000 logic //Plc_type;
        }

        if(i != gPcbplcInfo.mMaxRecipeVar)
        {
            break;
        }

        tRetVal = 0;

    }while(0);

    fclose(tFp);
    tFp = NULL;

    return (tRetVal);
}
#endif

int WriteModifiedRecipeFile(const char *pFile)
{
	unsigned char tBuf[8] = {0,};
    char tDebug[256] = {0,};
    int i = 0;
    int tStatus;
    int tRetVal = -1;

    MX25L_EraseSector_QPI(EPROM_REC_MODIFIED_START_ADDRESS);

    for(i=0 ; i<MAX_PLCVAR ; ++i)
    {
        //Dat.fl = gPcbplcInfo.mPlcVarArr[i];
        Dat.fl = gPcbplcInfo.mPlcVarArr_1[i];
        //if(i == 29)
        //  printf("gPcbplcInfo.mPlcVarArr[%d]:%f\n", i, gPcbplcInfo.mPlcVarArr[i]);
        tBuf[0]	= Dat.ch[0];
        tBuf[1]	= Dat.ch[1];
        tBuf[2]	= Dat.ch[2];
        tBuf[3]	= Dat.ch[3];

        //if(gPcbplcInfo.mPlcVarTypeArr[i] == 1)
        if(gPcbplcInfo.mPlcVarTypeArr[i])
        {
            tBuf[4] = 1;
        }
        else
        {
            tBuf[4] = 0;
        }

        tStatus = MX25L_PageProgram_QPI(tBuf, 5, EPROM_REC_MODIFIED_START_ADDRESS+(i*5));
        if(W25Q_OK != tStatus)
        {
        	sprintf(tDebug, "Error to write in file(%s)\r\n", pFile);
        	WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
        	break;
        }
    }

    if(i == MAX_PLCVAR)
    {
        tRetVal = 0;
    }

    return (tRetVal);
}

int ReadRecipeFile()
{
    unsigned char tBuf[8] = {0,};
    char tDebug[128] = {0,};
    int tStatus;

    for(int i=0 ; i<MAX_PLCVAR ; ++i)
    {
    	tStatus = MX25L_ReadRaw(tBuf, 5 ,EPROM_REC_MODIFIED_START_ADDRESS + (i*5));
    	if(W25Q_OK != tStatus)
    	{
    		sprintf(tDebug,"MX25L_ReadRaw error at %s\r\n", __func__);
    		WriteLog(pcbplc_logger, tDebug, LOG_INFO);
    		break;
    	}

        Dat.ch[0] = tBuf[0];
        Dat.ch[1] = tBuf[1];
        Dat.ch[2] = tBuf[2];
        Dat.ch[3] = tBuf[3];

        gPcbplcInfo.mPlcVarTypeArr[i] = tBuf[4];//gPlcRecFlash.mPlcVarTypeArr[i];

        if(1 == gPcbplcInfo.mPlcVarTypeArr[i]) /* Reading as per the RTU9000 logic */
        {
            gPcbplcInfo.mPlcVarArr_1[i] = Dat.fl;
            //plcVarArr[i] = gPlcRecFlash.plcVarArr[i];
            plcVarArr[i] = Dat.fl;

            sprintf(tDebug,"Read PV[%d]=%3.2f\r\n", i, gPcbplcInfo.mPlcVarArr_1[i]);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            sprintf(tDebug,"Read PV[%d]=%3.2f\r\n", i, plcVarArr[i]);
            WriteLog(pcbplc_logger, tDebug, LOG_INFO);

            //gFinalAnaValF[RECIPE_VAR_START_ARRAY_INDEX + i] = gPcbplcInfo.mPlcVarArr_1[i];
        }
    }

    for(int i=0 ; i<MAX_PLCVAR ; ++i)
    {
    	gFinalAnaValF[RECIPE_VAR_START_ARRAY_INDEX + i] = gPcbplcInfo.mPlcVarArr_1[RECindex[i]];
    }

    return 0;
}

short int GetInt(unsigned char ch1,unsigned char ch2)
{
    return ((ch2*256)+ch1);             // contructs int from two characters
}

void GetStringArryRec(unsigned char *pFile,unsigned long address,unsigned char *Arr,unsigned short len)
{
    //uGetFileData(pFile, address,Arr, len);
}

float ConvertFloatRec(unsigned char *bytes)
{
    //uGetFileData(pFile, address, ucB, 4);
    fpara.tempchar[0] = bytes[0];
    fpara.tempchar[1] = bytes[1];
    fpara.tempchar[2] = bytes[2];
    fpara.tempchar[3] = bytes[3];

    return(fpara.tempfloat);
}

float Readfloatrec(unsigned char *pFile,unsigned long address)
{
    unsigned char ucB[4]={0,};

    //uGetFileData(pFile, address, ucB, 4);
    fpara.tempchar[0] = ucB[0];
    fpara.tempchar[1] = ucB[1];
    fpara.tempchar[2] = ucB[2];
    fpara.tempchar[3] = ucB[3];
    return(fpara.tempfloat);
}

unsigned char ReadByterec(unsigned char *pFile,unsigned long address)
{
    unsigned char ucB[2]={0,};

    //uGetFileData(pFile, address,ucB, 1);
    return ucB[0];
}

short int ReadShortrec(unsigned char *pFile,unsigned long address)
{
    unsigned char ucB[2]={0,};
    //uGetFileData(pFile, address,ucB, 2);
    return(GetInt(ucB[0],ucB[1]));
}

/*****************************************************************************/
/************************ PCBPLC logic functions *****************************/
/*****************************************************************************/

/****************************************************************************
 * Function     :   PcbPlc_Confirm
 * Description  :   Get user confirmation, in actual, this function gets
 * user confirmation for some critical task. But here, it is always yes
 * for doing the task
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_Confirm()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    plcStack[stackIndex++] = 1;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcDisplay
 * Description  :   todo : need to specify after local GUI
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcDisplay(void)
{
    unsigned short int index_string;


    //system("clear");
    //printf("\n\n\n\n");

    if(str_flag == 1)
    {
        printf("\n%s\n", string1);
    }
    else
    {
        index_string = (short int)plcStack[stackIndex-1];
        printf("\n%s\n", stringbuff[index_string]);
    }

    //printf("\n");

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_ClearPLCDisplay
 * Description  :   todo : need to specify after local GUI
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ClearPLCDisplay(void)
{
    alarmToggleFlag = 0;
    plcDisplayFlag  = 1;
    return 0;
}

/****************************************************************************
 * Function     :   PcbPlc_PlcAlarmDisplay
 * Description  :   todo : need to specify after local GUI
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcAlarmDisplay(void)
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(alarmToggleFlag == 0)
    {
        printf("!!!!  ALARM  !!!!\n");
        alarmToggleFlag = 1;
    }
    else
    {
        alarmToggleFlag = 0;
    }

    if(str_flag == 1)
    {
        printf((const char *)string1);
    }

    plcDisplayFlag = 1;
    return 0;
}

/****************************************************************************
 * Function     :   PcbPlc_ResetPlcTimer
 * Description  :   Restart timer from beginning
 * arg          :   none
 * return       :   unconditonally 0
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ResetPlcTimer()
{
    short int timerNo;
    float   seconds;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 2;

    timerNo = (short int)plcStack[stackIndex];
    seconds = (float)plcStack[stackIndex+1];

    if(executing == INT)
    {
        //Timer Number: 200 to 299.
        timerNo += 200;
    }
    else if(executing == EMR)
    {
        //Timer Number: 100 to 199.
        timerNo += 100;
    }
    #if 0
    else if(executing == SEQ)
    {
        //Timer Number: 0 to 99
        timerNo += 0;
    }
    #endif

    clocks[timerNo].active = 1;

    if (!seconds)
    {
        seconds++;
    }

    clocks[timerNo].time    =   seconds * PCBPLC_CLK_TCK;
    clocks[timerNo].start   =   clock_pcbplc();
    clocks[timerNo].accu    =   0l;
    plcStack[stackIndex++]  =   0;

    return(0);
}

/****************************************************************************
 * Function     :   PcbPlc_SetPlcTimer
 * Description  :   Set a timer
 * arg          :   none
 * return       :   0
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetPlcTimer()
{
    short int timerNo;
    float seconds;

    stackIndex -= 2;

    timerNo = (short int)plcStack[stackIndex];
    seconds = (float)plcStack[stackIndex+1];

    if((timerNo < 1) || (timerNo >= 100))
    {
        plcStack[stackIndex++] = 0.0f;
        return(0);
    }

    if(executing == INT)
    {
        timerNo += 200;
    }
    else if(executing == EMR)
    {
        timerNo += 100;
    }

    if(clocks[timerNo].active == 0)     /* Passive mode */
    {
        clocks[timerNo].active = 1;

        if(seconds < 1)
        {
            clocks[timerNo].time = 1*PCBPLC_CLK_TCK;
        }
        else
        {
            clocks[timerNo].time = seconds*PCBPLC_CLK_TCK;
        }

        clocks[timerNo].start = clock_pcbplc();
        clocks[timerNo].accu = 0;
        plcStack[stackIndex++] = 0.0f;
        return(0);
    }

    if (clocks[timerNo].active == 2)        /* Hold mode */
    {
        if (clocks[timerNo].accu >= clocks[timerNo].time)
        {
            plcStack[stackIndex++] = 1.0f;
            return(1);
        }

        plcStack[stackIndex++] = 0.0f;
        return(0);
    }

    seconds = clock_pcbplc();
    seconds = seconds - clocks[timerNo].start + clocks[timerNo].accu;

    if (seconds >= clocks[timerNo].time)
    {
        plcStack[stackIndex++] = 1.0f;
        return(1);
    }
    plcStack[stackIndex++] = 0.0f;

    return(0);
}

/****************************************************************************
 * Function     :   PcbPlc_InitPlcTimer
 * Description  :   Initialised timer
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_InitPlcTimer()
{
    short int timerNo;

    stackIndex--;
    timerNo = (short int)plcStack[stackIndex];

    if (executing==INT)
    {
        timerNo += 200;
    }
    else if(executing==EMR)
    {
        timerNo += 100;
    }

    clocks[timerNo].active=0;
    clocks[timerNo].accu=0;
    clocks[timerNo].start=0;
    clocks[timerNo].time=0;

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_HoldPlcTimer
 * Description  :   Hold timer
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_HoldPlcTimer()
{
    short int timerNo;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex--;
    timerNo = (short int)plcStack[stackIndex];

    if (executing==INT)
    {
        timerNo += 200;
    }
    else if(executing==EMR)
    {
        timerNo += 100;
    }

    if(clocks[timerNo].active != 1)
    {
        return(0);
    }

    clocks[timerNo].active = 2;
    clocks[timerNo].accu += clock_pcbplc()-clocks[timerNo].start;

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_ReleasePlcTimer
 * Description  :   Makes the holded timer an Active timer.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ReleasePlcTimer()
{
    short int timerNo;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex--;
    timerNo=(short int)plcStack[stackIndex];

    if(executing==INT)
    {
        timerNo += 200;
    }
    else if(executing==EMR)
    {
        timerNo += 100;
    }

    if (clocks[timerNo].active!=2)        /* if not Hold mode */
    {
        return(0);
    }

    clocks[timerNo].start=clock_pcbplc();
    clocks[timerNo].active=1;

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_InitAllPlcTimers
 * Description  :   Initialize all the timers with default values.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_InitAllPlcTimers()
{
    short int s = 0;
    short int j = 100;
    short int k = 200;
    short int i;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    for(i = 0 ; i < 100 ;s++,j++,k++,i++)
    {
        /* As per RTU 9000 logic */
        clocks[s].active = clocks[s].accu = clocks[s].start = clocks[s].time = 0;
        clocks[j].active = clocks[j].accu = clocks[j].start = clocks[j].time = 0;
        clocks[k].active = clocks[k].accu = clocks[k].start = clocks[k].time = 0;

        #if 0
        //As per RTU 6000 logic
        clocks[s].active = clocks[s].accu = clocks[s].start = 0;
        clocks[j].active = clocks[j].accu = clocks[j].start = 0;
        clocks[k].active = clocks[k].accu = clocks[k].start = 0;
        #endif
    }

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetPlcTimer
 * Description  :   Get elapsed time of timer.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetPlcTimer()
{
    short int timerNo;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex--;

    timerNo = (short int)plcStack[stackIndex];

    if (executing==INT)
    {
        timerNo += 200;
    }
    else if (executing==EMR)
    {
        timerNo += 100;
    }

    if (clocks[timerNo].active==0) /* Not active */
    {
        plcStack[stackIndex++]=0;
    }
    else if (clocks[timerNo].active==2) /* if hold */
    {
        plcStack[stackIndex++]=(float)clocks[timerNo].accu/PCBPLC_CLK_TCK;
    }
    else /* Active Timer */
    {
        plcStack[stackIndex++]=(float)(clock_pcbplc()-clocks[timerNo].start+clocks[timerNo].accu)/PCBPLC_CLK_TCK;
    }

    return((short int)plcStack[stackIndex]);
}

/****************************************************************************
 * Function     :   PcbPlc_HoldAllPlcTimers
 * Description  :   Holds all the timers of perticular category.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_HoldAllPlcTimers()
{
    short int s,i;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(executing==INT)
    {
        s = 200;
    }
    else if(executing==EMR)
    {
        s = 100;
    }
    else
    {
        s = 0;
    }

    for(i = 0 ; i < 100 ; s++,i++)
    {
        if (clocks[s].active==2) /* if already Hold mode then no action*/
        {
            continue;
        }

        clocks[s].active = 2;
        clocks[s].accu += (clock_pcbplc()-clocks[s].start);
    }

    return(1);
} 

/****************************************************************************
 * Function     :   PcbPlc_ReleaseAllPlcTimers
 * Description  :   Releases all the timers of perticular category.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ReleaseAllPlcTimers(void)
{
    short int s,i;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(executing==INT)
    {
        s = 200;
    }
    else if(executing==EMR)
    {
        s = 100;
    }
    else
    {
        s = 0;
    }

    for(i = 0 ; i < 100 ; s++,i++)
    {
        if (clocks[s].active != 2) /* if not Hold mode*/
        {
            continue;
        }

        clocks[s].active = 1;
        clocks[s].start = clock_pcbplc();
    }

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_ResetCycle
 * Description  :   Restarts the logic from beginning.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ResetCycle()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    seqStep = seqStartNode;               // sequence start node
    emrStep = emrStartNode;               // Emr start node
    intStep = intStartNode;               // Int start node
    reset_flag = 1;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetTrans
 * Description  :   Gets some flag status.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetTrans()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    plcStack[stackIndex++] = transFlagMode;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_SetTrans
 * Description  :   Sets flag to zero(0).
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetTrans(void)
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    transFlagMode = 0;
    return(1);
}

/*****************************************************************************/
/*      ClearWaitState(): Clears the waiting state for a channel.			 */
/*****************************************************************************/
short int PcbPlc_ClearWaitState(void)
{
    short int i;

    if(gpcbplcCnfg.debug)
    printf("Func:%s executed\n", __func__);

    for(i = 0 ; i < totalBits ; ++i)
    {
        waitingFor[i]=dig_bit_array[i];
    }

    return(0);
}

/****************************************************************************
 * Function     :   PcbPlc_GetHour
 * Description  :   Get RTC hour.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetHour()
{
    plcStack[stackIndex++] = gTimeInfo.mHour;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetMinute
 * Description  :   Get RTC Minute.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetMinute()
{
    plcStack[stackIndex++] = gTimeInfo.mMinute;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetSecond
 * Description  :   Get RTC second.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetSecond()
{
    plcStack[stackIndex++] = (short int)gTimeInfo.mSecond;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetDay
 * Description  :   Get RTC Date.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetDay()
{
    plcStack[stackIndex++] = (short int)gTimeInfo.mDate;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetMonth
 * Description  :   Get RTC Month.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetMonth()
{
    plcStack[stackIndex++] = (short int)gTimeInfo.mMonth;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetYear
 * Description  :   Get RTC Year.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetYear()
{
    plcStack[stackIndex++] = (short int)gTimeInfo.mYear;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_GetWeekDay
 * Description  :   Get week day.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_GetWeekDay()
{
    plcStack[stackIndex++] = (short int)gTimeInfo.mDayofWeek;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_MakeTime
 * Description  :   Make time using hour, minute and second.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_MakeTime()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 3;
    plcStack[stackIndex] = (plcStack[stackIndex] * 3600) + (plcStack[stackIndex+1] * 60) + plcStack[stackIndex+2];
    stackIndex++;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_MakeDate
 * Description  :   Make date using date, month and year
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_MakeDate()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 3;
    plcStack[stackIndex] = plcStack[stackIndex] + (plcStack[stackIndex+1] * 30) + (plcStack[stackIndex+2] * 365);
    stackIndex++;
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_InPortB
 * Description  :   Set General purpose array of typedshort int.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_InPortB()
{
    short int Indx;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 1;
    Indx = (short int)plcStack[stackIndex];

    if ((Indx >= 5) && (Indx < MAX_PLCVAR)) // This validation is in RTU9000, RTU6000 it is not there.
    {
        plcStack[stackIndex] = (short int)plcUserArr[Indx];
        stackIndex++;
    }

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_OutPortB
 * Description  :   Get General purpose array of typedshort int.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_OutPortB()
{
    int Indx, Val;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 2;
    Indx = (short int)plcStack[stackIndex];
    Val = (short int)plcStack[stackIndex+1];

    if ((Indx >= 5) && (Indx < MAX_PLCVAR))
    {
        plcUserArr[Indx] = Val;
    }

    return(1);
}

short int PcbPlc_UserGetBit()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    #if 0 //Below commented part in RTU6000 is there, RTU9000 is not there.
    unsigned int Indx, BitVal, Val;
    char testtp[100];
    unsigned int temp=0;

    stackIndex -= 2;
    Val = (int)plcStack[stackIndex];
    Indx = (int)plcStack[stackIndex+1];

    sprintf(testtp,"\n %d %d ",Val,Indx);
    fnDebugMsg(testtp);

    BitVal=0;
    BitVal|= 1<<(Indx-1);

    if(Val>=5001 && Val<=5100)
    {
        if(FinalAnaValL[Val-5001]&BitVal)
        {
            plcStack[stackIndex++]  = 1;
        }
        else
        {
            plcStack[stackIndex++] = 0;
        }
    }
    else if(Val>=6001 && Val<=6100)
    {
        if(FinalAnaValS[Val-6001]&BitVal)
        {
            plcStack[stackIndex++] = 1;
        }
        else
        {
            plcStack[stackIndex++] = 0;
        }
    }
    #endif
    return (1);
}			 

short int PcbPlc_UserSetBit()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

short int PcbPlc_UserClrBit()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

short int PcbPlc_SetHiTimer()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

short int PcbPlc_LoadMimicFile()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

short int PcbPlc_PlaySoundFile()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_Plcabsolute
 * Description  :   Arithmatic operation - abs
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_Plcabsolute()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = abs((short int )plcStack[(stackIndex-1)-1]);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcCos
 * Description  :   Arithmatic operation - cos
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcCos()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];
    plcStack[stackIndex++] = (short int )var_index;

    plcStack[stackIndex++] = cos(plcStack[(stackIndex-1)-1]);

    #if 0
    //Below part in RTU6000 is there, RTU9000 is not there.
    Mod_PCBPLC.U_FC=(unsigned char)plcStack[(stackIndex-1)-1];
    Mod_PCBPLC.Modbus_W=1;
    #endif
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcSine
 * Description  :   Arithmatic operation - sin
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcSine()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];
    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = sin(plcStack[(stackIndex-1)-1]);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcTan
 * Description  :   Arithmatic operation - tan
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcTan()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = tan(plcStack[(stackIndex-1)-1]);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcAsine
 * Description  :   Arithmatic operation - asin
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcAsine()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = asin(plcStack[(stackIndex-1)-1]);

    #if 0
    //Below part in RTU6000 is there, RTU9000 is not there.
    Mod_PCBPLC.U_Address=(unsigned short int)plcStack[(stackIndex-1)-1];
    #endif
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcAcos
 * Description  :   Arithmatic operation - acos
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcAcos()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = acos(plcStack[(stackIndex-1)-1]);

    #if 0
    //Below part in RTU6000 is there, RTU9000 is not there.
    Mod_PCBPLC.UnitID= (unsigned char)plcStack[(stackIndex-1)-1];
    #endif

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcAtan
 * Description  :   Arithmatic operation - atan
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcAtan()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = atan(plcStack[(stackIndex-1)-1]);

    #if 0
    //Below part in RTU6000 is there, RTU9000 is not there.
    Mod_PCBPLC.U_Value=(float)plcStack[(stackIndex-1)-1];
    #endif
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcPower
 * Description  :   Write specified variable as per PCBPLC map
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcPower()
{
    //unsigned int var_index;
    unsigned int addr = 0;
    short int DI_num;
    //unsigned int i;
    //char tBuf[128] = {0,};


    addr = (unsigned int)plcStack[stackIndex-2];
    DI_num  = (short int)addr;

    //sprintf(tBuf, "Address(%d)->VALUE(%d)\r\n", DI_num, plcStack[stackIndex-1]);
    //WriteLog(pcbplc_logger, tBuf, LOG_INFO);

    /* Actual DO Write : 1 to 960 */
    if( (addr > 0) && (addr <= gpcbplcCnfg.mMaxDoEnabled) )
    {
        if((int)plcStack[stackIndex-1])
        {
            outputDOflag[addr-1] = 1;
        }
        else
        {
            outputDOflag[addr-1] = 0;
        }
    }
    /* General purpose analog array Write support */
    else if( (addr >= 961) && (addr < (961+MAX_GEN_ANA_PARA)) )	  //961+300=1261
    {
        if(gpcbplcCnfg.debug)
        {
            char tBuff[512] = {0,};
            sprintf(tBuff, "addr: %d, Value: %f\r\n", addr, plcStack[stackIndex - 1]);
            WriteLog(pcbplc_logger, tBuff, LOG_CRITICAL);
        }
        AnaOutLogic(addr-961+1, plcStack[stackIndex-1]);
    }
    else if( (addr >= 1301) && ((addr < (1301+(MODMAX_PARA*2)))) )//10501
    {
        if(1 == (DI_num%2))
        {
            DI_num = DI_num-1301;
            DI_num = DI_num/2;

            if( (DI_num >= 600) && (DI_num < 4600) )
            {
                /* TODO : Perform Logical action here. */
                ControlLogic (DI_num, plcStack[stackIndex-1]);
            }
        }
    }

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcSqrt
 * Description  :   Arithmatic operation - sqrt
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcSqrt()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = sqrt(plcStack[(stackIndex-1)-1]);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcLog
 * Description  :   Read specified variable as per PCBPLC map.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcLog()
{
    short int DI_num;
    //short int Do_Status = 0;
    float analog_OP;
    //char tBuf[128] = {0,};


    stackIndex -= 1;

    DI_num = (short int)plcStack[stackIndex];

    //sprintf(tBuf, "Address(%d)\n", DI_num);
    //WriteLog(pcbplc_logger, tBuf, LOG_INFO);

    /* DI-1 to 960 status get support */
    if( (DI_num > 0) && (DI_num <= 960) )
    {
    	plcStack[stackIndex++] = (unsigned char )!dig_bit_array[DI_num-1];
    }
    /* General purpose Analog variable get support */
    else if( (DI_num >= 961) && (DI_num <= (961 + MAX_GEN_ANA_PARA)) )	   //961+300=1261
    {
        analog_OP = signal_arr[DI_num-961];
        plcStack[stackIndex++] = analog_OP;
    }
    else if( (DI_num >= 1301) && (DI_num <= (1301 + (MODMAX_PARA * 2))) )
    {
        if( 1 == (DI_num % 2) )
        {
            DI_num = DI_num - 1301;
            DI_num = DI_num / 2;

            analog_OP = gFinalAnaValF[DI_num];
            plcStack[stackIndex++] = analog_OP;
        }
        else
        {
            plcStack[stackIndex++] = 0;
        }
    }

    //sprintf(tBuf, "%s: Address(%d), Value(%d)\r\n", __func__, DI_num, plcStack[stackIndex-1]);
    //WriteLog(pcbplc_logger, tBuf, LOG_INFO);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_PlcLog10
 * Description  :   Arithmatic operation - log10
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_PlcLog10()
{
    unsigned int var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    var_index = (short int )plcStack[stackIndex-2];

    plcStack[stackIndex++] = (short int )var_index;
    plcStack[stackIndex++] = log10(plcStack[(stackIndex-1)-1]);

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_stringlength
 * Description  :   Find String length
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_stringlength()
{
    unsigned short int string_index,var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(str_flag == 0)
    {
        string_index = (short int)(plcStack[stackIndex - 1]);
        var_index =  (short int)(plcStack[stackIndex - 2]);

        strcpy((char *)string1, (const char *)stringbuff[string_index]);
    }
    else
    {
        var_index =  (short int)(plcStack[stackIndex - 1]);
    }

    plcStack[stackIndex++] =  (float)var_index;
    plcStack[stackIndex++] = (float)strlen((const char *)string1);

    return(1);
} 

/****************************************************************************
 * Function     :   PcbPlc_Isnumeric
 * Description  :
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_Isnumeric()
{
    unsigned char flag_digit = 0,i = 0,string_index,var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(str_flag == 0)
    {
        string_index = (short int)(plcStack[stackIndex - 1]);
        var_index =  (short int)(plcStack[stackIndex - 2]);        

        strcpy((char *)string1,(const char *)stringbuff[string_index]);
    }
    else
    {
        var_index =  (short int)(plcStack[stackIndex - 1]);
    }

    while(string1[i] != '\0')
    {
        if( (string1[i] >= 0x30) && (string1[i] <= 0x39))
        {
            flag_digit = 1;
        }

        if(flag_digit == 0)
        {
            break;
        }
        i++;
    }

    plcStack[stackIndex++] = (float)var_index;
    plcStack[stackIndex++] = (float)flag_digit;

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_texttoval
 * Description  :   Text to value conversion
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_texttoval()
{
    unsigned short int string_index,var_index;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(str_flag == 0)
    {
        string_index = (short int)(plcStack[stackIndex - 1]);
        var_index =  (short int)(plcStack[stackIndex - 2]);

        strcpy((char *)string1,(const char *)stringbuff[string_index]);
    }
    else
    {
        var_index =  (short int)(plcStack[stackIndex - 1]);
    }

    plcStack[stackIndex++] = (float) var_index;
    plcStack[stackIndex++] = atof((const char *)string1);

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_assignstring
 * Description  :
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_assignstring()
{
    unsigned char index_string = 0;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    index_string = (short int)plcStack[stackIndex-1];

    strcpy((char *)stringbuff[index_string],(const char *)string1);
    plcStack[stackIndex++] = index_string;

    return 1;
}

/****************************************************************************
 * Function     :   PcbPlc_strmid
 * Description  :   Text to value conversion
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_strmid()
{
    unsigned short int string_pos,length,i=0,index_string,passed_string;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    index_string    = (short int)plcStack[stackIndex-4];
    passed_string   = (short int)plcStack[stackIndex-3];
    string_pos  =(short int)plcStack[stackIndex-2];
    length  = (short int)plcStack[stackIndex-1];
    string_pos  = string_pos - 1;

    strcpy((char *)string1,(const char *)stringbuff[passed_string]);

    for(i=0;i<length;i++)
    {
        stringbuff[index_string][i] = string1[string_pos++];
    }

    plcStack[stackIndex++] = (float)index_string;
    plcStack[stackIndex++] = (float)index_string;

    return 1;
}

/****************************************************************************
 * Function     :   PcbPlc_strinstring
 * Description  :
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_strinstring()
{
    unsigned char start_pos,index_string;
    unsigned char mainstr,substr,i=0,first_occ,string_match_flag;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    index_string =  (short int)plcStack[stackIndex-4];
    start_pos    =  (short int)plcStack[stackIndex-3];
    mainstr 	 =  (short int)plcStack[stackIndex-2];
    substr	 =  (short int)plcStack[stackIndex-1];
    start_pos    = start_pos - 1;

    while(stringbuff[mainstr][start_pos] != '\0')
    {
        if(stringbuff[mainstr][start_pos] == stringbuff[substr][i])
        {
            first_occ = start_pos+1;

            while(stringbuff[substr][i] != '\0')
            {
                if(stringbuff[mainstr][start_pos] == stringbuff[substr][i])
                {
                    string_match_flag = 1;
                }
                else
                {
                    string_match_flag = 0;
                }
                i++;
                start_pos++;
            }
            i = 0;
            if(string_match_flag == 1)
            {
                break;
            }
        }
        start_pos++;
    }


    if(string_match_flag == 1)
    {
        plcStack[stackIndex++] = (float)index_string;
        plcStack[stackIndex++] = (float)first_occ;
    }
    else
    {
        plcStack[stackIndex++] = (float)index_string;
        plcStack[stackIndex++] = 0.0;
    }

    return 1;
}

/****************************************************************************
 * Function     :   PcbPlc_stringsplit
 * Description  :
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_stringsplit()
{
    unsigned char index_string;
    unsigned char mainstr,substr,i=0,j=0,first_occ=0,string_match_flag,stringindex;
    unsigned char copy_start=0,copy_end=0,start_pos=0;

    if(gpcbplcCnfg.debug)
    printf("Func:%s executed\n", __func__);

    index_string =  (short int)plcStack[stackIndex-4];
    mainstr 	= (short int)plcStack[stackIndex-3];
    substr 	= (short int)plcStack[stackIndex-2];
    stringindex = (short int)plcStack[stackIndex-1];

    copy_end = strlen((const char *)stringbuff[mainstr]);

    while(stringbuff[mainstr][start_pos] != '\0')
    {
        if(stringbuff[mainstr][start_pos] == stringbuff[substr][i])
        {
            while(stringbuff[substr][i] != '\0')
            {
                if(stringbuff[mainstr][start_pos] == stringbuff[substr][i])
                {
                    string_match_flag = 1;
                }
                else
                {
                    string_match_flag = 0;
                }
                i++;
                start_pos++;
            }
            i = 0;
            if(string_match_flag == 1)
            {
                first_occ++;
                if(first_occ == stringindex)
                {
                    copy_start = start_pos;
                }
                if(first_occ == (stringindex + 1))
                {
                    copy_end  = start_pos - ((strlen((const char *)stringbuff[substr])+1));
                }
                string_match_flag = 0;
            }
        }
        start_pos++;
    }

    for(i=copy_start;i<=copy_end;i++)
    {
        stringbuff[index_string][j++] = stringbuff[mainstr][i];
    }
    stringbuff[index_string][j] = '\0';

    plcStack[stackIndex++] = (float)index_string;
    plcStack[stackIndex++] =  (float)index_string;

    return 1;
}

/****************************************************************************
 * Function     :   PcbPlc_CheckAutoManual
 * Description  :   Get RTU mode - Manual, Scheduled etc...
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_CheckAutoManual()
{
//    if(gpcbplcCnfg.debug)
//    {
//        printf("Func:%s executed\n", __func__);
//    }
	plcStack[stackIndex++] = EPROM_General.DoModeDetails.Do_Mode;

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SetAutoMode
 * Description  :   Set RTU in auto mode
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetAutoMode()
{
    short int i;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(RTU_DO_MODE_MANUAL == EPROM_General.DoModeDetails.Do_Mode) // if in manual mode
    {
        for(i = 0 ; i < gpcbplcCnfg.mMaxDoEnabled; i++)
        {
            outputDOflag[i] = 0;
            //set_do_off(i);
        }
    }

    gpcbplcCnfg.mRtuDoMode = RTU_DO_MODE_AUTO;
    EPROM_General.DoModeDetails.Do_Mode = RTU_DO_MODE_AUTO;
    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SetManualMode
 * Description  :   Set manual mode.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetManualMode()
{
    short int i;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    if(RTU_DO_MODE_AUTO == EPROM_General.DoModeDetails.Do_Mode) // if in auto mode
    {
        for(i = 0; i < gpcbplcCnfg.mMaxDoEnabled; ++i)
        {
            outputDOflag[i] = 0;
           // set_do_off(i);
        }
    }

    gpcbplcCnfg.mRtuDoMode = RTU_DO_MODE_MANUAL;
    EPROM_General.DoModeDetails.Do_Mode = RTU_DO_MODE_MANUAL;
    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_BuildSms
 * Description  :   Buid data in SMS to send
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_BuildSms()
{
    unsigned int index=0;
    unsigned char arg_no = 0, i=0, j=0;
    short int len=0,ind_string;
    unsigned char *strptr;
    char str[100] = {0, };

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    strptr = string1;

    while(*strptr)
    {
        if(*strptr == ESCAP)
        {
            arg_no++;
        }
        strptr++;
    }

    index = 0;
    i = 0;
    strptr = string1;

    while(*strptr != '\0')
    {
        if(*strptr == ESCAP)
        {
            strptr++;
            str[i]='\0';

            switch (*strptr)
            {
                case 'd':
                case 'D':
                {
                    len = sprintf((char *)temparr,"%d",(short int)plcStack[(stackIndex - arg_no) + index]);
                    strcat(( char *)str,(const char *)temparr);
                    break;
                }
                case 'a':
                case 'A':
                {
                    len = sprintf((char *)temparr,"%3.2f",(float)plcStack[(stackIndex - arg_no) + index]);
                    strcat(( char *)str,(const char *)temparr);
                    break;
                }
                case 'r':
                case 'R':
                {
                    len = sprintf((char *)temparr,"%6.2f",(float)plcStack[(stackIndex - arg_no) + index]);
                    strcat(( char *)str,(const char *)temparr);
                    break;
                }
                case 't':
                case 'T':
                {
                    len = sprintf((char *)temparr,"%6.2f",plcstringArr[(int)plcStack[(stackIndex - arg_no) + index]]);
                    strcat((char *)str,(const char *)temparr);
                    break;
                }
                default:
                {
                    break;
                }
            }

            i += len;
            strptr++;
            index++;
        }
        else
        {
            str[i++] = *strptr;
            strptr++;
        }
    }

    str[i] = '\0';

    ind_string = plcStack[(stackIndex - arg_no) - 1];

    i = 0;
    j = 0;
    while(str[i])
    {
        if(str[i] != 39)
        {
            stringbuff[ind_string][j]= str[i];
            j++;
        }
        i++;
    }

    stringbuff[ind_string][j] = '\0';

    plcStack[stackIndex++]  = (float)ind_string ;
    plcStack[stackIndex++]  = (float)ind_string ;

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SendData
 * Description  :   Send data on log rate, trigger by pcbplc logic
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SendData()
{
//    char tDataBuf[1024] = {0,};
    if(gpcbplcCnfg.debug)
    {
        char logBuff[512] = {0,};
        sprintf(logBuff,"Func:%s executed\r\n", __func__);
        WriteLog(pcbplc_logger, logBuff, LOG_INFO);
    }

    Build_Data_for_server();
    //LoraPublish();
    //if(!Build_Data_for_server())
    {
    	flagMqttPubLogData = 1;
    	flagLORAPubLogData = 1;
        //send_event_string(tDataBuf);
        //send_data_string(tDataBuf);
    }
//    else
//    {
//        WriteLog(pcbplc_logger, "Data Sent on LogRate failed due to payload is not framed\r\n", LOG_CRITICAL);
//    }

    //Build_Data(tDataBuf);
    //send_data_string(tDataBuf);

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SendDataInSMS
 * Description  :   Send data in SMS
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SendDataInSMS()
{
//    char tempBuf[128] = {0, };
//    char tFinalBuffer[512] = {0,};
//    char tDataBuf[512] = {0,};
//    char tBuffer[512] ={0, };
//
//    if(gpcbplcCnfg.debug)
//        printf("Func:%s executed\n", __func__);
//
//    Build_Data(tDataBuf);
//
//    for(int tAlarmIdx = 0 ; tAlarmIdx < MAXALARM ; ++tAlarmIdx)
//    {
//        if(2 != gpcbplcCnfg.mAlarm[tAlarmIdx].mType) //Enabled for Alarm
//        {
//            continue;
//        }
//
//        sprintf(tFinalBuffer, "%s@%s\r\n", (char*)gpcbplcCnfg.mAlarm[tAlarmIdx].mobileNo, (char*)tDataBuf);
//        WriteLog(pcbplc_logger, tFinalBuffer, LOG_INFO);
//
//        if(destination_list)
//        {
//            //for(int tIndex = 0; destination_list[tIndex]; ++tIndex)
//            for(int tIndex = 0; tIndex < gpcbplcCnfg.mNumOfDest; ++tIndex)
//            {
//                printf("%s\n", destination_list[tIndex]);
//
//                if (!message_bus_send(message_bus, destination_list[tIndex], Data, SMSText, (char *) tFinalBuffer, (long) strlen(tFinalBuffer)))
//                {
//                    WriteLog(pcbplc_logger, "Send Data SMS failure over IPC\r\n", LOG_ERROR);
//                }
//            }
//        }
//    }

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_Analog_output_Plc
 * Description  :
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_Analog_output_Plc()
{
    float tAoPercentage;
    unsigned int tAoChannel;

    if(gpcbplcCnfg.debug)
    {
        WriteLog(pcbplc_logger, " Func PcbPlc_Analog_output_Plc executed\r\n", LOG_INFO);
    }

    tAoChannel = (unsigned int)plcStack[stackIndex-2];
    tAoPercentage = (float)plcStack[stackIndex-1];

    //printf("tAoChannel:%d and tAoPercentage:%f\n", tAoChannel, tAoPercentage);
    if((tAoChannel > 0) && (tAoChannel <= gpcbplcCnfg.mMaxAoEnabled) )
    {
        gAoValue[tAoChannel-1] = tAoPercentage;
        set_ao(tAoChannel-1, tAoPercentage);
    }
    else if((tAoChannel >= 5) && (tAoChannel <= 12)) //Do Key status set and clear
    {
        if((0 == tAoPercentage) || (1 == tAoPercentage))
        {
            if(1 == tAoPercentage)
            {
                outputDOflag[tAoChannel-5] = 1;
                gpcbplcCnfg.mDoStatus[tAoChannel-5] = 1;
                EPROM_General.DoModeDetails.DO_Value[tAoChannel-5] = 1;
            }
            else
            {
                outputDOflag[tAoChannel-5] = 0;
                gpcbplcCnfg.mDoStatus[tAoChannel-5] = 0;
                EPROM_General.DoModeDetails.DO_Value[tAoChannel-5] = 0;
            }

            if(gpcbplcCnfg.mDoStatus[tAoChannel-5] != gpcbplcCnfg.mOldDoStatus[tAoChannel-5])
            {
                gpcbplcCnfg.mOldDoStatus[tAoChannel-5] = gpcbplcCnfg.mDoStatus[tAoChannel-5];
                update_do_status_key(tAoChannel-5+1, gpcbplcCnfg.mDoStatus[tAoChannel-5]);
            }
            flag_flashUpdateEPROM_General = 1;
            flag_flashUpdateEPROM_General_WaitCounter = 5;
        }

    }
    #if 0
    unsigned short Digi_Ch,ana_ch11,ana_per11;
    ana_per11   =   (unsigned short)plcStack[stackIndex-1];
    ana_ch11 	=   (unsigned short)plcStack[stackIndex-2];
    if(ana_ch>=3201 && ana_ch<=(4000)) // due to maximum add
    {
        if(ana_per11==0 || ana_per11==1)
        {
            if(ana_per11==1)
            {
                VDO_Status[ana_ch11-3201]=1;	   //set Dummay DI for 2100
            }
            else
            {
                VDO_Status[ana_ch11-3201]=0;
            }
        }
    }
    else if(ana_ch>4000 && ana_ch<=(4000+MAX_VAO))
    {
        EPROM.VAO_VALUE[ana_ch-4001]=ana_per;	   //set Dummay DI for 2100
    }
    else //if(ana_ch11>=(2101+960) && ana_ch11<(2101+960+960))	   //Dummmay Di generate from PCNPLC
    {
        if(ana_per11==0 || ana_per11==1)
        {
            if(ana_per11==1)
            {
                EPROM.DOSignal[ana_ch11-1]=1;	   //set Dummay DI for 2100
            }
            else
            {
                EPROM.DOSignal[ana_ch11-1]=0;
            }
        }
    }
    #endif
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_Get_sch_time
 * Description  :   Get schedule time in minutes
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_Get_sch_time()
{
    short int tSchNo = 0;
    unsigned short int tSch_stop_min = 0;
    unsigned short int tSch_start_min = 0;
    unsigned short int tSch_min = 0;
    unsigned char tBuffer[128] = {0, };


    stackIndex -= 1;
    tSchNo = (short int) plcStack[stackIndex];

//    if(RTU_DO_MODE_MANUAL != EPROM_General.DoModeDetails.Do_Mode )
//    {
//        if((tSchNo >= 1) && (tSchNo <= MAXSCH))
//        {
//            tSchNo = tSchNo-1;
//
//            if(gpcbplcCnfg.sSch[tSchNo].mIsSchEnabled)
//            {
//                tSch_start_min  = ((gpcbplcCnfg.sSch[tSchNo].mStartHour) * 60) + gpcbplcCnfg.sSch[tSchNo].mStartMin;
//                tSch_stop_min   = ((gpcbplcCnfg.sSch[tSchNo].mStopHour)  * 60) + gpcbplcCnfg.sSch[tSchNo].mStopMin;
//
//                tSch_min =  tSch_stop_min - tSch_start_min;
//            }
//        }
//        else
//        {
//            sprintf((char *)tBuffer, "Schedule(%d) not allow more that %d\r\n", tSchNo, MAXSCH);
//            WriteLog(pcbplc_logger,(const char *) tBuffer, LOG_INFO);
//        }
//
//    }

   // if(RTU_DO_MODE_MANUAL != EPROM_General.DoModeDetails.Do_Mode ) // 1.0.0.12
    if(RTU_DO_MODE_AUTO == EPROM_General.DoModeDetails.Do_Mode )
    {
        if((tSchNo >= 1) && (tSchNo <= MAXSCH))
        {
            tSchNo = tSchNo-1;

            if(EPROM_Schedule.Schedule[tSchNo].Sch_En_Di)
            {
                tSch_start_min  = ((EPROM_Schedule.Schedule[tSchNo].Start_HH) * 60) + EPROM_Schedule.Schedule[tSchNo].Start_Min;
                tSch_stop_min   = ((EPROM_Schedule.Schedule[tSchNo].Stop_HH)  * 60) + EPROM_Schedule.Schedule[tSchNo].Stop_Min;

                tSch_min =  tSch_stop_min - tSch_start_min;
            }
        }
        else
        {
            sprintf((char *)tBuffer, "Schedule(%d) not allow more that %d\r\n", tSchNo, MAXSCH);
            WriteLog(pcbplc_logger,(const char *) tBuffer, LOG_INFO);
        }

    }

    plcStack[stackIndex++] = (short int)tSch_min;

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_DO_Key_Status
 * Description  :   todo : need to add description
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_DO_Key_Status()
{
    short int Do_num;
    short int tDoStatus = 0;
    //float analog_OP;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 1;

    Do_num = (short int) plcStack[stackIndex];

    if( (Do_num >= 1) && (Do_num <= MAXDO))
    {
        //tDoStatus = gpcbplcCnfg.mDoStatus[Do_num-1];
    	tDoStatus = EPROM_General.DoModeDetails.DO_Value[Do_num-1];
        plcStack[stackIndex++] = (short int)tDoStatus;
    }
    return 0;
}

/****************************************************************************
 * Function     :   PcbPlc_DummyHandler
 * Description  :   Dummy function.
 * arg          :   none
 * return       :   none
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_DummyHandler(void)
{
    if(gpcbplcCnfg.debug)
        printf("Executed %s\n", __func__);

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SendAlarm
 * Description  :
 * arg          :   none
 * return       :   none
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SendAlarm(void)
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    return (1);
}

/****************************************************************************
 * Function     :   PcbPlc_SetDO
 * Description  :   Set DO ON/OFF.
 * arg          :   none
 * return       :   none
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetDO()
{
    int tDOIndex = 0;
    int tValue = 0;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex-=2;

    tDOIndex = (short int)plcStack[stackIndex];
    tValue = (short int)plcStack[stackIndex+1];

    if(tDOIndex > 0)
    {
        tDOIndex = tDOIndex - 1;
    }

    if(tValue > 0)
    {
		outputDOflag[(short int)tDOIndex] = 1;
    }
    else
    {
        outputDOflag[(short int)tDOIndex] = 0;
    }

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_ChkSchedule
 * Description  :   It checks whether schedule is running or not.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_ChkSchedule(void)
{
    short int tSchNo;
    short int tPmpOnStatus = 0;

    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);

    stackIndex -= 1;

    tSchNo = (short int) plcStack[stackIndex];

    //if(RTU_DO_MODE_MANUAL != EPROM_General.DoModeDetails.Do_Mode ) //1.0.0.12
    if(RTU_DO_MODE_AUTO == EPROM_General.DoModeDetails.Do_Mode )
    {
        if((tSchNo >= 1) && (tSchNo <= MAXSCH))
        {
            tSchNo = tSchNo-1;

            tPmpOnStatus = 0;

//            if(EPROM_Schedule.Schedule[tSchNo].Sch_En_Di)
//            {
//                if((gTimeInfo.mHour >= gpcbplcCnfg.sSch[tSchNo].mStartHour) &&
//                        (gTimeInfo.mHour <= gpcbplcCnfg.sSch[tSchNo].mStopHour))
//                {
//                    tPmpOnStatus = 1;
//                }
//
//                if((gTimeInfo.mHour == gpcbplcCnfg.sSch[tSchNo].mStartHour) &&
//                        (gTimeInfo.mMinute < gpcbplcCnfg.sSch[tSchNo].mStartMin))
//                {
//                    tPmpOnStatus = 0;
//                }
//
//                if((gTimeInfo.mHour == gpcbplcCnfg.sSch[tSchNo].mStopHour) &&
//                        (gTimeInfo.mMinute >=gpcbplcCnfg.sSch[tSchNo].mStopMin))
//                {
//                    tPmpOnStatus = 0;
//                }
//            }

#if 0
			if(EPROM_Schedule.Schedule[tSchNo].Sch_En_Di)// Old RTU schedule as per last release v1.0.0.13
			{
				if((gTimeInfo.mHour >= EPROM_Schedule.Schedule[tSchNo].Start_HH) &&
						(gTimeInfo.mHour <= EPROM_Schedule.Schedule[tSchNo].Stop_HH))
				{
					tPmpOnStatus = 1;
				}

				if((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Start_HH) &&
						(gTimeInfo.mMinute < EPROM_Schedule.Schedule[tSchNo].Start_Min))
				{
					tPmpOnStatus = 0;
				}

				if((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Stop_HH) &&
						(gTimeInfo.mMinute >=EPROM_Schedule.Schedule[tSchNo].Stop_Min))
				{
					tPmpOnStatus = 0;
				}
			}
#endif
			if(EPROM_Schedule.Schedule[tSchNo].Sch_En_Di) // New Logic for 23:59 Midnight time schedule
			{
				if(((EPROM_Schedule.Schedule[tSchNo].Stop_HH) == 23) && ((EPROM_Schedule.Schedule[tSchNo].Stop_Min) == 59))
				{
					if ((gTimeInfo.mHour > EPROM_Schedule.Schedule[tSchNo].Start_HH) ||
						((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Start_HH) &&
						(gTimeInfo.mMinute >= EPROM_Schedule.Schedule[tSchNo].Start_Min)))
					{
						// Current time is equal to or after the start time
						if ((gTimeInfo.mHour < EPROM_Schedule.Schedule[tSchNo].Stop_HH) ||
							((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Stop_HH) &&
							 (gTimeInfo.mMinute <= EPROM_Schedule.Schedule[tSchNo].Stop_Min)))
						{
							// Current time is before or equal to the stop time
							tPmpOnStatus = 1;
						}
						else
						{
							tPmpOnStatus = 0;
						}
					}
					else
					{
						tPmpOnStatus = 0;
					}
				}
				else
				{
					if ((gTimeInfo.mHour > EPROM_Schedule.Schedule[tSchNo].Start_HH) ||
						((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Start_HH) &&
						 (gTimeInfo.mMinute >= EPROM_Schedule.Schedule[tSchNo].Start_Min)))
					{
						// Current time is equal to or after the start time
						if ((gTimeInfo.mHour < EPROM_Schedule.Schedule[tSchNo].Stop_HH) ||
							((gTimeInfo.mHour == EPROM_Schedule.Schedule[tSchNo].Stop_HH) &&
							 (gTimeInfo.mMinute < EPROM_Schedule.Schedule[tSchNo].Stop_Min)))
						{
							// Current time is before or equal to the stop time
							tPmpOnStatus = 1;
						}
						else
						{
							tPmpOnStatus = 0;
						}
					}
					else
					{
						tPmpOnStatus = 0;
					}
				}
			}
        }
    }

    plcStack[stackIndex++] = (short int)tPmpOnStatus;

    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_SetPValue
 * Description  :   Sets P value of the pid loop. Now This function Checks
 * the Time P Value and generates the Daily SMS. It is also part of plc file
 * execution.
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SetPValue()
{
    if(gpcbplcCnfg.debug)
        printf("Func:%s executed\n", __func__);
    return(1);
}

/****************************************************************************
 * Function     :   PcbPlc_SendAlarmSMS
 * Description  :   Send Alarm Data in SMS
 * arg          :   none
 * return       :   1
 * Remark       :   none
 * **************************************************************************/
short int PcbPlc_SendAlarmSMS()
{
//    if(gpcbplcCnfg.debug)
//        printf("Func:%s executed\n", __func__);
//
//    unsigned char tBuffer[512] = {0,};
//    unsigned char tFinalBuffer[512] = {0, };
//    unsigned char tAlarmBuffer[512] = {0, };
//    unsigned char tempBuf[128] = {0, };
//    unsigned int string_index,mobile_index;
//
//    mobile_index = (unsigned int)plcStack[stackIndex - 1];
//    string_index = (unsigned int)plcStack[stackIndex - 2];
//
//    if(strlen(gpcbplcCnfg.mSiteName))
//    {
//        sprintf(tAlarmBuffer,"Site: %s\n",gpcbplcCnfg.mSiteName);
//    }
//
//    sprintf(tempBuf,"UNIT:%02d\n",(int)gpcbplcCnfg.mRtuAddr);
//    strcat(tAlarmBuffer,tempBuf);
//
//    if(1 == str_flag)
//    {
//        strcat(tAlarmBuffer, string1);
//    }
//    else
//    {
//        strcat(tAlarmBuffer, stringbuff[string_index]);
//    }
//
//    sprintf(tempBuf, "\nTime:%02d/%02d/%02d %02d:%02d:%02d ",
//                        (int)gTimeInfo.mDate,(int)gTimeInfo.mMonth,(int)gTimeInfo.mYear,
//                        (int)gTimeInfo.mHour,(int)gTimeInfo.mMinute,(int)gTimeInfo.mSecond);
//    strcat(tAlarmBuffer, tempBuf);
//
//    sprintf(tBuffer, "alarm msg: \n%s\r\n", tAlarmBuffer);
//    WriteLog(pcbplc_logger, tBuffer, LOG_INFO);
//
//    for(int tAlarmIdx = 0 ; tAlarmIdx < MAXALARM ; ++tAlarmIdx)
//    {
//        if(1 != gpcbplcCnfg.mAlarm[tAlarmIdx].mType) //Enabled for Alarm
//        {
//            continue;
//        }
//
//        sprintf(tFinalBuffer, "%s@%s\r\n", gpcbplcCnfg.mAlarm[tAlarmIdx].mobileNo, (char*)tAlarmBuffer);
//        WriteLog(pcbplc_logger, tFinalBuffer, LOG_INFO);
//
//        if(destination_list)
//        {
//            //for(int tIndex = 0 ; destination_list[tIndex]; ++tIndex)
//            for(int tIndex = 0 ; tIndex < gpcbplcCnfg.mNumOfDest; ++tIndex)
//            {
//                if (!message_bus_send(message_bus, destination_list[tIndex], Data, SMSText, (char *) tFinalBuffer, (long) strlen(tFinalBuffer)))
//                {
//                    WriteLog(pcbplc_logger, "Send SMS failure over IPC\r\n", LOG_ERROR);
//                }
//            }
//        }
//    }

    return(1);
}

/**********************************************************************************
 ************************ End of PLC LOGIC Functions ******************************
 **********************************************************************************/

/*****************************************************************************
* main parser function for plcfile execution.
* This function will parse the plcfile string and takes appropriate actions.
*****************************************************************************/
float PcbPlc_Logic_Interpreter(unsigned char *str1,short int execute_flag)
{
    unsigned char *atr = NULL;
    int flag = 1;
    int totalParam = 0;
    unsigned char j;

    if(NULL == str1)
    {
        WriteLog(pcbplc_logger, "str1 is null\r\n", LOG_ERROR);
        return 0;
    }

    gsLastCommand = 0;
    str_flag = 0;
    //plcStack is the array of 100 floats
    strcpy((char *)str,(const char *)str1);               // Copy data received from PC to other array str
    atr = str;

    stackIndex = 0;

    while(*atr)
    {
        /* '0' to '9' or '.' */
        if( (((*atr) >= '0') && ((*atr) <= '9')) || ((*atr) == '.') )
        {
            if( (*(atr-1)) == '-')	//Minus sign	// Some problem may be
            {
                plcStack[stackIndex++] = atof((const char *)atr-1);
            }
            else
            {
                plcStack[stackIndex++] = atof((const char *)atr);
            }

            //    changed at 8/4/08. for inserting negetive value
            while(isdigit(*atr) || (*atr=='.') || (*atr == '-'))
            {
                // First take all digits and .
                atr++;
            }
            continue;
        }

        flag = 1;

        switch(*atr)
        {
            case 0x23: /*'#'*/
            {
                break;
            }
            case 0x3B: /*';'*/
            {
                stackIndex = 0;
                break;
            }
            case 0x21: /*'!'*/
            {
                plcStack[stackIndex-1] = !plcStack[stackIndex-1];
                break;
            }
            case 0x2B: /*'+'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] + plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x2D: /*'-'*/
            {
                if(*(atr+1) == '#')		   // changed at 8/4/08.   for inserting negetive value
                {
                    plcStack[stackIndex-2] = plcStack[stackIndex-2] - plcStack[stackIndex-1];
                    stackIndex--;
                }
                break;
            }
            case 0x2A: /*'*'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] * plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x2F: /*'/'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] / plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x3C: /*'<'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] < plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x3E: /*'>'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] > plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x26: /*'&'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] && plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case 0x7C: /*'|'*/
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] || plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case GREATER_EQUAL_TO_CODE :
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] >= plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case LESS_EQUAL_TO_CODE :
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] <= plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case EQUAL_TO_CODE :
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] == plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case NOT_EQUAL_TO_CODE :
            {
                plcStack[stackIndex-2] = plcStack[stackIndex-2] != plcStack[stackIndex-1];
                stackIndex--;
                break;
            }
            case STRING_START_CODE:
            {
                /* 	On receiving string start code (0x02), it will collect charactor till
                                    string end code receive which is (0x03).
                                    e.g. 02 56 20 31 2E 30 2E 30 03 23 98
                                            02 - String start code
                                            56 20 31 2E 30 2E 30 - String charactor.
                                            03 - End of string code
                                            23 - #
                                            98 - Received string display on LCD.
                            */
                j = 1;
                while(*(atr+j) != 3) // 3 Means End	of text
                {
                    *(string1 + j - 1) = *(atr+j);
                    j++;
                }

                *(string1+j-1) = '\0';
                str_flag = 1;
                atr = atr + strlen((const char *)string1) + 2;
                break;
            }
            case GET_DIG_CODE :                 // gets status of requested digital signal
            {
                plcStack[stackIndex-1] = getBit((short int)plcStack[stackIndex-1]);
                break;
            }
            case GET_ANA_CODE :                   // gets status of requested AI
            {
                plcStack[stackIndex-1] = signal_arr[(short int)plcStack[stackIndex-1]];
                break;
            }
            case GET_VAR_CODE:
            {
                plcStack[stackIndex-1] = plcVarArr[(short int)plcStack[stackIndex-1]];
                break;
            }
            case GET_STRING_CODE:
            {
                plcStack[stackIndex-1] = plcstringArr[(short int)plcStack[stackIndex-1]];
                break;
            }
            case GET_STACK_VAR_CODE:
            {
                getStackVar(paramSp,(short int)plcStack[stackIndex-1]);
                break;
            }
            case CALL_CODE :
            {
                if(executing==SEQ)
                {
                    stackIndex--;

                    paramStack[paramSp++] = totalParam;
                    totalParam = 0;
                    paramStack[paramSp++] = plcDataFile[seqStep].yes;

                    seqStep = (short int)plcStack[stackIndex];
                    paramStack[0]++;

                    strcpy((char *)str,(const char *)plcDataFile[seqStep].str);
                    atr = str;
                    stackIndex = 0;
                    flag = 0;
                }
                else if(executing == EMR)
                {
                    stackIndex--;

                    paramStack[paramSp++] = totalParam;
                    totalParam = 0;

                    paramStack[paramSp++] = plcDataFile[emrStep].yes;

                    emrStep = (short int)plcStack[stackIndex] + maxSeqNode;

                    paramStack[299]++;

                    strcpy((char *)str,(const char *) plcDataFile[emrStep].str);
                    atr = str;
                    stackIndex = 0;
                    flag = 0;
                }
                else
                {
                    unsigned char *p1;

                    stackIndex--;

                    paramStack[paramSp++] = totalParam;
                    totalParam = 0;
                    paramStack[paramSp++] = plcDataFile[intStep].yes;

                    intStep = (short int)plcStack[stackIndex]+maxSeqNode+maxEmrNode;
                    paramStack[2]++;

                    strcpy((char *)str,(const char *)plcDataFile[intStep].str);
                    atr = str;
                    p1 = strchr((const char*)atr,(int)';');
                    if(p1) *p1='\x0';
                    stackIndex = flag = 0;
                }
                break;
            }
            case PUT_IN_DIG_CODE:
            case PUT_IN_ANA_CODE:
            case PUT_IN_VAR_CODE:
            case PUT_IN_STACK_CODE:
            case PUT_IN_STRING_CODE:						  //change by samir for dual arg string
            {
                gsLastCommand = (*atr);
                break;
            }
        case 0x3D: /*'='*/
        {
            stackIndex--;
            switch(gsLastCommand)
            {
                case PUT_IN_DIG_CODE:
                {
                    if((short int)plcStack[stackIndex])
                    {
                        setBit((short int)plcStack[stackIndex-1]);
                        if(execute_flag)
                        {
                            outputDOflag[(short int)plcStack[stackIndex-1]] = 1;
                        }
                    }
                    else
                    {
                        clrBit((short int)plcStack[stackIndex-1]);
                        if(execute_flag)
                        {
                            outputDOflag[(short int)plcStack[stackIndex-1]] = 0;
                        }
                    }
                    break;
                }
                /* Analog variables updates - General purpose */
                case PUT_IN_ANA_CODE:
                {
                    AnaOutLogic((short int)plcStack[stackIndex-1] + 1 , plcStack[stackIndex]);
                    gsLastCommand = 0;
                    break;
                }
                /* PLC (REC) variables updates */
                case PUT_IN_VAR_CODE:
                {
                    plcVarArr[(short int)plcStack[stackIndex-1]] = plcStack[stackIndex];
                    gsLastCommand = 0;
                    break;
                }
                case PUT_IN_STACK_CODE://;				channge by samir for the dual string
                {
                    putInStack(execute_flag,paramSp,(short int)plcStack[stackIndex-1]);
                    gsLastCommand = 0;
                    break;
                }
                case PUT_IN_STRING_CODE:					 //    add for string var
                {
                    plcstringArr[(short int)plcStack[stackIndex-1]] = plcStack[stackIndex];
                    gsLastCommand = 0;
                    break;
                }
                default:
                {
                    break;
                }
            }
            break;
        }
        case PUSH_2:
        {
            stackIndex-=2;
            paramStack[paramSp++] = plcStack[stackIndex];
            paramStack[paramSp++] = plcStack[stackIndex+1];
            totalParam+=2;
            break;
        }
        default :
        {
            if((*atr >= PLC_FUNCTION_CODE)&&(*atr < PLC_FUNCTION_CODE+89))
            {
                (*FnPtr[*atr-PLC_FUNCTION_CODE])();
            }
            break;
        }
        }// Switch statement end

        if(flag)
        {
            atr++;
        }
    }

    return(plcStack[--stackIndex]);
}

void setBit(unsigned short int bitNo)
{
    if(bitNo > MAX_DIG_CHNS)
    {
        char tBuffer[256] = {0,};

        sprintf(tBuffer, "bitNo(%d) is invalid\r\n", bitNo);
        WriteLog(pcbplc_logger, tBuffer, LOG_CRITICAL);
    }

    if(executing==SEQ)
    {
        if((modifiedArr[bitNo] > 1) && (outBits[bitNo]==0))
        {
            conflictArr[bitNo] = 1;
        }
        else
        {
            modifiedArr[bitNo] = 1;
            outBits[bitNo] = 1;
        }
    }
    else if(executing == EMR)
    {
        if(modifiedArr[bitNo] != 3)
        {
            modifiedArr[bitNo]	=	2;
            outBits[bitNo]	=	1;
        }
    }
    else
    {
        modifiedArr[bitNo]	=	3;
        outBits[bitNo]	=	1;
    }
}

short int getBit(unsigned short int bitNo)
{
    if(feedbackArr[bitNo])
    {
        waitingFlag++;
    }

    return(!dig_bit_array[bitNo]);
}

void clrBit(unsigned short int bitNo)
{
    if(executing==SEQ)
    {
        if((modifiedArr[bitNo]>1)&&(outBits[bitNo]==1))
        {
            conflictArr[bitNo]=1;
        }
        else
        {
            modifiedArr[bitNo]=1;
            outBits[bitNo] = 0;
        }
    }
    else if (executing==EMR)
    {
        modifiedArr[bitNo]=2;
        outBits[bitNo] = 0;
    }
    else
    {
        modifiedArr[bitNo]=3;
        outBits[bitNo] = 0;
    }
}

void putInStack(short int execute_flag,short int stkPtr,short int offset)
{
    short int type,addr,index,bp;

    bp = stkPtr-paramStack[stkPtr-2]-2;
    index = bp+offset*2;
    type = paramStack[index];
    addr = paramStack[index+1];

    switch(type)
    {
    case PROC_DIGITAL_VAR:
    {
        if((int)plcStack[stackIndex])
        {
            setBit(addr);
            if(execute_flag)
            {
                outputDOflag[addr] = 1;
            }
        }
        else
        {
            clrBit(addr);
            if(execute_flag)
            {
                outputDOflag[addr] = 0;
            }
        }
        break;
    }
    case PROC_ANALOG_VAR:
    {
        AnaOutLogic(addr, plcStack[stackIndex]);
        break;
    }
    case PROC_PLC_VAR:
    {
        plcVarArr[addr]=plcStack[stackIndex];
        break;
    }
    case PROC_CONSTANT_VAR:
    {
        paramStack[index+1]=plcStack[stackIndex];
        break;
    }
    case PROC_STACK_VAR:
    {
        putInStack(execute_flag,bp,addr);
        break;
    }
    }
}

void getStackVar(short int stkPtr,short int offset)
{
    short int type,addr,index,bp;

    bp	= stkPtr-paramStack[stkPtr-2]-2;
    index = bp+offset*2;
    type = paramStack[index];
    addr = paramStack[index+1];

    switch(type)
    {
    case PROC_DIGITAL_VAR:
    {
        plcStack[stackIndex-1] = getBit(addr);
        break;
    }
    case PROC_ANALOG_VAR:
    {
        plcStack[stackIndex-1] = signal_arr[addr];
        break;
    }
    case PROC_PLC_VAR:
    {
        plcStack[stackIndex-1]=plcVarArr[addr];
        break;
    }
    case PROC_CONSTANT_VAR:
    {
        plcStack[stackIndex-1]=paramStack[index+1];
        break;
    }
    case PROC_STACK_VAR:
    {
        getStackVar(bp,addr);
        break;
    }
    }
}

/*****************************************************************************/
/*      PlcAlloc(): Allocates memory buffer of specified size of bytes		 */
// This function will return pointer to first byte of buffer
/*****************************************************************************/

void *PlcAlloc(unsigned char pcbplc_logger,short int size)
{

	if(memIndex+size > BUFFER_SIZE)        //  BUFFER_SIZE 40000 bytes
	{
		//DispAt(1,"Plc Alloc Err");
		PlcFlag = 0;               // if size >BUFFER_SIZE then return 0
		return(NULL);              // else return address of first byte of buffer
	}
	memIndex+= size;
	return((void*)(&buffer[memIndex-size]));
}

//void *PlcAlloc(unsigned char pcbplc_logger, short int size)
//{
//    void *temp = NULL;
//    char tDebug[128] = {0,};
//
//    if(size > BUFFER_SIZE)
//    {
//        sprintf(tDebug, "Plc memory Allocation Error of %d bytes", size);
//        WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
//        return NULL;
//    }
//
//    temp = calloc(1, size);
//    if(temp == NULL)
//    {
//        sprintf(tDebug, "Plc memory Allocation Error by calloc of %d bytes", size);
//        WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
//    }
//
//    return temp;
//}

/*****************************************************************************/
/*      InitGlobalVar(): Initializes global memory variables to default value*/
/*****************************************************************************/
void InitGlobalVar()
{
    unsigned short int i;

    for(i=0;i<BUFFER_SIZE;i++)
    {
        buffer[i] = 0;
    }

    memIndex = 0;

    memset(string, 0, sizeof (string));
    memset(string1, 0, sizeof (string1));
    memset(AnaScan, 0, sizeof(AnaScan));

    memset(outputStatus, 0, sizeof (outputStatus));
    memset(outBits, 0, sizeof (outBits));
    memset(accessTable, 0, sizeof (accessTable));
    memset(feedbackArr, 0, sizeof (feedbackArr));
    memset(waitingFor, 0, sizeof (waitingFor));
    memset(modifiedArr, 0, sizeof (modifiedArr));

    falseInterlock = NULL;
    memset(conflictArr, 0, sizeof (conflictArr));

    int_arr = NULL;
    plcDataFile = NULL;
    memset(paramStack, 0, sizeof (paramStack));
    memset(plcStack, 0, sizeof (plcStack));

    memset(plcstringArr, 0, sizeof (plcstringArr));

    plcTimerTicks = 0;

    totalBits = 0;
    totalFloats = 0;
    totalPid = 0;

    maxSeqNode=0;
    maxEmrNode=0;
    maxIntNode=0;
    maxNodes=0;
    seqStartNode=0;
    emrStartNode=0;
    intStartNode=0;
    seqStep=0;
    emrStep=0;
    intStep=0;
    emrLastStep=0;
    curStep=0;
    executing=0;
    waitingFlag=0;
    maxPlcVar=0;
    paramSp=0;
    stackIndex=0;
    gsLastCommand = 0;
}



/* General purpose analog variables for pcbplc logic updation */
static void AnaOutLogic(short int index, float val)
{
    if( (index > 0) && (index <= MAX_GEN_ANA_PARA) )
    {
        index--;
        signal_arr[index] = val;
        gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG + index] = val;
    }
}

void ControlLogic(unsigned int Mod_Address, float Mod_Value)
{
	unsigned char Char_value;
    unsigned short int USInt_value;
    unsigned int UInt_value;
    float Float_value;
    unsigned int i=0,j=0;

	lwgsm_ip_t ip_t;
	unsigned char *buf_t;

    Char_value  = (unsigned char )Mod_Value;
    USInt_value = (unsigned short int )Mod_Value;
    UInt_value  = (unsigned int )Mod_Value;
    Float_value = (float )Mod_Value;

    switch (Mod_Address)
    {
		 case 604: //LORA Delta OTA Msg Sending Interval
		{
            if((USInt_value > 5) && (USInt_value <= 60))
            {
            	EPROM_General.LOta_MsgInterval = USInt_value;
            	gFinalAnaValF[Mod_Address] = USInt_value;
            	flag_flashUpdateEPROM_General=1;
            	flag_flashUpdateEPROM_General_WaitCounter=5;
            }
			break;
		}
		case LORA_ACTIVE_REGION_gFinalAnaValF://615:
		{

			break;
		}
		case LORA_CLASS_gFinalAnaValF://616:
		{

			break;
		}
    	case 605://900://
        {
            /* Unit Address */
            if((USInt_value > 0) && (USInt_value <=255))
            {
                gpcbplcCnfg.mRtuAddr = USInt_value;
                EPROM_General.Rtu_Detail.RTUId = USInt_value;
                gFinalAnaValF[Mod_Address] = USInt_value;
                flag_flashUpdateEPROM_General=1;
                flag_flashUpdateEPROM_General_WaitCounter=5;
            }
            break;
        }
    	case 606://900://
		{
			/* Log Rate */
			if(USInt_value > 1 && USInt_value <= 1440)
			{
				EPROM_General.LogRate = UInt_value;  // for log rate data structure match to use in IsLogRateMatched() func || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-22
				gFinalAnaValF[Mod_Address] = UInt_value;
				flagLORAPubLogData = 1;
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
    	case 610: // Wait to wait for node response of read data command
		{
			if((UInt_value >= 1) && (UInt_value <= 4))
			{
				EPROM_General.readCmdDelay = UInt_value;
				gFinalAnaValF[Mod_Address] = UInt_value;
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
				xTimerChangePeriod(readCmdTimer, pdMS_TO_TICKS(EPROM_General.readCmdDelay*1000), 0);
			}
			break;
		}
    	case 640:
		{
			if(USInt_value > 0 && USInt_value <= 10)
			{
			gFinalAnaValF[Mod_Address] = USInt_value;
			EPROM_General.LoraRetryCount = USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
    	case 641:
    	{
    		if(USInt_value > 0 && USInt_value <= 240)
    		{
    			gFinalAnaValF[Mod_Address] = USInt_value;
    			EPROM_General.LogSendDelay = USInt_value;
        		flag_flashUpdateEPROM_General=1;
        		flag_flashUpdateEPROM_General_WaitCounter=5;
        		xTimerChangePeriod(sendTimer, pdMS_TO_TICKS(EPROM_General.LogSendDelay*1000), 0);
    		}
    		break;
    	}
    	case 642:
    	{
			if(USInt_value > 7 && USInt_value <= 250)
			{
				gFinalAnaValF[Mod_Address] = USInt_value;
				EPROM_General.LoraRetryDelay = USInt_value;
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
				xTimerChangePeriod(ackTimer, pdMS_TO_TICKS(EPROM_General.LoraRetryDelay*1000), 0);
			}
			break;
    	}
    	case 618 :
        {
        	EPROM_General.DoModeDetails.Do_Mode = USInt_value;
            gpcbplcCnfg.mRtuDoMode = USInt_value;
            gFinalAnaValF[Mod_Address] = USInt_value;
            flag_flashUpdateEPROM_General=1;
            flag_flashUpdateEPROM_General_WaitCounter=5;
            break;
        }
        case 607://915:
        {
            if((USInt_value > 0) && (USInt_value <= MAXDO))
            {
            	EPROM_General.AI_DI_DO_Detail.Total_Do = USInt_value;
                gpcbplcCnfg.mMaxDoEnabled = USInt_value;
                gFinalAnaValF[Mod_Address] = USInt_value;
                flag_flashUpdateEPROM_General=1;
                flag_flashUpdateEPROM_General_WaitCounter=5;
            }
            break;
        }
        case 608://916 :
        {
            if((USInt_value > 0) && (USInt_value <= MAX_DI))
            {
            	EPROM_General.AI_DI_DO_Detail.Total_Di = USInt_value;
                gpcbplcCnfg.mMaxDiEnabled = USInt_value;
                gFinalAnaValF[Mod_Address] = USInt_value;
                flag_flashUpdateEPROM_General=1;
                flag_flashUpdateEPROM_General_WaitCounter=5;
            }
            break;
        }
        case 609://917 :
        {
            if((USInt_value > 0) && (USInt_value <= MAX_AI))
            {
            	EPROM_General.AI_DI_DO_Detail.Total_Ai = USInt_value;
                gpcbplcCnfg.mMaxAiEnabled = USInt_value;
                gFinalAnaValF[Mod_Address] = USInt_value;
                flag_flashUpdateEPROM_General=1;
                flag_flashUpdateEPROM_General_WaitCounter=5;
            }
            break;
        }
        case 611:
		{
			if(USInt_value > MODBUS_MASTER_MAX_TOTAL_QUERY)
			{
				USInt_value = MODBUS_MASTER_MAX_TOTAL_QUERY;
			}
			EPROM_Modbus_Quary_Detail.TotalQuery = USInt_value;
			gFinalAnaValF[Mod_Address] = USInt_value;
			BuildModbusMasterQueryTelegrams();
			flag_flashUpdateEPROM_Modbus_Quary_Detail=1;
			flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter=5;
			break;
		}
        case 612:
		{
			EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq = USInt_value;
			gFinalAnaValF[Mod_Address] = USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
        case 613:
        {
        	EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled = USInt_value;
            gFinalAnaValF[Mod_Address] = USInt_value;
            flag_flashUpdateEPROM_Modbus_Quary_Detail=1;
            flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter=5;
            break;
        }
        case 614 :
        {
        	switch(USInt_value)
        	{
        		case 1:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"airtelgprs.com");
				break;
				case 2:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"internet");
				break;
				case 3:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"www");
				break;
				case 4:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"bsnlstatic");
				break;
				case 5:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"bsnlnet");
				break;
				case 6:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"bsnllive");
				break;
				case 7:
					strcpy(EPROM_General.Mo_Comm.Mo_APN,"jionet");
				break;
				default:
					USInt_value = 255;
				break;
		   }
        	gFinalAnaValF[Mod_Address] = USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
		   break;
        }
        case 616:
		{
			EPROM_General.Cust_Detail.Client_Id = USInt_value;
			gFinalAnaValF[Mod_Address] = USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
        case 617:
		{
			EPROM_General.Cust_Detail.Reader_Id = USInt_value;
			gFinalAnaValF[Mod_Address] = USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
        case MODEM_MQTT_BROKER_IP_0_gFinalAnaValF:
		{
			if((USInt_value>=0) && (USInt_value<=255))
			{
				buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
			    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);
			    ip_t.ip[0] = USInt_value;
			    gFinalAnaValF[Mod_Address] = USInt_value;
			    memset(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,0,sizeof(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP));
			    sprintf((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,"%d.%d.%d.%d",ip_t.ip[0],ip_t.ip[1],ip_t.ip[2],ip_t.ip[3]);
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
        case MODEM_MQTT_BROKER_IP_1_gFinalAnaValF:
		{
			if((USInt_value>=0) && (USInt_value<=255))
			{
				buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
			    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);
			    ip_t.ip[1] = USInt_value;
			    gFinalAnaValF[Mod_Address] = USInt_value;
			    memset(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,0,sizeof(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP));
			    sprintf((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,"%d.%d.%d.%d",ip_t.ip[0],ip_t.ip[1],ip_t.ip[2],ip_t.ip[3]);
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
        case MODEM_MQTT_BROKER_IP_2_gFinalAnaValF:
		{
			if((USInt_value>=0) && (USInt_value<=255))
			{
				buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
			    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);
			    ip_t.ip[2] = USInt_value;
			    gFinalAnaValF[Mod_Address] = USInt_value;
			    memset(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,0,sizeof(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP));
			    sprintf((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,"%d.%d.%d.%d",ip_t.ip[0],ip_t.ip[1],ip_t.ip[2],ip_t.ip[3]);
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
        case MODEM_MQTT_BROKER_IP_3_gFinalAnaValF:
		{
			if((USInt_value>=0) && (USInt_value<=255))
			{
				buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
			    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);
			    ip_t.ip[3] = USInt_value;
			    gFinalAnaValF[Mod_Address] = USInt_value;
			    memset(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,0,sizeof(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP));
			    sprintf((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,"%d.%d.%d.%d",ip_t.ip[0],ip_t.ip[1],ip_t.ip[2],ip_t.ip[3]);
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
        case MODEM_MQTT_BROKER_PORT_gFinalAnaValF:
		{
			if((USInt_value>0)&&(USInt_value<=0xFFFF))
			{
				gFinalAnaValF[Mod_Address] = USInt_value;
				EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port = USInt_value;
				flag_flashUpdateEPROM_General=1;
				flag_flashUpdateEPROM_General_WaitCounter=5;
			}
			break;
		}
        case RS232_1_BAUDRATE_gFinalAnaValF:
		{
			switch(UInt_value) // for handling || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-26
			{
				case 1200:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 2400:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 4800:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 9600:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 14400:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 19200:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 38400:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 57600:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 115200:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 128000:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				case 256000:
				{
					gFinalAnaValF[Mod_Address] = UInt_value;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= UInt_value;
					flag_flashUpdateEPROM_General=1;
					break;
				}
				default:
				{
					gFinalAnaValF[Mod_Address] = 9600;
					EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate= 9600;
					flag_flashUpdateEPROM_General=1;
					break;
				}
			}
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
/*        case RS232_1_MASTER_SLAVE_DEBUG_gFinalAnaValF:
		{
			gFinalAnaValF[Mod_Address] = USInt_value;
			EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu= USInt_value;
			BuildModbusMasterQueryTelegrams();
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}*/
//      case RS232_1_MODBUS_RTU_ASCII_gFinalAnaValF:
//		{
//			gFinalAnaValF[Mod_Address] = USInt_value;
//			EPROM_General.S_Comm.Rs232_1_Info.S_Protocol= USInt_value;
//			flag_flashUpdateEPROM_General=1;
//			flag_flashUpdateEPROM_General_WaitCounter=5;
//			break;
//		}
        case RS232_1_MODBUS_POLL_FRQ_gFinalAnaValF:
		{
			gFinalAnaValF[Mod_Address] = USInt_value;
			EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq= USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
		case MODEM_MQTT_LIVE_FRQ_gFinalAnaValF:
		{
			gFinalAnaValF[Mod_Address] = USInt_value;
			EPROM_General.Mo_Comm.MQTT_LiveFreq= USInt_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
		case GPS_LAT_gFinalAnaValF:
		{
			gFinalAnaValF[GPS_LAT_gFinalAnaValF] = Float_value;
			EPROM_General.Cust_Detail.Lattitude = Float_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			Get_Astro_time();
			break;
		}
		case GPS_Log_gFinalAnaValF:
		{
			gFinalAnaValF[GPS_Log_gFinalAnaValF] = Float_value;
			EPROM_General.Cust_Detail.Longitude = Float_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			Get_Astro_time();
			break;
		}
		case COMM_MODE_ETHER_GPRS_gFinalAnaValF:
		{
			gFinalAnaValF[COMM_MODE_ETHER_GPRS_gFinalAnaValF] = Char_value;
			EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode = Char_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
		case TIMEZONE_SIGN_gFinalAnaValF:
		{
			gFinalAnaValF[TIMEZONE_SIGN_gFinalAnaValF] = Char_value;
			EPROM_General.Cust_Detail.Tz_sign = Char_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			Get_Astro_time();
			break;
		}
		case TIMEZONE_HOUR_gFinalAnaValF:
		{
			gFinalAnaValF[TIMEZONE_HOUR_gFinalAnaValF] = Char_value;
			EPROM_General.Cust_Detail.Tz_hours = Char_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			Get_Astro_time();
			break;
		}
		case TIMEZONE_MIN_gFinalAnaValF:
		{
			gFinalAnaValF[TIMEZONE_MIN_gFinalAnaValF] = Char_value;
			EPROM_General.Cust_Detail.Tz_min = Char_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			Get_Astro_time();
			break;
		}
		case ASTRO_OFFSET_gFinalAnaValF:
		{
			gFinalAnaValF[ASTRO_OFFSET_gFinalAnaValF] = Float_value;
			break;
		}
		case DEVICE_REBOOT_gFinalAnaValF:
		{
			gFinalAnaValF[DEVICE_REBOOT_gFinalAnaValF] = Char_value;
			reboot_device_func();
			osDelay(1000);
			break;
		}
		case DEVICE_REBOOT_TIME_DAY_NIGHT_gFinalAnaValF:
		{
			gFinalAnaValF[DEVICE_REBOOT_TIME_DAY_NIGHT_gFinalAnaValF] = Char_value;
			EPROM_General.Cust_Detail.reboot_day_night = Char_value;
			flag_flashUpdateEPROM_General=1;
			flag_flashUpdateEPROM_General_WaitCounter=5;
			break;
		}
		case PULSE_DI_INTERRUPT_TYPE_gFinalAnaValF:
		{
			gFinalAnaValF[PULSE_DI_INTERRUPT_TYPE_gFinalAnaValF] = Char_value;
			EPROM_Frequent.Pulse_DI_Interrupt_Type = Char_value;
			flag_flashUpdateEPROM_Frequent = 1;
			flag_flashUpdateEPROM_Frequent_WaitCounter = 5;
			break;
		}
		case PULSE_DI_FREQUENCY_METHOD_gFinalAnaValF:
		{
			gFinalAnaValF[PULSE_DI_FREQUENCY_METHOD_gFinalAnaValF] = Char_value;
			EPROM_Frequent.Pulse_DI_frequency_Method = Char_value;
			actualPulse_DI_frequency_time = EPROM_Frequent.Pulse_DI_frequency_time*EPROM_Frequent.Pulse_DI_frequency_Method;
			flag_flashUpdateEPROM_Frequent = 1;
			flag_flashUpdateEPROM_Frequent_WaitCounter = 5;
			break;
		}
		case PULSE_DI_FREQUENCY_TIME_gFinalAnaValF:
		{
			gFinalAnaValF[PULSE_DI_FREQUENCY_TIME_gFinalAnaValF] = USInt_value;
			EPROM_Frequent.Pulse_DI_frequency_time = USInt_value;
			actualPulse_DI_frequency_time = EPROM_Frequent.Pulse_DI_frequency_time*EPROM_Frequent.Pulse_DI_frequency_Method;
			flag_flashUpdateEPROM_Frequent = 1;
			flag_flashUpdateEPROM_Frequent_WaitCounter = 5;
			break;
		}
        case SAMPLE_TIME_TO_Collect_AI_gFinalAnaValF:
        {
            if((Char_value > 0) && (Char_value <= 255))
            {
            	EPROM_General.AI_DI_DO_Detail.Sample_time_to_collect_AI = Char_value;
                gFinalAnaValF[SAMPLE_TIME_TO_Collect_AI_gFinalAnaValF] = Char_value;
            }
            flag_flashUpdateEPROM_General=1;
            flag_flashUpdateEPROM_General_WaitCounter=5;
            break;
        }
        default:
        {
        	if( (Mod_Address >= START_IDX_GEN_ANA_PARA_TAG) &&
        		(Mod_Address < START_IDX_GEN_ANA_PARA_TAG+MAX_GEN_ANA_PARA))
        	{
        		AnaOutLogic(Mod_Address-START_IDX_GEN_ANA_PARA_TAG+1, Float_value);
        		//gFinalAnaValF[Mod_Address] = Float_value;
        	}
        	else if(Mod_Address >= 981 && Mod_Address<1461)	   //981+480=1461
            {
                //EPROM.DOSignal[Mod_Address-981]=Char_value;
            }
        	else if(Mod_Address >= 1485 && Mod_Address <= 1904)
        	{
        		Mod_Address -= 1485;
        		i = (unsigned int)Mod_Address/5;
        		j = Mod_Address%5;

        		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i * 5)+j ] = Char_value;

        		switch(j)
        		{
        			case 0:
					{
						EPROM_Schedule.Schedule[i].Sch_En_Di = Char_value;
						break;
					}
					case 1:
					{
						EPROM_Schedule.Schedule[i].Start_HH = Char_value;
						break;
					}
					case 2:
					{
						EPROM_Schedule.Schedule[i].Start_Min = Char_value;
						break;
					}
					case 3:
					{
						EPROM_Schedule.Schedule[i].Stop_HH = Char_value;
						break;
					}
					case 4:
					{
						EPROM_Schedule.Schedule[i].Stop_Min = Char_value;
						break;
					}
					default:
					{
						break;
					}
        		}
        		flag_flashUpdateEPROM_Schedule=1;
        		flag_flashUpdateEPROM_Schedule_WaitCounter=5;
        	}
        	else if((Mod_Address >= 1945) && (Mod_Address <= 2244))
            {
        		Mod_Address -= 1945;

        		i = (unsigned int)Mod_Address / 6;
                j = Mod_Address % 6;

                gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i * 6)+j ] = Float_value;

                switch(j)
                {
                	case 0:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection = Float_value;
                		break;
                	}
                	case 1:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId = Float_value;
                		break;
                	}
                	case 2:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr = Float_value;
                		break;
                	}
                	case 3:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mDataType = Float_value;
                		break;
                	}
                	case 4:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mNoOfRegister = Float_value;
                		break;
                	}
                	case 5:
                	{
                		EPROM_Modbus_Quary_Detail.Mod_Quary[i].mFunctionCode = Float_value;
                		break;
                	}
                	default:
                	{
                		break;
                	}
                }
                BuildModbusMasterQueryTelegrams();
                flag_flashUpdateEPROM_Modbus_Quary_Detail=1;
                flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter=5;
            }
        	else if((Mod_Address >= 2245) && (Mod_Address <= 2252))
			{
				Mod_Address -= 2245;

				i = (unsigned int)Mod_Address / 8;
				j = Mod_Address % 8;

				gFinalAnaValF[MODBUS_WRITE_QUERY_gFinalAnaValF + (i * 8)+j ] = Float_value;

				switch(j)
				{
					case 0:
					{
						MODBUS_Write[i].mPortSelection_write = Float_value;
						break;
					}
					case 1:
					{
						MODBUS_Write[i].mSlaveId_write = Float_value;
						break;
					}
					case 2:
					{
						MODBUS_Write[i].mRegStartAddr_write = Float_value-1;
						break;
					}
					case 3:
					{
						MODBUS_Write[i].mDataType_write = Float_value;
						break;
					}
					case 4:
					{
						MODBUS_Write[i].mNoOfRegister_write = Float_value;
						break;
					}
					case 5:
					{
						MODBUS_Write[i].mFunctionCode_write = Float_value;
						break;
					}
					case 6:
					{
						MODBUS_Write[i].mValue_write = Float_value;
						break;
					}
					case 7:
					{
						MODBUS_Write[i].mWriteQueryNumber = Float_value;
						flagWriteQueryAvailabe = 1;
						break;
					}
					default:
					{
						break;
					}
				}
			}
        	else if((Mod_Address >= 2253) && (Mod_Address <= 2284))
        	{
        		Mod_Address -= 2253;
        		mWriteQueryAck[Mod_Address] = Float_value;  
        		gFinalAnaValF[MODBUS_WRITE_QUERY_ACK_gFinalAnaValF + Mod_Address ] = Float_value;
        	}
        	else if((Mod_Address >=2285) && (Mod_Address <= 2434))
        	{
        		Mod_Address-=2285;

        		i = (unsigned int)Mod_Address/5;
        		j = Mod_Address%5;

        		switch(j)
        		{
					case 0:
					{
						EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type = USInt_value;
						gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5)+ j] = USInt_value;
		        		flag_flashUpdateEPROM_General=1;
						break;
					}
					case 1:
					{
						EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ = Float_value;
						gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5)+ j] = Float_value;
		        		flag_flashUpdateEPROM_General=1;
						break;
					}
					case 2:
					{
						EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS = Float_value;
						gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5)+ j] = Float_value;
		        		flag_flashUpdateEPROM_General=1;
						break;
					}
					case 3:
					{
						EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo = Float_value;
						gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5)+ j] = Float_value;
		        		flag_flashUpdateEPROM_General=1;
						break;
					}
					case 4:
					{
						EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi = Float_value;
						gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5)+ j] = Float_value;
		        		flag_flashUpdateEPROM_General=1;
						break;
					}
					default:
					{
						break;
					}
        		}
        		flag_flashUpdateEPROM_General_WaitCounter=5;
			}
            /* Recipe variable update by Modscan */
            else if( (Mod_Address >= (RECIPE_VAR_START_MODBUS_INDEX/2)) &&
                     (Mod_Address <= (RECIPE_VAR_END_MODBUS_INDEX/2)) )
            {
            	unsigned int t_index = Mod_Address - RECIPE_VAR_START_ARRAY_INDEX;

                if(	(3 != gPlcRecFlash.mPlcVarTypeArr[RECindex[t_index]]) ||
                	((Float_value >= gPcbplcInfo.mPlcVarArr_3[RECindex[t_index]]) &&
                	(Float_value <= gPcbplcInfo.mPlcVarArr_2[RECindex[t_index]])))
                {
                    plcVarArr[RECindex[t_index]] = Float_value;
                    gPlcRecFlash.plcVarArr[RECindex[t_index]] = Float_value;
                    gPcbplcInfo.mPlcVarArr_1[RECindex[t_index]] = Float_value;
                    gFinalAnaValF[Mod_Address] = Float_value;
            		flag_flashSaveRecipe =1;
            		flag_flashSaveRecipe_WaitCounter = 5;
                }
                else
                {
                    //fnDebugMsg("\r\nRec Variable value is out of range\r\n");
                }
            }
            else if((Mod_Address >=4350) && (Mod_Address <= 4429)) //pulse DI 16 DI
            {
        		Mod_Address-=4350;

        		i = (unsigned int)Mod_Address/5;
        		j = Mod_Address%5;

        		switch(j)
        		{
					case 0:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_count = UInt_value;
						if(i == 0)
						{
							EPROM_Frequent.DI1_Pulse = UInt_value;
							DI1_Pulse_Count =  UInt_value;
						}
						else if(i == 1)
						{
							EPROM_Frequent.DI2_Pulse = UInt_value;
							DI2_Pulse_Count =  UInt_value;
						}
						gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5)+ j] = UInt_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
		        		flag_flashUpdateEPROM_Frequent=1;
		        		flag_flashUpdateEPROM_Frequent_WaitCounter=5;
						break;
					}
					case 1:
					{
						//EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Freq = Float_value;
						//gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5)+ j] = Float_value;
						//break;
					}
					case 2:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Const = Float_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5)+ j] = Float_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					case 3:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Configured = Float_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5)+ j] = Float_value;
						flagLORAPubLogData = 1;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					case 4:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Calculated = Float_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5)+ j] = Float_value;
						break;
					}
					default:
					{
						break;
					}
        		}
            }
            else if((Mod_Address >=4430) && (Mod_Address <= 4559)) //pulse DO 26
            {
        		Mod_Address-=4430;

        		i = (unsigned int)Mod_Address/5;
        		j = Mod_Address%5;

        		switch(j)
        		{
					case 0:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_type = USInt_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5)+ j] = USInt_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					case 1:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_polarity = USInt_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5)+ j] = USInt_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					case 2:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width = USInt_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5)+ j] = USInt_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					case 3:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Count = USInt_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5)+ j] = USInt_value;
						break;
					}
					case 4:
					{
						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width_scale = USInt_value;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5)+ j] = USInt_value;
		        		flag_flashUpdateEPROM_General=1;
		        		flag_flashUpdateEPROM_General_WaitCounter=5;
						break;
					}
					default:
					{
						break;
					}
        		}
            }
			else if((Mod_Address >= 4600) && (Mod_Address <= 4625))
			{
        		Mod_Address-=4600;

        		i = (unsigned int)Mod_Address/2;
        		j = Mod_Address%2;
        		switch(j)
        		{
					case 0:
					{
						if(USInt_value==1)
						{
							Dual_DO_Pulse_Stage[i] = USInt_value;
							gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (i*2)+ j] = USInt_value;
						}
						break;
					}
					case 1:
					{
						if((USInt_value>0)&&(USInt_value<60000))
						{
							Dual_DO_actual_PulseWidth[i] = USInt_value;
							gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (i*2)+ j] = USInt_value;
						}
						break;
					}
					default:
					{
						break;
					}
        		}
			}
        }
    }
}

int Build_Data_for_server()
{
	flashDataSturct tFlashDataSturct;

	unsigned char NO_DI_LG = 0,ch,j;
	unsigned short int i,k = 0;

	tFlashDataSturct.sf_client_id = EPROM_General.Cust_Detail.Client_Id;//2;//EPROM.client_id;  				//4
	tFlashDataSturct.sf_reader_id = EPROM_General.Cust_Detail.Reader_Id;//EPROM.reader_id;  				//4+4=8
	tFlashDataSturct.sf_Date = gDate.Date;								//8+1=9
	tFlashDataSturct.sf_Month = gDate.Month;							//9+1=10
	tFlashDataSturct.sf_Year = gDate.Year;								//10+1=11
	tFlashDataSturct.sf_Hour = gTime.Hours;								//11+1=12
	tFlashDataSturct.sf_Min = gTime.Minutes;							//12+1=13
	tFlashDataSturct.sf_Sec = gTime.Seconds;							//13+1=14

	NO_DI_LG = (int)(EPROM_General.AI_DI_DO_Detail.Total_Di / 8);
	if((EPROM_General.AI_DI_DO_Detail.Total_Di % 8) != 0)
	{
		NO_DI_LG++;
	}

	tFlashDataSturct.sf_NO_DI_LG = 2; // 16/8=2							//14+1=15
	tFlashDataSturct.sf_NoofSMSTag = EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled;//(int)(EPROM.NoofSMSTag);			//15+1=16
	tFlashDataSturct.sf_Address = EPROM_General.Rtu_Detail.RTUId;//15;//EPROM.Address; 					//16+4=20

	for(i = 0 ; i < tFlashDataSturct.sf_NO_DI_LG ; i++)
	{
		ch = 0;
		for(j = 0 ; j < 8 ; j++)
		{
			//if(dig_bit_array1[(i*8)+j])
			if(dig_bit_array[(i*8)+j])
			{
				ch|=(1<<j);
			}
		}
		tFlashDataSturct.sf_DI[i]=ch;
	}

	k = 0;
	do
	{
		tFlashDataSturct.sf_AnalogValue[k] = gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+k];
		k++;
	}
	while(k < EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled);//EPROM.NoofSMSTag);

	if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		ExtFlash_WriteHistoricalData(tFlashDataSturct);
		xSemaphoreGive(sendExternalFlashSemaphore);
	}


    return 0;
}

int8_t LoraPublish()
{
	unsigned char tempBuffer[300];

    uint16_t BufferSize = 0;
    union_Datatypes D1;
//0103500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000398C
    tempBuffer[ ID ]       = EPROM_General.Rtu_Detail.RTUId;
    tempBuffer[ FUNC ]     = 0x03;
    tempBuffer[ 2 ]        = EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled*4;//0x52;
    BufferSize=3;
    for(unsigned char i=0;i<EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled;i++)
    {
    	D1.fl = gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+i];
    	tempBuffer[ BufferSize ] = D1.s_ch[1];
    	BufferSize++;
		tempBuffer[ BufferSize ] = D1.s_ch[0];
		BufferSize++;
		tempBuffer[ BufferSize ] = D1.s_ch[3];
		BufferSize++;
		tempBuffer[ BufferSize ] = D1.s_ch[2];
		BufferSize++;
    }

	if(flagExLog)
	{
		tempBuffer[2] += 12;
		for(uint8_t idx = 0; idx < 3; idx++)
		{
			switch(idx)
			{
			case 0:
				D1.fl = EPROM_Dync.rebootCount;
				break;
			case 1:
				D1.fl = EPROM_General.LogRate;
				break;
			case 2:
				char *ver = EPROM_General.Rtu_Detail.Hex_Version;
				uint16_t curVer = 0;

				while (*ver != '\0')
				{
				    if (*ver >= '0' && *ver <= '9')
				    {
				        curVer = curVer * 10 + (*ver - '0');
				    }
				    ver++;
				}
				D1.fl = curVer;
				break;
			}
			tempBuffer[ BufferSize ] = D1.s_ch[1];
			BufferSize++;
			tempBuffer[ BufferSize ] = D1.s_ch[0];
			BufferSize++;
			tempBuffer[ BufferSize ] = D1.s_ch[3];
			BufferSize++;
			tempBuffer[ BufferSize ] = D1.s_ch[2];
			BufferSize++;
		}
	}

	uint16_t u16crc = calcCRC(tempBuffer, BufferSize);
	tempBuffer[ BufferSize ] = u16crc >> 8;
	BufferSize++;
	tempBuffer[ BufferSize ] = u16crc & 0x00ff;
	BufferSize++;

	memcpy(lora_tx_buf,0,sizeof(lora_tx_buf));
	asciiStringToHexString(tempBuffer,lora_tx_buf,BufferSize);
	BufferSize = 2 * BufferSize;

	for(unsigned char i = 0; i < sizeof(LoRa_Modem.lora_serial_no); i++)
		lora_tx_buf[BufferSize++] = LoRa_Modem.lora_serial_no[i];

	lora_tx_buf[BufferSize++] = '\0';
}

/**************************************************************************//**
 * Function name 	: ExtFlash_WriteHistoricalData
 * arguments		:
 * 		 			:
 * return			:
 * Note				:
 * 					:
 *****************************************************************************/
void ExtFlash_WriteHistoricalData(flashDataSturct tflashDataSturct)
{
	//sprintf((char*)debug_print,"size of dataStruct : %d size of runtimeStruct : %d",sizeof(tFlashDataSturct),sizeof(g_flashRunTimeParaSturct));
	//fnDebugMsg(debug_print);
    unsigned int WriteAddress;

	if(gStopHistoricalDataStore == 0)
	{

		if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter<MAX_HISTORY_DATA_PACKETS)
		{
			WriteAddress = g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter*512+HISTORY_DATA_FILE_START_ADDRESS;

			if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter%8==0)
			{
				MX25L_EraseSector_QPI(WriteAddress);
			}

			W25Q_Write_continous((u8_t *)&tflashDataSturct, sizeof(tflashDataSturct),WriteAddress);

		}
		else
		{
			g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter = 0;
			g_flashRunTimeParaSturct.s_ExtDataFlash_IsDataLogOverwritten = 1;
			WriteAddress = g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter*512+HISTORY_DATA_FILE_START_ADDRESS;
			if(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter%8==0)
			{
				MX25L_EraseSector_QPI(WriteAddress);
			}
			W25Q_Write_continous((u8_t *)&tflashDataSturct, sizeof(tflashDataSturct),WriteAddress);
			//WriteFlashtoflashMemory_new(g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter,(unsigned char *)&tflashDataSturct);
		}

		g_flashRunTimeParaSturct.s_ExtDataFlash_PageCounter++;
		MX25L_Erase_Write_One_Sector((unsigned char *)&g_flashRunTimeParaSturct,sizeof(g_flashRunTimeParaSturct), HISTORY_DATA_RUN_TIME_PARA_START_ADDRESS);
	}
}

