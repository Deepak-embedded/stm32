/***********************************************************************
        RTU PROGRAM Project : 12-3-08
        AUTHOR : samir B. Malvi
        File name : Comm.c
        Description : This file is having communication related routines.
***********************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "define.h"
#include "structure.h"
#include "pcbplc.h"
#include "pcbplccomm.h"
#include  "pcbplcService.h"
#include "main.h"

extern UART_HandleTypeDef huart1;
extern char csStrList[8][GSM_MSG_SIZE];
extern char dig_bit_array[];
short count;
unsigned char Buf[1700];
int tsPcbplcFd = -1;
struct circbuffst circBuffer;
short MsgCnt;
unsigned long ModTOUT=0,ModTOUTEn=0;
unsigned char pcbplc_logger = 1;
/*************************************************************/

struct sDateTime sCurDateTime;									//structure to store date time information
struct sSendFrame sFrame;										//Frame structure for header of message
struct sScalling sScaleUnit[16];  								//scalling info. structures for all analog channels // Made 16 from MAX_ANA_CH16 jk
struct sSetValue sChangeVal[10];								//structure for changing value for analog or digital output channel // Made 10 from 30 jk
struct sHwDbInfo sCurConfig; 									//current hardware and database configuration

struct splcDataFile *plcDataFile = NULL;  								//plc logic data file
//union floattochar para;

unsigned char PLCCOMM = 1;



/*****************************     globle variable **********************************************/


short int 				seqStep,emrStep,intStep;						   		
unsigned short int  totalBits; /* Max Digital Nodes */
unsigned short int totalFloats; /* Max Analog Nodes */
short int 				seqStartNode,emrStartNode,intStartNode;
unsigned short int 		curCommStatus=0,xCount=0;
unsigned char 			*tempPtr;
unsigned int 			rcv_time;
unsigned char 			Header[8];
unsigned char 			transBuf[MAXTRANSBLOCK]; 				//MAXTRANSBLOCK=500//buffer to transmit
unsigned char 			Scnet=0;
unsigned short int 		isize=0,dataSize;      					//counter=0;
unsigned short int 		newMessage=0;
unsigned char 			dummyDataPtr[MAX_BUFSIZE];
char 					inchar,curchar; 						//currently received character is inchar,current character being fetched from circular buffer is curchar.
unsigned char 			bNotCompleted = 0; 						//current message is completed or not.
unsigned char 			bAllDownload=1;							//all things to be received from pc is yet came or not.
unsigned char 			transflag=0; 							//any thing to transmit or not
unsigned short int 		translen;
unsigned short int 		maxAna=50,maxDig=32;  					//maximum number of analog and digital channels
unsigned char 			AiFlag,DiFlag,PidFlag,PlcFlag,LogicCtrl;//AI is to be scan?, DI is to be scan?, Plc logic execution is to be done?,pid loop is to be executed?,keyboard is to be scanned?
unsigned char 			*pCurPtr;								//pointer to current character pointer depending upon the data being downloaded from pc


unsigned char 			overRideFlagArr[MAX_DIG_CHNS];			//jk (unsigned)//memory buffer for overrides for digital channels
char 					plcFileRec=0;
char 					prevPlcFlag;

unsigned short int 		gChkSum;
short int   plcUserArr[MAX_PLCVAR /*50*/]; //Changed By mukesh, because it may crate buffer overflow problem.

char 					firstTime=1;
unsigned short int 				memIndex=0;
short int 				*int_arr;
unsigned short int 		crc_rem1;
unsigned short int 		crc=0;
unsigned short int 		mod1;
unsigned short int 		bias1,inner1;
unsigned char			temp2;
unsigned short int 		k;
//volatile unsigned char  *serialPointer1;
unsigned int  serialPointer1;
unsigned int 			stemp;
unsigned long int 		intstat;


unsigned int logicSize=0,packetSize=0;
unsigned char packetno=0,totalpacket=0;
unsigned short int dataCheckSum=0, dataLength=0;

//unsigned short int EPROM.Max_Plc_var = 0;
unsigned char Init_Storage=0;


extern short int fnReadPLCFile(void);
extern unsigned char * uOpenFile(char *ptrfileName);
extern unsigned long ModTOUT,ModTOUTEn;



//static unsigned char * MyptrFile;
//static unsigned char ucMimeType = 5;
//static unsigned long file_length ;

//extern char FactoryDef;
char Restart_Device;

unsigned char PLC_PROG_Stat;
int SaveToDisk;

/*****************************************************************************
 * RcvHandler(): Received characters handler.
 * This function is responsible for decode any message came from the master
 * or from other slave. It decodes the message and put the data bytes to
 * corresponding buffer and prepares transmit buffer, for ACK or NACK.
 * It also decodes for data request and prepared the transmit buffer with
 * analog and digital scanned data.
*****************************************************************************/
//void RcvHandler()
//{
//    short int i=0,size;
//    unsigned char frameCkSum;
//    unsigned short int chkSum=0;
//    unsigned char TPRec[20];
//
//    /* No meaning of below check */
//    if(!PLCCOMM)
//    {
//        return;
//    }
//
//    /* PCBPLC Programming mode */
//    if(program_bit)
//    {
//        /*if adding of new char fills up reset tail */
//        if(circBuffer.tail >= BSIZE_E)
//        {
//            circBuffer.tail = 0;
//        }
//        // In program mode consider BSIZE_E
//    }
//    /* PCBPLC Run mode */
//    else
//    {
//        //this is add for the first time allocation when power on
//        if(circBuffer.tail == 0 && plcallocmemflag == 0)
//        {
//            plcFileRec=1;
//            firstTime = 1;
//            goto samir;
//        }
//
//        /*if adding of new char fills up reset tail */
//        if(circBuffer.tail >= BSIZE)
//        {
//            circBuffer.tail = 0;
//        }
//    }
//
//    switch(curStatus)  /* RTU's communication state */
//    {
//    case WAIT_FOR_FRAME:
//    {
//        if((circBuffer.tail - circBuffer.head) >= FRAMESIZE)  				/* if any frame has came */
//        {
//            for(i = 0 ; i < FRAMESIZE; ++i)
//            {
//                Header[i] = comin();										/* get from the circular buffer */
//            }
//
//            temp2 = Header[2];									   // comming data format is reserved so change orded
//            Header[2] = Header[3];								  // change MSB to LSB. for data and check sum only.
//            Header[3] = temp2;
//
//            temp2 = Header[4];
//            Header[4] = Header[5];
//            Header[5] = temp2;
//
//            if(Header[0]  > 63)        										// check first byte of header
//            {
//                Header[0] = (Header[0] - 63);
//                Scnet = 1;
//            }
//            else
//            {
//                Scnet = 0;
//            }
//
//            memcpy(&sFrame,Header,FRAMESIZE);  								// copy in the frame structure value of header
//            frameCkSum = 0;
//            if(Scnet == 0)
//            {
//                frameCkSum = Header[0] + Header[1] + Header[2] + Header[3] + Header[4] + Header[5] + Header[6];
//            }
//            else
//            {
//                for(i=0;i<7;i++)
//                {
//                    frameCkSum  ^= Header[i];
//                }
//            }
//
//            if((frameCkSum != sFrame.reserved)) /*|| (sFrame.reserved == 0))*/
//            {
//                flushccb();     // If checksum is 0 then clear circular buffer and return.
//                return;
//            }
//
//            if(Scnet == 0)
//            {
//                if((sFrame.info == PLCFILE) || (sFrame.info == RCPFILE) || (sFrame.info == OVERRIDE))
//                    return;
//                if((sFrame.info == LIMITS) || (sFrame.info == ANASCANSKIP))
//                    return;
//            }
//            //if(sFrame.address != EPROM.Address)   //check if adress matches, if no return
//            if(sFrame.address != gpcbplcCnfg.mRtuAddr)
//            {
//                sFrame.dataSize = ((((short int)Header[3]) << 8) & 0xff00) + (Header[2] & 0x00ff);
//                if(sFrame.dataSize != 0)
//                {
//                    if(sFrame.dataSize >= BUFFER_SIZE)  //PLCDATA jk
//                    {
//                        flushccb();         // if size exceeds max size then also return
//                        return;
//                    }
//                    sFrame.info = NOT_FOR_THIS_RTU;
//                }
//                else                        // Add information of not this RTU if no address
//                {                           // match is occuring.
//                    flushccb();
//                    return;
//                }
//            }
//
//            printf("\n Wait For Frame ");
//            printf("\nCmd=%x sz=%d ",(int)sFrame.info,(int)sFrame.dataSize);
//
//            dataSize = sFrame.dataSize;
//
//            switch(sFrame.info)							/* decode the message as the info byte */
//            {
//            case 0x01:  /* ENQ */ // this is used for the ack the software.
//            {
//                flushccb();
//                newMessage = 1;
//                MsgCnt = 0;
//
//                sFrame.address = gpcbplcCnfg.mRtuAddr;
//                if(bAllDownload)
//                {
//                    sFrame.reply = ACK;
//                }
//                else
//                {
//                    sFrame.reply = NAK;
//                }
//
//                memcpy(transBuf,&sFrame,FRAMESIZE);
//                transflag = 1;
//                translen = FRAMESIZE;
//                TransmitHandler();
//                printf("\n......... Ack ...................");
//                PLC_PROG_Stat=1;
//                break;
//            }
//            case 0x03: /* EOT */
//            {
//                flushccb();
//                newMessage = 1;   // Data xmission case which xmits analog and digital values
//                MsgCnt = 0;
//
//                if(1)
//                {
//                    sFrame.address = gpcbplcCnfg.mRtuAddr;
//                    bAllDownload = 1;
//                    i = (maxAna * sizeof(float)) + (maxDig/8) + 1 + 4;
//                    sFrame.dataSize = i;
//
//                    MakeBlock();
//                    chkSum = cal_crc1((unsigned char*)dummyDataPtr,sFrame.dataSize);
//
//                    sFrame.checkSum = chkSum;
//                    memmove(transBuf,&sFrame,FRAMESIZE);
//                    putInt(i,&transBuf[2]);
//                    putInt(chkSum,&transBuf[4]);
//
//                    memmove(&transBuf[FRAMESIZE],dummyDataPtr,sFrame.dataSize);
//
//                    transflag = 1;
//                    translen = FRAMESIZE + sFrame.dataSize;
//                }
//                else
//                {                           // if value is not 1 then datasize is 0
//                    bAllDownload = 0;         // and xmit frame.
//                    sFrame.address = gpcbplcCnfg.mRtuAddr;
//                    sFrame.dataSize = 0;
//                    memcpy(transBuf,&sFrame,FRAMESIZE);
//                    transflag = 1;
//                    translen = FRAMESIZE;
//                }
//                break;
//            }
//            case 0x04:                              /* EOT */
//            {
//                flushccb();					// This case also transfers data
//                newMessage = 1;             // but maxana=50 instead of 16
//                MsgCnt = 0;                 // and maxdig=32 instead of 16
//                                            // bAllDownload = 1;
//
//                sFrame.address = gpcbplcCnfg.mRtuAddr;
//                sFrame.info = 4;
//                sFrame.reply = ACK;
//
//                maxAna = 50;				// original 50
//                maxDig = 32;				// original 32
//                i = (maxAna * sizeof(float)) + (maxDig/8) + 1 + 4;
//                sFrame.dataSize = (short int)i;
//
//                MakeBlock();
//                chkSum = cal_crc((unsigned char*)dummyDataPtr,i);
//
//                sFrame.checkSum = chkSum;
//                memcpy(transBuf,&sFrame,FRAMESIZE);
//                putInt(i,&transBuf[2]);
//                putInt(chkSum,&transBuf[4]);
//                memcpy(&transBuf[FRAMESIZE],dummyDataPtr,i);
//                transflag = 1;
//                translen = (FRAMESIZE + i);
//                TransmitHandler();
//                flushccb();
//                printf("transmission sucess");
//                break;
//            }
//            case DATETIME:
//            case HDDBINFO:    /* Hardware-database information */
//            case PLCFILE:     /* Plc Logic file */
//            case RCPFILE:     /* corresponding Recipe file for the Plcfile  samir added*/
//            {
//                flushccb();         // Before file transfer clean buffer
//
//                if(ProgramMode == 0 && program_bit==0)
//                {
//                    printf("In RUN MODE");
//                    sleep(10);
//                    transBuf[0] = 1;
//                    transBuf[1] = 21;
//                    transBuf[2] = 0;
//                    transBuf[3] = 0;
//                    transBuf[4] = 0;
//                    transBuf[5] = 0;
//                    transBuf[6] = ACK;
//                    transBuf[7] = transBuf[0] + transBuf[1] + transBuf[2] + transBuf[3] + transBuf[4] + transBuf[5] + transBuf[6];
//                    translen = FRAMESIZE;
//                    WriteData(transBuf,translen);
//                    break;
//                }
//
//                newMessage = 1;
//                MsgCnt = 0;
//                curStatus = WAIT_FOR_DATA;
//                sFrame.reply = ACK;
//                isize = 0;
//                dataCheckSum = sFrame.checkSum;
//                dataLength = sFrame.dataSize;
//                printf("\nChkSum=%d size=%d",(int)dataCheckSum,(int)dataLength);
//
//                transBuf[0] = 1;
//                transBuf[1] = 13;
//                transBuf[2] = 0;
//                transBuf[3] = 0;
//                transBuf[4] = 0;
//                transBuf[5] = 0;
//                transBuf[6] = 0;
//                transBuf[7] = transBuf[0]+transBuf[1]+transBuf[2]+transBuf[3]+transBuf[4]+transBuf[5]+transBuf[6];
//                transflag = 1;
//                translen = FRAMESIZE;
//                bCommPriority = 1;
//                printf("Receiving .Rec");
//                printf("PLC Logi Xfer");
//                break;
//            }
//            case OVERRIDE:    /* override flags for the digital channels */
//            case LIMITS:      /* Scalling information for channels */
//            case CHANGEVAL:      /* Control the Digital Out */
//            case ANASCANSKIP: /* Analog scan skip variable for analog channels */
//            {
//                newMessage = 1;
//                MsgCnt = 0;
//                curStatus = WAIT_FOR_DATA;
//                break;
//            }
//            case PROFILE:     /* SetPoint's Profile for Pid loop */
//            {
//                PidFlag = 0;
//                newMessage = 1;
//                MsgCnt = 0;
//
//                size = (sFrame.dataSize/4);
//                size = size / 2;
//
//                curStatus = WAIT_FOR_DATA;
//                break;
//            }
//            case PLCLOGIC:   /* Start logic execution command */
//            {
//                flushccb();
//                newMessage = 1;
//                MsgCnt = 0;
//                if(LogicCtrl == 1)
//                    LogicCtrl = 0;
//                else LogicCtrl = 1;
//                memmove(&sFrame,0,FRAMESIZE);
//                sFrame.reply = ACK;
//                memmove(transBuf,&sFrame,FRAMESIZE);
//                transflag = 1;
//                translen = FRAMESIZE;
//                break;
//            }
//            case PLCTEST:
//            {
//                flushccb();
//                newMessage = 1;
//                MsgCnt = 0;
//                sFrame.reply = ACK;
//                memmove(transBuf,&sFrame,FRAMESIZE);
//                transflag = 1;
//                translen = FRAMESIZE;
//                curCommStatus = 0;
//                bCommPriority = 1;
//                printf("PLC Test");
//                printf("LOgic Transfer");
//                break;
//            }
//            case RESET_XFER:
//            {
//                printf("\nRESET XFER");
//                flushccb();
//                MsgCnt = 0;
//                logicSize = 0;
//                break;
//            }
//            case PLCLOGICXFER:
//            {
//                flushccb();         // Before file transfer clean buffer
//                if(ProgramMode == 0 && program_bit==0)
//                {
//                    transBuf[0] = 1;
//                    transBuf[1] = 21;
//                    transBuf[2] = 0;
//                    transBuf[3] = 0;
//                    transBuf[4] = 0;
//                    transBuf[5] = 0;
//                    transBuf[6] = ACK;
//                    transBuf[7] = transBuf[0] + transBuf[1] + transBuf[2] + transBuf[3] + transBuf[4] + transBuf[5] + transBuf[6];
//                    translen = FRAMESIZE;
//                    WriteData(transBuf,translen);
//                    break;
//                }
//
//                newMessage = 1;
//                MsgCnt = 0;
//                curStatus = WAIT_FOR_DATA;
//                sFrame.reply = ACK;
//                isize = 0;
//                dataCheckSum = sFrame.checkSum;
//                dataLength = sFrame.dataSize;
//                printf("\nChkSum=%d size=%d",(int)dataCheckSum,(int)dataLength);
//                transBuf[0] = 1;
//                transBuf[1] = 52;
//                transBuf[2] = 0;
//                transBuf[3] = 0;
//                transBuf[4] = 0;
//                transBuf[5] = 0;
//                transBuf[6] = 0;
//                transBuf[7] = transBuf[0]+transBuf[1]+transBuf[2]+transBuf[3]+transBuf[4]+transBuf[5]+transBuf[6];
//                transflag = 1;
//                translen = FRAMESIZE;
//                bCommPriority = 1;
//                printf("Receiving .PLC");   // Message before file xfer
//                printf("\nPLC Logi Xfer");
//                break;
//            }
//            case SMSRECEIVE:
//            {
//                newMessage = 1;
//                MsgCnt = 0;
//                isize = 0;
//                bCommPriority = 1;
//                curStatus = WAIT_FOR_DATA;
//                printf("\nSMS Receive");
//                break;
//            }
//            case 54:
//            {
//                break;
//            }
//            case NOT_FOR_THIS_RTU:   /* Other RTU's Data */
//            {
//                printf("Invalid Address");
//                flushccb();
//                break;
//            }
//            default:        /* garbage data, discard it */
//            {
//                flushccb();
//                break;
//            }
//            }
//        }
//        else     /* timeout */
//        {
//            if(MsgCnt > TIME_OUT)
//            {
//                // If received buffer not of proper length then
//                newMessage = 0;   // check timeout
//                MsgCnt = 0;
//                flushccb();
//                printf("Comm. Timeout");    // Communication timeout
//            }
//        }
//        break;                  // Wait _for_frame end here
//    }
//    case WAIT_FOR_DATA:		/* Data has to come */
//    {
//        if(ModTOUT > 200)
//        {
//            ModTOUT = 0;
//            ModTOUTEn = 0;
//
//            if(circBuffer.cnt == 0)
//            {
//            }
//            else
//            {
//                switch(sFrame.info)
//                {
//                case DATETIME:
//                    pCurPtr = (char*)&sCurDateTime;
//                    break;
//                case HDDBINFO:
//                    pCurPtr = (char*)&sCurConfig;
//                    break;
//                case PLCFILE:
//                    PlcFlag = 0;
//                    break;
//                case RCPFILE:
//                    break;
//                case PROFILE:
//                    break;
//                case OVERRIDE:
//                    pCurPtr = (char *)&overRideFlagArr;
//                    break;
//                case LIMITS:
//                    pCurPtr = (unsigned char *)sScaleUnit;
//                    break;
//                case CHANGEVAL:
//                    pCurPtr = (unsigned char *)&sChangeVal;
//                    break;
//                case ANASCANSKIP:
//                    //						pCurPtr = (char *)&anaScanSkip;
//                    break;
//                case NOT_FOR_THIS_RTU: /* if other RTU's data, flush the buffer, reset current status and return */
//                    flushccb();
//                    curStatus = WAIT_FOR_FRAME;
//                    return;
//                case PLCLOGICXFER:
//                    break;
//                case SMSRECEIVE:
//                    break;
//                default:
//                    curStatus = WAIT_FOR_FRAME;
//                    return;
//                }
//                bNotCompleted = 0;
//                MsgCnt = 0;											/* get the datasize much data from the circular buffer */
//
//
//                if((sFrame.info == PLCLOGICXFER) || (sFrame.info==PLCFILE))
//                {
//                    /* Delay */
//                    for(rcv_time=0;rcv_time<40000;rcv_time++);                    // put delay
//                    for(rcv_time=0;rcv_time<40000;rcv_time++);                   // put delay
//
//                    packetno 	=	circBuffer.Adata[0];
//                    totalpacket 	=	circBuffer.Adata[1];
//                    packetSize	= ((unsigned int)circBuffer.Adata[3] << 8 )+ (unsigned int)circBuffer.Adata[2];
//
//                    printf("\npkt=%d of %d",(int)packetno,(int)totalpacket);
//                    printf("\npkt sz=%d ",(int)packetSize);
//
//                    serialPointer1 =  PCB_PLC_FILE_START+logicSize;
//                    logicSize = logicSize + packetSize;
//
//                    if(Init_Storage==0)
//                    {
//                        Init_Storage = 1;
//
//                        /* create file and write file */
//                        printf("Creating File - 6.BIN and Storing first packet");
//
//                        //MyptrFile = uOpenFile("6.BIN");
//                        tsPcbplcFd = open("6.BIN", O_CREAT|O_RDWR|O_TRUNC, 0666);
//                        if(tsPcbplcFd < 0)
//                        {
//                            printf("Error to open file %s, error:%s at %s\n", "6.BIN",strerror(errno), __func__);
//                            //TODO: Need to return from here.
//                        }
//
//                        memset(Buf,0,1632);
//
//                        printf("DataLen=%u,packet_no=%d,total_packet=%d\n",dataLength,packetno,totalpacket);
//
//                        //TODO: Need to handle offset of data here.
//                        write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//
//                        if(1 == totalpacket)
//                        {
//                            close(tsPcbplcFd);
//                            tsPcbplcFd = -1;
//                        }
//                    }
//                    else
//                    {
//                        if(packetno != totalpacket)
//                        {
//                            //TODO: Need to handle offset of data here.
//                            write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//                            printf("Storing Packet\n");
//                        }
//                        else
//                        {
//                            //uFileWrite(MyptrFile,&circBuffer.Adata[4+circBuffer.head],packetSize,1);
//                            //file_length = uFileCloseMime(MyptrFile, &ucMimeType);
//                            //TODO: Need to handle offset of data here.
//                            write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//                            close(tsPcbplcFd);
//                            tsPcbplcFd = -1;
//                            printf("{PLC File Wrote=%ld}\n",file_length);
//                            printf("Storing Last Packet\n");
//                            printf("Close File\n");
//                        }
//                    }
//
//                    serialPointer1 = (PCB_PLC_FILE_START+logicSize-packetSize);
//                    bNotCompleted = 0;
//                    printf("1\n");
//                    printf("\nlogicSize= %d",(int)logicSize);
//
//                    if(packetno == totalpacket)
//                    {
//                        chkSum = plc_cal_crc(1,dataLength); 							// calculate data bytes checksum
//                        if(chkSum==dataCheckSum)
//                        {
//                            printf("m\n");
//                            sFrame.reply=ACK;
//                            transBuf[0] = 1;
//                            transBuf[1] = 51;
//                            transBuf[2] = 0;
//                            transBuf[3] = 0;
//                            transBuf[4] = 0;
//                            transBuf[5] = 0;
//                            transBuf[6] = ACK;
//                            transBuf[7] = packetno;
//                            transflag = 1;
//                            translen = FRAMESIZE;
//                            WriteData(transBuf,translen);
//                            //DispAt(31,"*");       //Chksum match
//                            printf("Checksum matched\n");
//samir:
//                            plcallocmemflag = 1;
//                            Extract_Alloc();      // Get File from circ buffer and allocate it
//                            //DispAt(32,"*");      // File transferred
//                            //flushccb();          // Clear Circ buffer after receiving File
//                            bCommPriority = 0;
//                            newMessage = 1;
//                            MsgCnt = 0;
//                            goto label1;
//                        }
//                        else
//                        {
//                            sFrame.reply=NAK;
//                            if(packetno == totalpacket)
//                            {
//                                logicSize=0;
//                            }
//                            goto label1;
//                        }
//                    }
//                    else
//                    {
//                        sFrame.reply = ACK;
//                    }
//
//                    printf("3\n");
//                    goto label1;
//                }
//                if(sFrame.info==RCPFILE)
//                {
//                    for(rcv_time=0;rcv_time<40000;rcv_time++);                    // put delay
//                    for(rcv_time=0;rcv_time<40000;rcv_time++);                   // put delay
//
//                    packetno = circBuffer.Adata[0];
//                    totalpacket = circBuffer.Adata[1];
//                    packetSize = ((unsigned int)circBuffer.Adata[3] << 8 )+ (unsigned int)circBuffer.Adata[2];
//
//                    printf("\npkt=%d of %d",(int)packetno,(int)totalpacket);
//                    printf("\npkt sz=%d ",(int)packetSize);
//
//                    serialPointer1 =  REC_FILE_START+logicSize;
//                    logicSize = logicSize + packetSize;
//
//                    if(1 == packetno)
//                    {
//                        // create file and write file
//                        //MyptrFile = uOpenFile("U.BIN");
//                        tsPcbplcFd = open("U.BIN", O_CREAT|O_RDWR|O_TRUNC, 0666);
//                        if(tsPcbplcFd < 0)
//                        {
//                            printf("Error to open file %s, error:%s at %s\n", "6.BIN",strerror(errno), __func__);
//                            //TODO: Need to return from here.
//                        }
//                        memset(Buf,0,1632);
//
//                        //uFileErase(MyptrFile,dataLength);
//                        write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//                        //uFileWrite(MyptrFile,&circBuffer.Adata[4+circBuffer.head],packetSize,1);
//
//                        if(totalpacket==1)
//                        {
//                            //file_length=uFileCloseMime(MyptrFile, &ucMimeType);
//                            close(tsPcbplcFd);
//                        }
//                    }
//                    else
//                    {
//                        if(packetno!=totalpacket)
//                        {
//                            write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//                            //uFileWrite(MyptrFile,&circBuffer.Adata[4+circBuffer.head],packetSize,1);
//                            //Write File every time
//                        }
//                        else
//                        {
//                            write(tsPcbplcFd, &circBuffer.Adata[4+circBuffer.head], packetSize);
//                            //uFileWrite(MyptrFile,&circBuffer.Adata[4+circBuffer.head],packetSize,1);
//                            //      file_length=uFileCloseMime(MyptrFile, &ucMimeType);
//                            //    printf("\r\n{ REC File Wrote=%d}",file_length);
//                            //fnDebugMsg(dispStr);
//                            close(tsPcbplcFd);
//                        }
//                    }
//
//                    serialPointer1 =  (REC_FILE_START+logicSize-packetSize);
//                    bNotCompleted = 0;
//                    printf("1\n");
//                    printf("\nlogicSize= %d",(int)logicSize);
//
//                    if(packetno == totalpacket)
//                    {
//                        chkSum = dataCheckSum;
//                        if(chkSum == dataCheckSum)
//                        {
//                            //fnDebugMsg("m\n");
//                            transBuf[0] = 1;
//                            transBuf[1] = RCPFILE;
//                            transBuf[2] = 0;
//                            transBuf[3] = 0;
//                            transBuf[4] = 0;
//                            transBuf[5] = 0;
//                            transBuf[6] = ACK;
//                            transBuf[7] = packetno;
//                            transflag = 1;
//                            translen = FRAMESIZE;
//
//                            WriteData(transBuf,translen);
//                            //flushccb();          // Clear Circ buffer after receiving File
//                            bCommPriority = 0;
//                            newMessage = 1;
//                            MsgCnt = 0;
//                            program_bit=0;
//                            ProgramMode=0;
//                            //DispAt(17,"CRC match");
//                            printf("CRC matched\n");
//                            //Delay(10000000);
//                            goto label1;
//                        }
//                        else
//                        {
//                            //DispAt(17,"CRC Not match");
//                            //Delay(10000000);
//                            printf("CRC not matched\n");
//                            sFrame.reply=NAK;
//                            if(packetno == totalpacket)
//                                logicSize=0;
//                            goto label1;
//                        }
//                    }
//                    else
//                        sFrame.reply = ACK;
//
//                    //fnDebugMsg("\n3");
//                    goto label1;
//                }
//
//
//                //fnDebugMsg("\n4");
//                while(isize < sFrame.dataSize)
//                {
//                    if(circBuffer.cnt > 0)
//                    {
//                        pCurPtr[isize] = comin();   // Till sent size (in structure) get data
//                        isize++;                    // PLC file gets copied from cir buffer
//                    }                               // to plcLogicFile buffer here only
//                    if(isize == sFrame.dataSize)
//                        break;
//                    if (circBuffer.head == circBuffer.tail)
//                    {
//                        bNotCompleted = 1;            // Message not received properly
//                        break;
//                    }
//                }
//
//                if(!bNotCompleted)								/* if total data has came */
//                {
//                    //WriteDebugMsg("\n5");
//                    isize = 0;     							// If reception O.K. then initialize byte counter
//                    newMessage = 1;
//                    MsgCnt = 0;
//                    flushccb();
//                    chkSum = cal_crc(pCurPtr,sFrame.dataSize); 			/* calculate data bytes checksum */
//                    bCommPriority = 0;
//                    if(chkSum != sFrame.checkSum)  						// if checksum matches then ack
//                    {
//                        //WriteDebugMsg("\n6");
//                        sFrame.reply = NAK;	        						// NACK for mismatch
//                        for(count =0;count<sFrame.dataSize;count++)
//                        {
//                            pCurPtr[count] = 0;     						// clear received buffer if chksum doesnot matches
//                        }
//                    }
//                    else
//                    {
//                        //WriteDebugMsg("\n7");
//                        sFrame.reply = ACK;
//                    }
//
//label1:
//                    sFrame.address = gpcbplcCnfg.mRtuAddr;
//                    sFrame.dataSize = 0;
//                    sFrame.checkSum = 0;
//                    memmove(transBuf,&sFrame,FRAMESIZE); 				/* prepare the transmit buffer */
//                    transflag = 1;
//                    translen = FRAMESIZE;
//
//                    curStatus = WAIT_FOR_FRAME;
//                    //WriteDebugMsg("\n8");
//                    if(sFrame.reply == ACK)
//                    {
//                        if(sFrame.info == DATETIME)
//                        {
//                            SetDateTime(); 							// set date and time for data time protcol
//                        }
//                        if(sFrame.info == PLCFILE)
//                        {
//                            plcFileRec = 1;
//                            if(sCurConfig.plcFile == 1)
//                            {
//                                if(sCurConfig.rcpFile == 0)
//                                {
//                                    if(sCurConfig.profile == 0)
//                                    {
//                                        if(sCurConfig.limits == 0)
//                                        {
//                                            if(sCurConfig.overRide == 0)
//                                            {
//                                                if(sCurConfig.anaSkip == 0)
//                                                {
//                                                    PlcFlag = 1; /* make the plcflag on to start the logic execution */
//                                                    prevPlcFlag = PlcFlag;
//                                                }
//                                            }
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                        if(sFrame.info == RCPFILE)
//                        {
//                            if (sFrame.reply == ACK)
//                                plcFileRec = 1;         // PLC file received flag
//                            transBuf[0] = 1; transBuf[1] = RCPFILE;
//                            transBuf[2] = 0; transBuf[3] = 0;
//                            transBuf[4] = 0; transBuf[5] = 0;
//                            transBuf[6] = ACK;
//
//                            printf("\n10");
//                            printf("Pkt %02d of %02d Rcv",(int)packetno,(int)totalpacket);
//                            //DispAt(1,dispStr);
//                            if(packetno == totalpacket)
//                            {
//                                //ClearDisp();
//                                //DispAt(1,".REC Received ");
//                                printf(".REC Received ");
//                                logicSize=0;
//                                transflag = 0;
//
//                                //								EPROM.FactoryDef=1;
//                                Restart_Device=1;
//                                SaveToDisk=1;
//                                //InsertIntoFlashBuff(__LINE__);
//                            }
//                            else
//                            {
//                                curStatus = WAIT_FOR_DATA;
//                                transBuf[7] = packetno;
//                                transflag = 1;
//                                translen = FRAMESIZE;
//                            }
//                            flushccb();
//                        }
//                        if(sFrame.info == PROFILE)
//                        {
//                            if(plcFileRec == 1)
//                            {
//                                if(sCurConfig.limits == 0)
//                                {
//                                    if(sCurConfig.overRide == 0)
//                                    {
//                                        if(sCurConfig.anaSkip == 0)
//                                        {
//                                            PlcFlag = 1;
//                                            prevPlcFlag = PlcFlag;
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                        if(sFrame.info == LIMITS)
//                        {
//
//                            //ClearDisp();
//                            //DispAt(2,"Limits Success");
//                            printf("Limits Success");
//                            if(plcFileRec == 1)
//                            {
//                                if(sCurConfig.overRide == 0)
//                                {
//                                    if(sCurConfig.anaSkip == 0)
//                                    {
//                                        PlcFlag = 1;
//                                        prevPlcFlag = PlcFlag;
//                                    }
//                                }
//                            }
//                        }
//                        if(sFrame.info == OVERRIDE)
//                        {
//                            if(plcFileRec == 1)
//                            {
//                                if(sCurConfig.anaSkip == 0)
//                                {
//                                    PlcFlag = 1;
//                                    prevPlcFlag = PlcFlag;
//                                }
//                            }
//                        }
//                        if(sFrame.info == HDDBINFO)
//                        {
//                            SetHwInfo();     // Call hardware information function
//                        }
//                        if(sFrame.info == CHANGEVAL)
//                        {
//                            if (ProcessCommands() != 0)
//                            {
//                                transBuf[0] = gpcbplcCnfg.mRtuAddr;
//                                transBuf[1] = 21;
//                                transBuf[2] = 0; transBuf[3] = 0;
//                                transBuf[4] = 0; transBuf[5] = 0;
//                                transBuf[6] = 0; transBuf[7] = 0;
//                                transflag = 1;
//                                translen = FRAMESIZE;
//                            }
//                        }
//                        if(sFrame.info == ANASCANSKIP)
//                        {
//                            if(plcFileRec == 1)
//                            {
//                                PlcFlag = 1;
//                                prevPlcFlag = PlcFlag;
//                            }
//                        }
//                        //WriteDebugMsg("\n9");
//                        if (sFrame.info == PLCLOGICXFER)
//                        {
//                            if (sFrame.reply == ACK)
//                                plcFileRec = 1;         // PLC file received flag
//                            transBuf[0] = 1; transBuf[1] = PLCLOGICXFER;
//                            transBuf[2] = 0; transBuf[3] = 0;
//                            transBuf[4] = 0; transBuf[5] = 0;
//                            transBuf[6] = ACK;
//
//                            printf("\n10");
//                            printf("Pkt %02d of %02d Rcv",(int)packetno,(int)totalpacket);
//                            //DispAt(1,dispStr);
//                            if(packetno == totalpacket)
//                            {
//                                //DispAt(1," Logic Received ");
//                                printf("\nLogic Received");
//                                logicSize=0;
//                                transflag = 0;
//                            }
//                            else
//                            {
//                                printf("\n14");
//                                curStatus = WAIT_FOR_DATA;
//                                transBuf[7] = packetno;
//                                transflag = 1;
//                                translen = FRAMESIZE;
//                            }
//                            flushccb();
//                        }
//                        if (sFrame.info == SMSRECEIVE)
//                        {
//                            sFrame.reply = ACK;
//                            transBuf[0] = 1; transBuf[1] = 55;
//                            transBuf[2] = 0; transBuf[3] = 0;
//                            transBuf[4] = 0; transBuf[5] = 0;
//                            transBuf[6] = 0; transBuf[7] = 0;
//                            transflag = 1;
//                            translen = FRAMESIZE;
//                            //DispAt(17, "SMSstr Rec");
//                            printf( "SMSstr Rec");
//                        }
//                    }
//                }
//            }
//        }
//        else    /* timeout */
//        {
//        }
//        break;
//    }
//    default:
//    {
//        curStatus = WAIT_FOR_FRAME;
//        //WriteDebugMsg("\n13");
//        break;
//    }
//    }                                     // End of main switch statement
//out:
//    ;
//}
////End of receive function

extern unsigned char MyPort3;
/*****************************************************************************/
/*      TransmitHandler(): Transmits the transmit buffer.					 */
/*		This function transmits the transmit buffer, if transmitflag is made on.
                This flag will be made on by the RcvHandler(). It will transmits translen
                much bytes from the transBuf. These all variables are set from RcvHandler.*/
/*****************************************************************************/
void TransmitHandler()
{
#if 0
    if(transflag)
    {
        if(sFrame.info != PLCLOGICXFER)
        {
            transBuf[7] = transBuf[0] + transBuf[1] + transBuf[2] + transBuf[3] + transBuf[4] + transBuf[5] + transBuf[6];
        }

        fnWrite(MyPort3,transBuf,translen);

        fnDebugMsg("\n................Tx.............");
        transflag = 0;
    }
#endif
}

/*****************************************************************************/
/*      MakeBlock(): Makes the data block to be transmit to the master       */
/*		This function makes the data block to be transmit to the master from
                dig_bit_array and signal_arr for digital and analog data respectively.
                The block consists of following things.
                maximum analog data in the block
                maximum digital data in the block
                analog data
                digital data in packed form. 										 */
/*****************************************************************************/

void MakeBlock()
{
//    float *ptr1 = NULL;
//
//    ptr1 = (float *)dummyDataPtr;
//
//    putInt(maxAna, dummyDataPtr);                    // prepare no of analog channels
//    putInt(maxDig, &dummyDataPtr[2]);                // prepare no of digital channels
//    memmove(&dummyDataPtr[4], (char *)signal_arr, sizeof(float)*maxAna);   // move anlog data
//    pack(dig_bit_array, dummyDataPtr+sizeof(float)*(maxAna+1), maxDig);   // mov digital data
}

/*****************************************************************************/
/*      PutInt(): Place the Integer value in the buffer in reverse manner.	 */
/*		This function exchanges the bytes of an integer. It makes the higher byte
                of an integer as lower byte and lower byte to the higher byte. Interpretation
                of an integer is different in PC and 8051. That's why it is necessary to
                reverse the bytes of an integer before transmit it to the other end. */
/*****************************************************************************/

void putInt(short int intVal,char *buf)
{
    buf[0] = intVal;
    buf[1] = (intVal>>8);
}


/*****************************************************************************/
/*      cal_crc(): calculates the checksum byte from the given information.	 */
/*		This function calculates the checksum for given char buffer of given
                length. */
/*****************************************************************************/


unsigned short int cal_crc(unsigned char *ptr1,unsigned short int count)
{
    crc_rem1 = 0xffff;
    for(bias1=0; bias1<count;bias1++)
    {
        crc_rem1 = crc_rem1 ^ ptr1[bias1];
        for(inner1=0;inner1<=7;inner1++)
        {
            mod1 = crc_rem1 & 0x0001;
            crc_rem1 >>= 1;
            if(mod1)
            {
                crc_rem1 = crc_rem1 ^ 0xa001;
            }
        }
    }
    crc = crc_rem1;
    gChkSum = crc;
    return crc;
}

extern unsigned char ReadByterec(unsigned char *pFile,unsigned long address);

unsigned short int plc_cal_crc(unsigned short int ptr1,unsigned short int count)
{
//    unsigned char tempdata;
////    unsigned char * MyptrFile;
////    unsigned char ucMimeType = 5;
////    unsigned long file_length ;
//
//    //DispAt(17,"CRC check...");
//    printf("CRC check....\n");
//
//    crc_rem1 = 0xffff;
//
//    if(ptr1 == 1)
//    {
//        //MyptrFile = uOpenFile("6.BIN");
//        //file_length =  uGetFileLength(MyptrFile);
//    }
//
//    if(ptr1 == 2)
//    {
//        //MyptrFile = uOpenFile("U.BIN");
//        //file_length =  uGetFileLength(MyptrFile);
//    }
//
//    for(bias1=0; bias1<count;bias1++)
//    {
//        //tempdata=ReadByterec(MyptrFile,(bias1));
//        //sprintf(temparr,"\ntempdata[%u] = %u",bias1,tempdata);
//        //fnDebugMsg(temparr);
//        crc_rem1 = crc_rem1 ^ tempdata;
//        for(inner1=0;inner1<=7;inner1++)
//        {
//            mod1 = crc_rem1 & 0x0001;
//            crc_rem1 >>= 1;
//            if(mod1)
//            {
//                crc_rem1 = crc_rem1 ^ 0xa001;
//            }
//        }
//    }
//
//    crc = crc_rem1;
//    gChkSum = crc;
//
//    return crc;
	return 0; // temp
}

short int cal_crc1(unsigned char *ptr1, short int count)
{
    short int crc=0;
    while (--count >= 0)
    {
        crc = crc + *ptr1;
        ptr1++;
    }
    return (short int)crc;
}

/*****************************************************************************/
/*		WriteData(): Writes data to the serial port for transmitting         */
/*		This function writes one by one data byte to the serial port.
                The data buffer and length of buffer is passed as parameters.		 */
/*****************************************************************************/

void WriteData(char *dataPtr,short int len)
{
    short int i;
    for (i=0;i<len;i++)
    {
        //PCBPLCComPutchar(dataPtr[i]);	//  sending data to the serial port 3.
    }
}

/*****************************************************************************/
/*      comin(): retreives one character from circular buffer				 */
/*		This function retreives a character in curchar from the circular buffer
                from head position. It adjusts the head, if it is more than buffer size
                resets it to zero. 													 */
/*****************************************************************************/
char comin(void)
{
    short int curr=0;

    if(circBuffer.head != circBuffer.tail)
    {
        curr = circBuffer.head;
        curchar = circBuffer.Adata[curr] ;
        circBuffer.head++;
        if(circBuffer.head >= BSIZE)  /* if at the end of list */
        {
            circBuffer.head = 0;
        }
        circBuffer.cnt--;
    }
    else
    {
        curchar = -1;
    }
    return curchar;
}

/*****************************************************************************/
/*      flushccb(): flushes the circular buffer								 */
/*****************************************************************************/
void flushccb(void)
{
    circBuffer.head = circBuffer.tail = circBuffer.cnt = 0;
}

/*****************************************************************************/
/*      pack(): packs the digital data										 */
/*		This function packs digital data. Unpacked character buffer and length
                is passed. It will put the pack data in passed pack buffer.			 */
/*****************************************************************************/
void pack(char *unpack, char *pack, short int no_chls)
{
    char *temp_unpack = unpack;
    char *temp_pack = pack;
    char temp_var,k;
    short int i = 0;

    while(i < no_chls)
    {
        (*temp_pack) = 0;

        for(k = 0; k < 8 ; ++k,i++,temp_unpack++)
        {
            if(*temp_unpack)
            {
                temp_var = 1;
            }
            else
            {
                temp_var = 0;
            }

            (*temp_pack) = ((temp_var) << k) | (*temp_pack);
        }

        temp_pack++;
    }
}

/*****************************************************************************/
/*      SetDigitalOutput(): Sets the digital output on the hardware			 */
/*		This function sets a perticular digital output from the structure came
                from the master.													 */
/*****************************************************************************/
void SetDigitalOutput(short int i)
{
    if(sChangeVal[i].value[0] == 0)
    {
        /* TODO : Add proper logic to handle DO */
        //(EPROM.DOSignal[sChangeVal[i].index]) = 0;
    }
    else
    {
        /* TODO : Add proper logic to handle DO */
        //(EPROM.DOSignal[sChangeVal[i].index]) = 1;
    }
}

/*****************************************************************************/
/*      SetPlcVar(): Sets a perticular Plc Variable 						 */
/*		This function sets a plc variable from the structure came from the
                master.																 */
/*****************************************************************************/

void SetPlcVar(short int i)
{
    float *val;
    val = (float *) sChangeVal[i].value;     // stores the value of in plcvararr
    plcVarArr[sChangeVal[i].index] = *val;   // This is the float value
}

/*****************************************************************************/
/*      SetHwInfo(): Sets the hardware information							 */
/*		This function initializes some necessary variables as per the hardware
                information came from the master.									 */
/*****************************************************************************/
void SetHwInfo()
{    
    maxAna = 0;
    if(sCurConfig.AI > 0)
    {
        AiFlag = 1;
        for(count=0;count<sCurConfig.AI;count++)
        {
            if(sCurConfig.cardType[count] == 0)
                maxAna += 16;
            else maxAna += 8;
        }
    }
    if(sCurConfig.DI > 0)
    {
        DiFlag = 1;
        maxDig = sCurConfig.DI * 16;
    }
    sCurConfig.maxPid = MAX_PID;
    sCurConfig.maxProfile = MAX_PROFILE;    
}


short int ProcessCommands()
{
    short int i,noCommands,size;
    size = sizeof(sChangeVal[0]);
    noCommands = dataSize/size;
    for(i=0;i<noCommands;i++)		  //noCommands
    {
        if(sChangeVal[i].type == DIGOUT)
        {            
            //if(RTU_DO_MODE_AUTO == gpcbplcCnfg.mRtuDoMode)//auto modeEPROM_General.DoModeDetails.Do_Mode
            if(RTU_DO_MODE_AUTO == EPROM_General.DoModeDetails.Do_Mode)//auto mode
            {                                   // 1 value inidicates in auto mode
                if (overRideFlagArr[sChangeVal[i].index] != 1)
                    return 0;
            }
            SetDigitalOutput(i);
        }
        else if(sChangeVal[i].type == ANAOUT)
        {
            sChangeVal[i].index++;
        }
        else if(sChangeVal[i].type == SETPLCVAR)
        {
            SetPlcVar(i);

        }
    }
    return 1;
}


void SetDateTime()
{
}

//********************************************************************************
// This is the function is used for giving alarm no.s and its corresponding
// SMS from PC. 
//********************************************************************************
void ParseString(char *ptrStr)
{
    short int index = 0,nCtr=0;
    while(*ptrStr != '\0')
    {
        index=0;
        if(*ptrStr == ';')
        {
            ptrStr++;
            while(*ptrStr != ':')
            {
                csStrList[nCtr][index++] = *ptrStr;
                ptrStr++;
            }
            csStrList[nCtr][index]='\0';
        }
        nCtr++;
        ptrStr++;
    }
}

//******************************************************************************     
// This function takes total .PLC file from circ buffer and allocates that 
// finally in buffer[BUFFER_SIZE] array. The file is directly extracted and allocated
// in this same function.
//******************************************************************************
void Extract_Alloc(void)
{
	if(PLC_RPOG_Flag != 1)
    {
		/* Initializes all .PLC variables before allocation */
        InitGlobalVar();

    	if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
    	{
            /* for downloading .PLC file initially */
    		/* @todo: Need to handle below commented condition */
            //if((plcFileRec) || (firstTime))
            {
                plcFileRec = 0;
                firstTime = 0;
                memIndex = 0;

                WriteLog(pcbplc_logger, "Extracting PLC file\r\n", LOG_INFO);

                if(PcbPlc_FileReadAndParse(pcbplc_logger, gPlcFile, gRecFile))
                {
                    WriteLog(pcbplc_logger, "Plc File Decode Error\r\n", LOG_ERROR);
                    PlcFlag = 0;                            // File decode error
                    PLCCOMM = 0;
                    xSemaphoreGive(sendExternalFlashSemaphore);
                    return;
                }

                seqStep = seqStartNode;               // sequence start node
                emrStep = emrStartNode;               // Emr start node
                intStep = intStartNode;               // Int start node

                if(!mem_alloc())
                {
                    WriteLog(pcbplc_logger, "Memero Alloc failed\r\n", LOG_ERROR);
                    PlcFlag = 0;                // memory allocation is unsuccessful
                    xSemaphoreGive(sendExternalFlashSemaphore);
                    return;
                }

                PcbPlc_InitAllPlcTimers();

                int_arr = (short int *) (PlcAlloc(pcbplc_logger, MAX_INTEGER_ARR_SIZE*sizeof(short int)));
                if(!int_arr)
                {
                    WriteLog(pcbplc_logger, "Alloc Error\r\n", LOG_ERROR);
                    PlcFlag = 0;
                }

                int_arr[0]=0;						/* RTU ok flag */
                //int_arr[1]=1;						/* auto manual mode */
                int_arr[2]=totalBits;				/* maximum digital channels */
                int_arr[3]=totalFloats;				/* maximum analog channels */
                int_arr[4]=0; 						/* for seq file check sum */
                int_arr[5]=0; 						/* not used */
                int_arr[6]=0;    					/* max plc seq var */
                int_arr[7]=10;						/* max back log */
                int_arr[8]=-1;						/* physical auto-manual channel index */
                int_arr[9]=0;						/* feddback delay from newdata */
                int_arr[10]=0;						/* master ok flags */
                int_arr[11]=0;						/* PLC SEQUENCE STEP NO */
                int_arr[12]=0;						/* not used */
                int_arr[13]=0;						/* not used */
                int_arr[14]=0;						/* not used */
                int_arr[1]=0;						/* auto manual mode */
            }
    		xSemaphoreGive(sendExternalFlashSemaphore);
    	}


    }
}


void SetupCircBuf()
{
    circBuffer.cnt = 0;
    circBuffer.tail = 0;
    circBuffer.head = 0;
    circBuffer.nearfull = 0;
    circBuffer.size = 0;
}
