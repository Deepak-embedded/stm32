/*
 * OTA.h
 *
 *  Created on: Jan 5, 2023
 *      Author: Shreyanss
 */

#ifndef INC_OTA_H_
#define INC_OTA_H_

//#include "libs.h"
#include "w25q_mem.h"
#include "json_parser.h"

typedef enum {
    OTA_FAIL = 0,
	OTA_SUCCESS,
} OTA_FILE_ACK;

typedef enum {
    PLC = 0,        //.PLC File
	REC, 			//.REC File
	HEX,			//.bin File
	DELTA_HEX_3100,	// delta bin File for 3100
	DELTA_HEX_1100	// delta bin File for 1100
} OTA_FILE_TYPE;

typedef enum {
    START = 0,			//Start OTA
	IN_PROGRESS,		//In Progress
	VERIFICATION,		//Verification
	FINISH,				//Finish
} OTA_STATE;

struct OTA
{
	int CMDState; //1
	int FileType; //1
	int Chunk_no; //1
	int Chunk_Size; //512
	int crc;
	int FileSize;
};


struct OTA_ACK
{
	int cmd;
	int CMDState;
	int Chunk_no;
	int FileType;
	char OtaACK;
};

typedef struct OTA_hex_Data
{
	unsigned char checkbyte;							//1
	unsigned char ChecksumOfStuct;						//1+1=2
	uint16_t SizeOfStuct;								//2+2=4
	char OTA_State;
	char HEX_Crc;
	long int File_Size;
	unsigned char bootloaderVersion[100];
	uint8_t sessionId;
	uint16_t OTApushTime;
	uint16_t forVer;
	uint32_t DeltaSize;
	unsigned char DeltaCRC;
	int FileType;
	char TgtHEX_Crc;
	long int TgtFile_Size;
	char extrs_reserved[85];
}OTA_hex_Data_t;

extern struct OTA_ACK OTA_ACK_Data;

char OTA_HexChartoHexByte(unsigned char a);
void Ota_File_write_ack(COM_TYPE com_mode,int fileType,int chunk_number,int chunk_lenth,char *data ,char * ACK_Response);
unsigned char verify_OTA_CRC(unsigned char crc,unsigned long int length,unsigned int start_address);
unsigned char convert_OTA_HextoAsciiString(char* i_HexString, char* O_AsciiString);
JSON_ERROR_RESPONSE LOta_Status_ack(COM_TYPE com_mode, int sessionID, char * ACK_Response);

#endif /* INC_OTA_H_ */
