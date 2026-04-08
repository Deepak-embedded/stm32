/*
 * OTA.c
 *
 *  Created on: Jan 5, 2023
 *      Author: Shreyanss
 */
#include "OTA.h"
#include "main.h"
#include "stdlib.h"
#include "json_parser.h"
#include "pcbplcService.h"

int send_OTA_status = 0;
int MQTT_send_OTA_status = 0;
extern struct OTA_hex_Data OTA_store_Data;
extern struct OTA OTA_Data;
extern W25Q_STATE W25Q_Write_continous(u8_t *buf, u16_t len, u32_t pageShift);
JSON_ERROR_RESPONSE response_ACK_OTA_JSON_frame(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response);
extern int OTAStart;
extern char msg_payload[1400];
#define Write_cyc_delay 10
char crcMatch = 0;
extern int CRC_value;
struct OTA_ACK OTA_ACK_Data;
extern int gPlcFileLength;
extern int gRecFileLength;
extern unsigned long int ghexFileLength;
extern unsigned char PLC_RPOG_Flag;
extern unsigned char REC_RPOG_Flag;
extern uint8_t gsessionId;
extern uint16_t gOTApushTime;
extern uint16_t gforVer;
extern uint32_t gDeltaSize;
extern unsigned char gDeltaCRC;

// make response and send it address in function
// Details of variable
// 			fileType 		selection of which file is received
//			chunk_number 	number of chunk
//			chunk_lenth 	size of length of chunk
//			*data			received data of file
//			* ACK_Response  make json response and write it for return
void Ota_File_write_ack(COM_TYPE com_mode,int fileType,int chunk_number,int chunk_lenth,char *data ,char * ACK_Response)
{
	JSON_ERROR_RESPONSE JSON_ret;
	W25Q_STATE FLASH_ret;

	OTA_ACK_Data.cmd = CMD_OTA;
	OTA_ACK_Data.CMDState = OTA_Data.CMDState;
	OTA_ACK_Data.Chunk_no = OTA_Data.Chunk_no;
	OTA_ACK_Data.FileType = OTA_Data.FileType;

	if(OTA_ACK_Data.CMDState == 2) // for crc check
	{
		OTAStart = 0;
		switch(fileType)
		{
			case PLC:
			{
				unsigned char result =0;
				result = verify_OTA_CRC(CRC_value,gPlcFileLength,PUB_FILE_START_ADDRESS);
				osDelay(1);
				if(1 == result)
				{
					result = verify_OTA_CRC(CRC_value,gPlcFileLength,PUB_FILE_START_ADDRESS);
					if(1 == result)
					{
						crcMatch =0;
						OTA_ACK_Data.OtaACK = OTA_FAIL;
					}
				}
				else
				{
					OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					gPlcRecFlash.mPlcFileLength = gPlcFileLength;
					gPlcRecFlash.mPlcFileCRC = CRC_value;
					crcMatch=1;
					PLC_RPOG_Flag=1;
					ExtFlash_update_gPlcRecFlash();
					osDelay(Write_cyc_delay);
				}
			}
			break;
			case REC:
			{
				unsigned char result =0;
				result = verify_OTA_CRC(CRC_value,gRecFileLength,REC_FILE_START_ADDRESS);
				osDelay(1);
				if(1 == result)
				{
					result = verify_OTA_CRC(CRC_value,gRecFileLength,REC_FILE_START_ADDRESS);
					if(1 == result)
					{
						crcMatch =0;
						OTA_ACK_Data.OtaACK = OTA_FAIL;
					}
				}
				else
				{
					OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					gPlcRecFlash.mRecFileLength = gRecFileLength;
					gPlcRecFlash.mRecFileCRC = CRC_value;
					crcMatch=1;
					REC_RPOG_Flag=1;
					gPlcRecFlash.extract_receipe = 1;
					ExtFlash_update_gPlcRecFlash();
				}
			}
			break;
			case HEX:
			case DELTA_HEX_3100:
			case DELTA_HEX_1100:
			{
				unsigned char result = 0;
				unsigned int address;
				unsigned long int FileLength = gDeltaSize;
				unsigned char crc = gDeltaCRC;

				if(fileType == HEX)
				{
					address = HEX_FILE_START_ADDRESS;
					FileLength = ghexFileLength;
					crc = CRC_value;
				}
				else if(fileType == DELTA_HEX_3100)
					address = DELTA_HEX_3100_START_ADD;
				else
					address = DELTA_HEX_1100_START_ADD;

				result = verify_OTA_CRC(crc,FileLength,address);
				if(1 == result)
				{
					osDelay(1);
					result = verify_OTA_CRC(crc,FileLength,address);
					if(1 == result)
					{
						crcMatch =0;
						OTA_ACK_Data.OtaACK = OTA_FAIL;
					}
				}
				if(0 == result)
				{
					OTA_store_Data.File_Size = ghexFileLength;
					OTA_store_Data.HEX_Crc = CRC_value;
					OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					if(fileType == HEX)
					{
						OTA_store_Data.OTA_State = OTA_ACK_Data.CMDState;
					}
					else		// Delta Hex
					{
						OTA_store_Data.DeltaSize = gDeltaSize;
						OTA_store_Data.DeltaCRC = gDeltaCRC;
						OTA_store_Data.sessionId = gsessionId;
						OTA_store_Data.OTApushTime = gOTApushTime;
						OTA_store_Data.forVer = gforVer;
						OTA_store_Data.TgtFile_Size = ghexFileLength;
						OTA_store_Data.TgtHEX_Crc = CRC_value;
						OTA_store_Data.FileType = OTA_Data.FileType - 2;
					}
					crcMatch=1;
					ExtFlash_update_OTA_Data();
				}
			}
			break;
			default:
			{

			}
			break;
		}
	}
	else
	{
		if(data && OTAStart)
		{
			switch(fileType)
			{
				case PLC:
				{
					if(chunk_number !=0)
					{
						if(data)
						{
							char *OTA_buffer;
							if(1 == chunk_number)
							{
								FLASH_ret = Erase_RECsector(PUB_FILE_START_ADDRESS,PUB_FILE_TOTAL_SEC);

								if(FLASH_ret != W25Q_OK)
								{
									FLASH_ret = Erase_RECsector(PUB_FILE_START_ADDRESS,PUB_FILE_TOTAL_SEC);
								}
							}

							OTA_buffer = (char *) pvPortMalloc((strlen(data))/2);
							memset(OTA_buffer,0,(strlen(data))/2);
							if((convert_OTA_HextoAsciiString(data,OTA_buffer))==0)
							{
								if(com_mode == MQTT)
								{
									MX25L_Erase_Write_One_Sector((u8_t*)OTA_buffer,(strlen(data))/2,PUB_FILE_START_ADDRESS+((chunk_number-1)*4096));
								}
								else
								{
									FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, 512, PUB_FILE_START_ADDRESS+((chunk_number-1)*512)); ///< Sector erase, save in flash
									osDelay(Write_cyc_delay);

									if(FLASH_ret != W25Q_OK)
									{
										FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer,512, PUB_FILE_START_ADDRESS+((chunk_number-1)*512)); ///< Sector erase, save in flash
										osDelay(Write_cyc_delay);
										if(FLASH_ret != W25Q_OK)
										{
											OTA_ACK_Data.OtaACK = OTA_FAIL;
										}
									}
								}
							}
							if(FLASH_ret == W25Q_OK)
							{
								OTA_ACK_Data.OtaACK = OTA_SUCCESS;
							}
							vPortFree(OTA_buffer);
						}
					}
					else
					{
						OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					}
				}
				break;
				case REC:
				{
					if(chunk_number !=0)
					{
						if(data)
						{
							char *OTA_buffer;
							if(1 == chunk_number)
							{
								FLASH_ret = Erase_RECsector(REC_FILE_START_ADDRESS,REC_FILE_TOTAL_SEC);
								if(FLASH_ret != W25Q_OK)
								{
									FLASH_ret = Erase_RECsector(REC_FILE_START_ADDRESS,REC_FILE_TOTAL_SEC);
								}
							}
							OTA_buffer = (char *) pvPortMalloc((strlen(data))/2);
							memset(OTA_buffer,0,(strlen(data))/2);
							if((convert_OTA_HextoAsciiString(data,OTA_buffer))==0)
							{
								if(com_mode == MQTT)
								{
									MX25L_Erase_Write_One_Sector((u8_t*)OTA_buffer,(strlen(data))/2,REC_FILE_START_ADDRESS+((chunk_number-1)*4096));
								}
								else
								{
									FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, strlen(data)/2, REC_FILE_START_ADDRESS+((chunk_number-1)*512)); ///< Sector erase, save in flash
									osDelay(Write_cyc_delay);

									if(FLASH_ret != W25Q_OK)
									{
										FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, strlen(data)/2, REC_FILE_START_ADDRESS+((chunk_number-1)*512)); ///< Sector erase, save in flash
										osDelay(Write_cyc_delay);
										if(FLASH_ret != W25Q_OK)
										{
											OTA_ACK_Data.OtaACK = OTA_FAIL;
										}
									}
									if(FLASH_ret == W25Q_OK)
									{
										OTA_ACK_Data.OtaACK = OTA_SUCCESS;
									}
								}
							}
							vPortFree(OTA_buffer);
						}
					}
					else
					{
						OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					}
				}
				break;
				case HEX:
				{
					if(chunk_number !=0)
					{
						if(data)
						{
							char *OTA_buffer;

							if(com_mode == MQTT)
							{

							}
							else
							{
								if((1 == chunk_number) || (((chunk_number-1)%8)==0))
								{
									FLASH_ret = Erase_RECsector((HEX_FILE_START_ADDRESS+((chunk_number-1)*512)),1);

									if(FLASH_ret != W25Q_OK)
									{
										FLASH_ret = Erase_RECsector((HEX_FILE_START_ADDRESS+((chunk_number-1)*512)),1);
									}
								}
							}

							OTA_buffer = (char *) pvPortMalloc((strlen(data)/2));
							memset(OTA_buffer,0,(strlen(data))/2);

							if((convert_OTA_HextoAsciiString(data,OTA_buffer))==0)
							{
								if(com_mode == MQTT)
								{
									MX25L_Erase_Write_One_Sector((u8_t*)OTA_buffer,(strlen(data))/2,HEX_FILE_START_ADDRESS+((chunk_number-1)*4096));
								}
								else
								{
									FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, 512, (HEX_FILE_START_ADDRESS+((chunk_number-1)*512)));
									osDelay(500);
									if(FLASH_ret != W25Q_OK)
									{
										FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, 512, (HEX_FILE_START_ADDRESS+((chunk_number-1)*512)));
										osDelay(500);
										if(FLASH_ret != W25Q_OK)
										{
											OTA_ACK_Data.OtaACK = OTA_FAIL;
										}
									}
									if(FLASH_ret == W25Q_OK)
									{
										OTA_ACK_Data.OtaACK = OTA_SUCCESS;
									}
								}
							}
							vPortFree(OTA_buffer);
						}
					}
					else
					{
						OTA_ACK_Data.OtaACK = OTA_SUCCESS;
					}
				}
				break;
				case DELTA_HEX_3100:
				{
					if(chunk_number !=0 && com_mode == MQTT)
					{
						char *OTA_buffer;

						if((1 == chunk_number) || (((chunk_number-1)%8)==0))
						{
							FLASH_ret = Erase_RECsector((DELTA_HEX_3100_START_ADD+((chunk_number-1)*512)),1);

							if(FLASH_ret != W25Q_OK)
							{
								FLASH_ret = Erase_RECsector((DELTA_HEX_3100_START_ADD+((chunk_number-1)*512)),1);
							}
						}

						OTA_buffer = (char *) pvPortMalloc((strlen(data)/2));
						memset(OTA_buffer,0,(strlen(data))/2);

						if((convert_OTA_HextoAsciiString(data,OTA_buffer))==0)
						{
							FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, 512, (DELTA_HEX_3100_START_ADD+((chunk_number-1)*512)));
							osDelay(500);
							if(FLASH_ret != W25Q_OK)
							{
								FLASH_ret = W25Q_Write_continous((u8_t*)OTA_buffer, 512, (DELTA_HEX_3100_START_ADD+((chunk_number-1)*512)));
								osDelay(500);
								if(FLASH_ret != W25Q_OK)
								{
									OTA_ACK_Data.OtaACK = OTA_FAIL;
								}
							}
							if(FLASH_ret == W25Q_OK)
							{
								OTA_ACK_Data.OtaACK = OTA_SUCCESS;
							}
						}
						vPortFree(OTA_buffer);
					}
					else
					{
						OTA_ACK_Data.OtaACK = OTA_FAIL;
					}
				}
				break;
				default :
				{

				}
				break;

			}
		}
		else if(chunk_number ==0)
		{
			OTA_ACK_Data.OtaACK = OTA_SUCCESS;// For test only to remove check crc
		}
	}
	JSON_ret = response_ACK_OTA_JSON_frame(com_mode , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_Response);
	if(JSON_ret != JSON_SUCCESS)
	{
		JSON_ret = response_ACK_OTA_JSON_frame(com_mode , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_Response);
	}
}

JSON_ERROR_RESPONSE response_ACK_OTA_JSON_frame(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response)
{
	cJSON *response_json_Obejct = NULL;
	JSON_ERROR_RESPONSE ret;
	ret = JSON_SUCCESS;
	response_json_Obejct = cJSON_CreateObject();
	if (response_json_Obejct == NULL)
	{
		ret = FAILED_CREATE_JSON_OBJECT;
	}

	switch(CMD)
	{
		case CMD_OTA:
		{

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", OTA_ACK_Data.CMDState))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(OTA_ACK_Data.Chunk_no != 0)
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Chunk_no", OTA_ACK_Data.Chunk_no))
				{
					ret = FAILED_ADD_JSON_OBJECT;
				}
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "ACK", iACK))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case CMD_CONF:
		{

		}
		break;
		default:
		{

		}
		break;
	}

	switch(com_mode)
	{
		case MQTT:
		{
			cJSON_AddNumberToObject(response_json_Obejct, "client_id", EPROM_General.Cust_Detail.Client_Id);
			cJSON_AddNumberToObject(response_json_Obejct, "group_id", EPROM_General.Cust_Detail.Reader_Id);
			cJSON_AddNumberToObject(response_json_Obejct, "rtu_id", EPROM_General.Rtu_Detail.RTUId);
			memset(&MqttPubBuf,0x00,sizeof(MqttPubBuf));
			if(!(cJSON_PrintPreallocated(response_json_Obejct, (char *)&MqttPubBuf,sizeof(MqttPubBuf),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			else
			{
				MQTT_send_OTA_status = 1;//TODO : Set flag
			}

			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}
		}
		break;
		case UART:
		{
			memset(msg_payload,0x00,sizeof(msg_payload));
			if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			strcpy(ACK_Response,msg_payload);

			send_OTA_status = 1;
			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}
		}
		break;
		default:
		{

		}
		break;
	}
	return ret;
}

JSON_ERROR_RESPONSE response_ACK_JSON_frame1(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response) // For test only to remove check crc
{
	cJSON *response_json_Obejct = NULL;
	JSON_ERROR_RESPONSE ret;
	ret = JSON_SUCCESS;
	response_json_Obejct = cJSON_CreateObject();
	if (response_json_Obejct == NULL)
	{
		ret = FAILED_CREATE_JSON_OBJECT;
	}

	switch(CMD)
	{
		case CMD_OTA:
		case CMD_CONF:
		{
			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", 3))//OTA_ACK_Data.CMDState))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "ACK",1))// ACK))
			{
				ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;

		default:
		{

		}
		break;
	}

	switch(com_mode)
	{
		case MQTT:
		{
			cJSON_AddNumberToObject(response_json_Obejct, "client_id", EPROM_General.Cust_Detail.Client_Id);
			cJSON_AddNumberToObject(response_json_Obejct, "group_id", EPROM_General.Cust_Detail.Reader_Id);
			cJSON_AddNumberToObject(response_json_Obejct, "rtu_id", EPROM_General.Rtu_Detail.RTUId);
			memset(msg_payload,0x00,sizeof(msg_payload));
			if (!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			strcpy(ACK_Response,msg_payload);

			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}

		}
		break;
		case UART:
		{
			memset(msg_payload,0x00,sizeof(msg_payload));
			if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
			{
				ret = FAILED_TRANSFER_STRING_FROM_JSON_OBJECT;
			}
			strcpy(ACK_Response,msg_payload);
			UART_OTA_ACKflag=1;
			if(response_json_Obejct)
			{
				cJSON_Delete(response_json_Obejct);
				response_json_Obejct = NULL;
			}
		}
		break;
		default:
		{

		}
		break;
	}
	return ret;
}

unsigned char verify_OTA_CRC(unsigned char crc,unsigned long int length,unsigned int start_address)// check 8 bit crc
{
	uint8_t result=1;
	unsigned char cal_CRC = 0;
	unsigned char sum = 0;
	uint64_t i_index = 0;
	char RX_Byte[2] ={0};
	for(i_index = 0;i_index<length;i_index++)
	{
		W25Q_STATE ret;
		ret =MX25L_ReadRaw((u8_t*) RX_Byte, 1, (start_address+i_index));
		if(ret!=W25Q_OK)
		{
			ret = MX25L_ReadRaw((u8_t*) RX_Byte, 1, (start_address+i_index));
		}
		sum += RX_Byte[0];
	}
	cal_CRC=255-sum;
	sprintf(print, "verify_OTA_CRC: 0x%02x\r\n", cal_CRC);
	WriteLog(1, print, 1);
	if(cal_CRC==crc)
	{
		result=0;
	}
	return result;
}

/**************************************************************************//**
 * convertHextoAsciiString
 *****************************************************************************/
unsigned char convert_OTA_HextoAsciiString(char* i_HexString,char* O_AsciiString)
{
	unsigned char result=0,buf=0;
	unsigned int i_length,i_index,o_index=0;
	i_length=strlen(i_HexString);
	if(i_length%2==0)
	{
        for(i_index = 0; i_index < i_length; i_index++)
        {
        	if(i_index == 0)
        	{
        		buf = (HexChartoHexByte(i_HexString[i_index]))<<4;
        	}
        	else if(i_index % 2 != 0)
			{
				O_AsciiString[o_index++]= buf|HexChartoHexByte(i_HexString[i_index]);//hex_to_ascii(buf, HexChartoHexByte(i_HexString[i_index]));
			}
			else
			{
				buf = (HexChartoHexByte(i_HexString[i_index]))<<4;
			}
        }
	}
	else
	{
		result=1;
	}
	return result;
}

char OTA_HexChartoHexByte(unsigned char a)
{
    if (a >= '0' && a <= '9') {
        return (a - '0');
    }
    if (a >= 'A' && a <= 'F') {
        return ((a - 'A') + 10);
    }
    if (a >= 'a' && a <= 'f') {
        return ((a - 'a') + 10);
    }
    return -1;
}

JSON_ERROR_RESPONSE LOta_Status_ack(COM_TYPE com_mode, int sessionID, char * ACK_Response)
{
	JSON_ERROR_RESPONSE ret;
	ret = FAILED_ADD_JSON_OBJECT;

	if(sessionID == OTA_store_Data.sessionId)
	{
		cJSON *response_json_Obejct = NULL;

		response_json_Obejct = cJSON_CreateObject();
		if (response_json_Obejct == NULL)
		{
	        return ret;
		}

		cJSON *nodes_Array = cJSON_CreateArray();
	    if (nodes_Array == NULL)
	    {
	        cJSON_Delete(response_json_Obejct);
	        return ret;
	    }

		OTA_ACK_Data.cmd = CMD_LOTA_QUERY;
		OTA_ACK_Data.CMDState = 4;
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", OTA_ACK_Data.cmd))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", OTA_ACK_Data.CMDState))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "sessionId", sessionID))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "FileType", LoraOTA.ota_type ? LoraOTA.ota_type+2 : 0))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddStringToObject(response_json_Obejct, "gwID", LoRa_Modem.lora_serial_no))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "OTAstatus", LoraOTA.ota_state))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalChunk", LoraOTA.totalChunks))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CurrentChunk", LoraOTA.curentChunk))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		for(uint8_t idx = 0; idx < NoConnNodes; idx++)
		{
			if(!cJSON_AddItemToArray(nodes_Array, cJSON_CreateString(nodesCon[idx].macId)))
			{
		        cJSON_Delete(response_json_Obejct);
		        return ret;
			}
		}
		if(!cJSON_AddItemToObject(response_json_Obejct, "OTAnodes", nodes_Array))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		if(!cJSON_AddStringToObject(response_json_Obejct, "fwVer", EPROM_General.Rtu_Detail.Hex_Version))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}

		memset(msg_payload,0x00,sizeof(msg_payload));
		if (!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))
		{
	        cJSON_Delete(response_json_Obejct);
	        return ret;
		}
		strcpy(ACK_Response,msg_payload);

		if(response_json_Obejct)
		{
			cJSON_Delete(response_json_Obejct);
			response_json_Obejct = NULL;
		}
		ret = JSON_SUCCESS;
		return ret;
	}
	else
        return ret;

}
