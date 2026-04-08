/*
 * Configuration.c
 *
 *  Created on: Jan 9, 2023
 *      Author: Shreyanss
 */

#include "main.h"
#include "OTA.h"
#include "Configuration.h"

struct Save_Para_General EPROM_General;
struct Save_Para_AI_Calibration EPROM_AI_Calibration;
struct Save_Para_Schedule_Configuration EPROM_Schedule;
struct Save_Para_Modbus_Configuration EPROM_Modbus_Quary_Detail;
struct Save_Para_PCBPLC_General_Reg EPROM_PCBPLC_General_Reg;
struct Modbus_WriteQuary MODBUS_Write[1];
struct Configuration Config;
struct OTA_hex_Data OTA_store_Data;
struct Save_Para_Frequent EPROM_Frequent;
EPROM_Dync_t EPROM_Dync;

extern unsigned int tempLora_Frequency;
extern uint8_t tempLora_Spreading_Factor;
extern uint8_t tempLora_Bandwidth;
extern uint8_t tempLora_Code_Rate;
extern uint16_t tempLora_Preamble_Length;
extern uint8_t tempLora_TX_Power;

extern int Fream_id;
unsigned char mWriteQueryAck[MAX_WRITE_QUERY_ACK];
char tDebug[500];

//void ExtFlash_update_EPROM_General();
//void ExtFlash_update_EPROM_Schedule();
//void ExtFlash_update_EPROM_Modbus_Quary_Detail();
//void ExtFlash_Read_EPROM_General();
//void ExtFlash_Read_EPROM_Schedule();
//void ExtFlash_Read_EPROM_Modbus_Quary_Detail();
//void ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();
//void ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG();
//void syncExtFlashVariableWithPCBPLCVariable();
/**************************************************************************//**
 * Function name 	: calculateCheckSumOf_QSPI_Store_para
 * arguments		: 1) sizeofstruct
 * return 		 	: Checksum
 * Note				: calculate CheckSum Of QSPI_Store_para structure
 * 					  According to Size
 *****************************************************************************/

unsigned char calculateCheckSumOfStruct(unsigned char *structAddress,uint16_t sizeofstruct)
{
	unsigned char sum=0;
	unsigned char *cksum_ptr=structAddress;
	for(int i=4;i<sizeofstruct;i++)
	{
		sum+=cksum_ptr[i];
	}
//	sprintf((char *)print,"\r\nchecksum in function : %d",255-sum);
//	HAL_UART_Transmit(&huart1, print,strlen(print), 1000);
	return 255-sum;
}

/**************************************************************************//**
 * Function name 	: calculateCheckSumOf_QSPI_Store_para
 * arguments		: 1) sizeofstruct
 * return 		 	: Checksum
 * Note				: calculate CheckSum Of QSPI_Store_para structure
 * 					  According to Size
 *****************************************************************************/

void ExtFlash_Read_EPROM_General(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0,i=0;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_General, sizeof(EPROM_General), EPROM_GENERAL_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}


	if((EPROM_General.checkbyte != 0xAB)||(EPROM_General.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;

		sprintf((char *)print,"ExtFlash_Read_EPROM_General:EPROM_General.checkbyte:%d, %d\r\n",EPROM_General.checkbyte,EPROM_General.SizeOfStuct);
		WriteLog(1, print, 1);
//		Print_Flash();
	}
	else if(EPROM_General.SizeOfStuct<sizeof(EPROM_General))
	{
		if(EPROM_General.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_General,EPROM_General.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;

			sprintf((char *)print,"Stuct Size Change CRC mismatch : %d,%d,%d\r\n",EPROM_General.checkbyte,EPROM_General.SizeOfStuct,sizeof(EPROM_General));
			WriteLog(1, print, 1);
			//Print_Flash();
		}
	}
	else
	{
		if(EPROM_General.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_General,sizeof(EPROM_General)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
			sprintf((char *)print,"same size but CRC mismatch : %d,%d,%d\r\n",EPROM_General.checkbyte,EPROM_General.SizeOfStuct,sizeof(EPROM_General));
			WriteLog(1, print, 1);
		}
	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_General.checkbyte = 0xAB;
		EPROM_General.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_General,sizeof(EPROM_General));
		EPROM_General.SizeOfStuct = sizeof(EPROM_General);
		EPROM_General.rebootCount = 0;
		EPROM_General.LogRate = DEFAULT_LOGRATE;		// Min
		EPROM_General.History_En_Di = CONF_ENABLE;

		strcpy((char *)EPROM_General.Rtu_Detail.HW_Version,DEFAULT_HW_VERSION);
		strcpy((char *)EPROM_General.Rtu_Detail.Hex_Version,DEFAULT_FV_VERSION);
		strcpy((char *)EPROM_General.Rtu_Detail.PLC_Version,DEFAULT_PLC_VERSION);
		strcpy((char *)EPROM_General.Rtu_Detail.REC_Version,DEFAULT_REC_VERSION);
		EPROM_General.Rtu_Detail.RTUId = DEFAULT_RTUID;
		EPROM_General.slot_id=DEFAULT_SLOTID;
		EPROM_General.LoraRetryCount = DEFAULT_RETRY_COUNT;
		EPROM_General.LoraRetryDelay = DEFAULT_RETRY_DELAY;
		EPROM_General.LogSendDelay = DEFAULT_LOG_SEND_DELAY;
		EPROM_General.readCmdDelay = DEFAULT_READ_CMD_DELAY;

		strcpy((char *)EPROM_General.Cust_Detail.Proj_Code,DEFAULT_PROJECT_CODE);
		strcpy((char *)EPROM_General.Cust_Detail.Site_Name,DEFAULT_SITE_NAME);
		strcpy((char *)EPROM_General.Cust_Detail.Time_zone,DEFAULT_TIME_ZONE);
		EPROM_General.Cust_Detail.Tz_sign = DEFAULT_TIME_ZONE_SIGN;
		EPROM_General.Cust_Detail.Tz_hours = DEFAULT_TIME_ZONE_HOURS;
		EPROM_General.Cust_Detail.Tz_min = DEFAULT_TIME_ZONE_MINUTES;

		EPROM_General.Cust_Detail.reboot_day_night = DEFAULT_DAY_NIGHT_REBOOT;
		EPROM_General.Cust_Detail.Client_Id = DEFAULT_CLIENTID;
		EPROM_General.Cust_Detail.Reader_Id = DEFAULT_READERID;
		EPROM_General.Cust_Detail.Lattitude = DEFAULT_LAT;
		EPROM_General.Cust_Detail.Longitude = DEFAULT_LOGITUDE;

		EPROM_General.AI_DI_DO_Detail.Total_Di = DEFAULT_DI;
		EPROM_General.AI_DI_DO_Detail.Total_Do = DEFAULT_DO;
		EPROM_General.AI_DI_DO_Detail.Total_Ai = DEFAULT_AI;
		EPROM_General.AI_DI_DO_Detail.Sample_time_to_collect_AI = 1;

		i=0;
		for(i=0;i<3;i++)
		{
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].Id = i+1;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo = DEFAULT_SCALE_LO;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi = DEFAULT_SCALE_HI;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ = DEFAULT_CALZ;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS = DEFAULT_CALS;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type = DEFAULT_AI_CH_TYPE;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA = DEFAULT_MAILOWCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA = DEFAULT_MAILOWMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA = DEFAULT_MAIMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA = DEFAULT_MAIHIGHCAL_MA;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA_Point = DEFAULT_MAILOWCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA_Point = DEFAULT_MAILOWMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA_Point = DEFAULT_MAIMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA_Point = DEFAULT_MAIHIGHCAL_MA_POINT;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V = DEFAULT_MAILOWCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V = DEFAULT_MAILOWMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V = DEFAULT_MAIMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V = DEFAULT_MAIHIGHCAL_V;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V_Point = DEFAULT_MAILOWCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V_Point = DEFAULT_MAILOWMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V_Point = DEFAULT_MAIMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V_Point = DEFAULT_MAIHIGHCAL_V_POINT;

		}
		for(i=3;i<5;i++)
		{
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].Id = i+1;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo = DEFAULT_SCALE_LO;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi = 30;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ = DEFAULT_CALZ;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS = 30;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type = 2;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA = DEFAULT_MAILOWCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA = DEFAULT_MAILOWMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA = DEFAULT_MAIMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA = DEFAULT_MAIHIGHCAL_MA;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA_Point = DEFAULT_MAILOWCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA_Point = DEFAULT_MAILOWMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA_Point = DEFAULT_MAIMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA_Point = DEFAULT_MAIHIGHCAL_MA_POINT;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V = DEFAULT_MAILOWCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V = DEFAULT_MAILOWMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V = DEFAULT_MAIMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V = DEFAULT_MAIHIGHCAL_V;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V_Point = DEFAULT_MAILOWCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V_Point = DEFAULT_MAILOWMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V_Point = DEFAULT_MAIMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V_Point = DEFAULT_MAIHIGHCAL_V_POINT;

		}

		EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di = DEFAULT_RS232_1_ENABLE;
		EPROM_General.S_Comm.Rs232_1_Info.S_Protocol = DEFAULT_RS232_1_PROTCOL;
		EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu = DEFAULT_RS232_1_MASTER_SLAVE;
		EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate = DEFAULT_RS232_1_BAUDRATE;
		EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id = DEFAULT_RS232_1_PORT_ID;
		EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq = DEFAULT_RS232_1_POLL_FRQ;

		EPROM_General.Mo_Comm.Mo_Co_En_Di = DEFAULT_MODEM_ENABLE;
		EPROM_General.Mo_Comm.Mo_Com_Int = DEFAULT_MODEM_SERIAL_USB;
		EPROM_General.Mo_Comm.Mo_Proto = DEFAULT_MODEM_PROTOCOL;
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP,DEFAULT_MODEM_MQTT_BROKER_IP);
		EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port = DEFAULT_MODEM_MQTT_BROKER_PORT;
		EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode = DEFAULT_MQTT_COMM_MODE;
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name,DEFAULT_MODEM_MQTT_USR_NAME);
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass,DEFAULT_MODEM_MQTT_USR_PASS);
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id,DEFAULT_MODEM_MQTT_CLIENTID);
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic,DEFAULT_MODEM_MQTT_PUB_TOPIC);
		strcpy((char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic,DEFAULT_MODEM_MQTT_SUB_TOPIC);
		strcpy((char *)EPROM_General.Mo_Comm.Mo_APN,DEFAULT_MODEM_APN);
		EPROM_General.Mo_Comm.MQTT_LiveFreq = DEFAULT_MODEM_MQTT_LIVE_FRQ;

		EPROM_General.gpsDetails.GPS_Co_En_Di = DEFAULT_GPS_ENABLE;
		EPROM_General.gpsDetails.GPS_Poll_Freq = DEFAULT_GPS_POLL_FRQ;

		EPROM_General.sdCardDetails.SD_Card_Co_En_Di = DEFAULT_SD_ENABLE;
		EPROM_General.sdCardDetails.SD_Card_Size = DEFAULT_SD_SIZE;

		EPROM_General.DoModeDetails.Do_Mode = DEFAULT_DO_MODE;
		i=0;
		for(i=0;i<35;i++)
		{
			EPROM_General.DoModeDetails.DO_Value[i] = DEFAULT_DO_OFF;
		}
		memset(EPROM_General.DeviceID,0,sizeof(EPROM_General.DeviceID));

		EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Di = 2;
		EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do = DEFAULT_DO;

		for(i=0;i<EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Di ;i++)
		{
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_count = 0;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Freq = 0;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Const = 0.1;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Configured = 3.5;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Calculated = 0;
		}

		for(i=0;i<EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do;i++)
		{
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_type = 1;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_polarity = 0;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width = 100;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Count = 0;
			EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width_scale = 1;
		}

		EPROM_General.Lora_Frequency = 865062500;
		EPROM_General.Lora_Spreading_Factor = 8;
		EPROM_General.Lora_Bandwidth = 0;
		EPROM_General.Lora_Code_Rate = 1;

		EPROM_General.Lora_Preamble_Length = 8;
		EPROM_General.Lora_TX_Power = 22;
		EPROM_General.Modem_EC200_presence = 0;
		EPROM_General.LOta_MsgInterval = OTA_TIMEOUT;
		EPROM_General.pro_CheckByte = 0;
		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_EPROM_General();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}
	}
	else
	{
		if(Pro_Application_flag == 0)
		{
			EPROM_General.rebootCount++;
			if(strcmp(EPROM_General.Rtu_Detail.Hex_Version, DEFAULT_FV_VERSION))
			{
				strcpy(EPROM_General.Rtu_Detail.Hex_Version, DEFAULT_FV_VERSION);
				flag_flashUpdateEPROM_General = 1;
			}
			if(EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode < 0 || EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode > 1)
			{
				EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode = 0;
				flag_flashUpdateEPROM_General = 1;
			}

			if(EPROM_General.Cust_Detail.Tz_sign < 0 || EPROM_General.Cust_Detail.Tz_sign > 1)
			{
				EPROM_General.Cust_Detail.Tz_sign = 0;
				flag_flashUpdateEPROM_General = 1;
			}

			if(EPROM_General.Cust_Detail.reboot_day_night < 0 || EPROM_General.Cust_Detail.reboot_day_night > 3)
			{
				EPROM_General.Cust_Detail.reboot_day_night = 0;
				flag_flashUpdateEPROM_General = 1;
			}

			if(EPROM_General.Cust_Detail.Tz_hours < 0 || EPROM_General.Cust_Detail.Tz_hours > 23)
			{
				EPROM_General.Cust_Detail.Tz_hours = 5;
				flag_flashUpdateEPROM_General = 1;
			}

			if(EPROM_General.Cust_Detail.Tz_min < 0 || EPROM_General.Cust_Detail.Tz_min > 60)
			{
				EPROM_General.Cust_Detail.Tz_min = 30;
				flag_flashUpdateEPROM_General = 1;
			}
			if(!EPROM_General.LOta_MsgInterval || EPROM_General.LOta_MsgInterval == 0xFF)
			{
				EPROM_General.LOta_MsgInterval = OTA_TIMEOUT;
				flag_flashUpdateEPROM_General = 1;
			}
			if(flag_flashUpdateEPROM_General)
				flag_flashUpdateEPROM_General_WaitCounter = 10;
		}
	}
	snprintf((char *)print, sizeof(print),
		"Modem_EC200_presence : %d\r\n"
	    "Lora_Frequency: %u\r\n"
	    "Lora_Spreading_Factor: %u\r\n"
	    "Lora_Bandwidth: %u\r\n"
	    "Lora_Code_Rate: %u\r\n"
	    "Lora_Preamble_Length: %u\r\n"
	    "Lora_TX_Power: %u\r\n"
		"Hex_Version: %s\r\n",
		EPROM_General.Modem_EC200_presence,
	    EPROM_General.Lora_Frequency,
	    EPROM_General.Lora_Spreading_Factor,
	    EPROM_General.Lora_Bandwidth,
	    EPROM_General.Lora_Code_Rate,
	    EPROM_General.Lora_Preamble_Length,
	    EPROM_General.Lora_TX_Power,
		EPROM_General.Rtu_Detail.Hex_Version
	);
	WriteLog(1, print, 1);

	tempLora_Frequency = EPROM_General.Lora_Frequency;
	tempLora_Spreading_Factor = EPROM_General.Lora_Spreading_Factor;
	tempLora_Bandwidth = EPROM_General.Lora_Bandwidth;
	tempLora_Code_Rate = EPROM_General.Lora_Code_Rate;
	tempLora_Preamble_Length = EPROM_General.Lora_Preamble_Length;
	tempLora_TX_Power = EPROM_General.Lora_TX_Power;
}

void ExtFlash_update_EPROM_General()
{
	struct Save_Para_General tempEPROM_General;

	if(!EPROM_General.LOta_MsgInterval || EPROM_General.LOta_MsgInterval == 0xFF)
		EPROM_General.LOta_MsgInterval = OTA_TIMEOUT;
	memcpy(&tempEPROM_General,&EPROM_General,sizeof(EPROM_General));

	sprintf((char *)print,
			"ExtFlash_update_EPROM_General"
			"Modem_EC200_presence : %d\r\n"
		    "Lora_Frequency: %u\r\n"
		    "Lora_Spreading_Factor: %u\r\n"
		    "Lora_Bandwidth: %u\r\n"
		    "Lora_Code_Rate: %u\r\n"
		    "Lora_Preamble_Length: %u\r\n"
		    "Lora_TX_Power: %u\r\n"
			"LOta_MsgInterval: %x\r\n",
			EPROM_General.Modem_EC200_presence,
		    EPROM_General.Lora_Frequency,
		    EPROM_General.Lora_Spreading_Factor,
		    EPROM_General.Lora_Bandwidth,
		    EPROM_General.Lora_Code_Rate,
		    EPROM_General.Lora_Preamble_Length,
		    EPROM_General.Lora_TX_Power,
			EPROM_General.LOta_MsgInterval
		);
	WriteLog(1, print, 1);

	tempEPROM_General.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempEPROM_General,sizeof(tempEPROM_General));
	tempEPROM_General.SizeOfStuct = sizeof(tempEPROM_General);

	MX25L_Erase_Write_One_Sector((unsigned char *)&tempEPROM_General,sizeof(tempEPROM_General), EPROM_GENERAL_START_ADDRESS);
	HAL_Delay(100);
	flag_flashUpdateEPROM_General = 0;
}

void ExtFlash_Read_EPROM_AI_Calibration(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0,i=0;
	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_AI_Calibration, sizeof(EPROM_AI_Calibration), AI_CALIBRATION_DATA_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}


	if((EPROM_AI_Calibration.checkbyte != 0xAB)||(EPROM_AI_Calibration.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;

		sprintf((char *)print,"ExtFlash_Read_EPROM_AI_Calibration:EPROM_AI_Calibration.checkbyte:%d, %d\r\n",EPROM_AI_Calibration.checkbyte,EPROM_AI_Calibration.SizeOfStuct);
		WriteLog(1, print, 1);
//		Print_Flash();
	}
	else if(EPROM_AI_Calibration.SizeOfStuct<sizeof(EPROM_AI_Calibration))
	{
		if(EPROM_AI_Calibration.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_AI_Calibration,EPROM_AI_Calibration.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;

			sprintf((char *)print,"EPROM_AI_Calibration Stuct Size Change CRC mismatch : %d,%d,%d\r\n",EPROM_AI_Calibration.checkbyte,EPROM_AI_Calibration.SizeOfStuct,sizeof(EPROM_AI_Calibration));
			WriteLog(1, print, 1);
//			Print_Flash();
		}
	}
	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
		// if fail than default flash case else below steps
		// varify all parameter in external flash
		// increament bootcount
		// calulate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_AI_Calibration.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_AI_Calibration,sizeof(EPROM_AI_Calibration)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
			sprintf((char *)print,"EPROM_AI_Calibration same size but CRC mismatch : %d,%d,%d\r\n",EPROM_AI_Calibration.checkbyte,EPROM_AI_Calibration.SizeOfStuct,sizeof(EPROM_AI_Calibration));
			WriteLog(1, print, 1);
//			Print_Flash();
		}
	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_AI_Calibration.checkbyte = 0xAB;
		EPROM_AI_Calibration.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_AI_Calibration,sizeof(EPROM_AI_Calibration));
		EPROM_AI_Calibration.SizeOfStuct = sizeof(EPROM_AI_Calibration);

		for(i=0;i<DEFAULT_AI;i++)
		{
			EPROM_AI_Calibration.AI_Detail[i].Id = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].Id;
			EPROM_AI_Calibration.AI_Detail[i].scaleLo = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo;
			EPROM_AI_Calibration.AI_Detail[i].scaleHi = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi;
			EPROM_AI_Calibration.AI_Detail[i].calZ = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ;
			EPROM_AI_Calibration.AI_Detail[i].calS = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS;

			EPROM_AI_Calibration.AI_Detail[i].AI_ch_Type = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type;//DEFAULT_AI_CH_TYPE;

			EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_mA = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA;//DEFAULT_MAILOWCAL_MA;
			EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_mA = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA;//DEFAULT_MAILOWMIDCAL_MA;
			EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_mA = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA;//DEFAULT_MAIMIDCAL_MA;
			EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_mA = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA;//DEFAULT_MAIHIGHCAL_MA;

			EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_mA_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA_Point;//DEFAULT_MAILOWCAL_MA_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_mA_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA_Point;//DEFAULT_MAILOWMIDCAL_MA_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_mA_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA_Point;//DEFAULT_MAIMIDCAL_MA_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_mA_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA_Point;//DEFAULT_MAIHIGHCAL_MA_POINT;

			EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_V = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V;//DEFAULT_MAILOWCAL_V;
			EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_V = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V;//DEFAULT_MAILOWMIDCAL_V;
			EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_V = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V;//DEFAULT_MAIMIDCAL_V;
			EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_V = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V;//DEFAULT_MAIHIGHCAL_V;

			EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_V_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V_Point;//DEFAULT_MAILOWCAL_V_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_V_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V_Point;//DEFAULT_MAILOWMIDCAL_V_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_V_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V_Point;//DEFAULT_MAIMIDCAL_V_POINT;
			EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_V_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V_Point;//DEFAULT_MAIHIGHCAL_V_POINT;

		}

		memset(EPROM_AI_Calibration.DeviceID,0,sizeof(EPROM_AI_Calibration.DeviceID));
		memcpy(EPROM_AI_Calibration.DeviceID,EPROM_General.DeviceID,sizeof(EPROM_General.DeviceID));


		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_EPROM_AI_Calibration();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}
	}
	else
	{
		for(i=0;i<DEFAULT_AI;i++)
		{
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type = EPROM_AI_Calibration.AI_Detail[i].AI_ch_Type;//DEFAULT_AI_CH_TYPE;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA = EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_mA;//DEFAULT_MAILOWCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA = EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_mA;//DEFAULT_MAILOWMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA = EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_mA;//DEFAULT_MAIMIDCAL_MA;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA = EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_mA;//DEFAULT_MAIHIGHCAL_MA;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_mA_Point = EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_mA_Point;//DEFAULT_MAILOWCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_mA_Point = EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_mA_Point;//DEFAULT_MAILOWMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_mA_Point = EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_mA_Point;//DEFAULT_MAIMIDCAL_MA_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_mA_Point = EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_mA_Point;//DEFAULT_MAIHIGHCAL_MA_POINT;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V = EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_V;//DEFAULT_MAILOWCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V = EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_V;//DEFAULT_MAILOWMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V = EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_V;//DEFAULT_MAIMIDCAL_V;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V = EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_V;//DEFAULT_MAIHIGHCAL_V;

			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowCal_V_Point = EPROM_AI_Calibration.AI_Detail[i].mAiLowCal_V_Point;//DEFAULT_MAILOWCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiLowMidCal_V_Point = EPROM_AI_Calibration.AI_Detail[i].mAiLowMidCal_V_Point;//DEFAULT_MAILOWMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiMidCal_V_Point = EPROM_AI_Calibration.AI_Detail[i].mAiMidCal_V_Point;//DEFAULT_MAIMIDCAL_V_POINT;
			EPROM_General.AI_DI_DO_Detail.AI_Detail[i].mAiHighCal_V_Point = EPROM_AI_Calibration.AI_Detail[i].mAiHighCal_V_Point;//DEFAULT_MAIHIGHCAL_V_POINT;
		}

		memset(EPROM_General.DeviceID,0,sizeof(EPROM_General.DeviceID));
		memcpy(EPROM_General.DeviceID,EPROM_AI_Calibration.DeviceID,sizeof(EPROM_AI_Calibration.DeviceID));
	}

}

void ExtFlash_update_EPROM_AI_Calibration()
{
	struct Save_Para_AI_Calibration tempEPROM_AI_Calibration;
	memcpy(&tempEPROM_AI_Calibration,&EPROM_AI_Calibration,sizeof(tempEPROM_AI_Calibration));

	tempEPROM_AI_Calibration.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempEPROM_AI_Calibration,sizeof(tempEPROM_AI_Calibration));
	tempEPROM_AI_Calibration.SizeOfStuct = sizeof(tempEPROM_AI_Calibration);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempEPROM_AI_Calibration,sizeof(tempEPROM_AI_Calibration), AI_CALIBRATION_DATA_START_ADDRESS);
}

void ExtFlash_Read_EPROM_Schedule(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0,i;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_Schedule, sizeof(EPROM_Schedule), EPROM_SCHEDULE_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((EPROM_Schedule.checkbyte != 0xAB)||(EPROM_Schedule.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_EPROM_Schedule:checkbyte default\r\n");
	    WriteLog(1, tDebug, 1);
	}
	else if(EPROM_Schedule.SizeOfStuct<sizeof(EPROM_Schedule))
	{
		if(EPROM_Schedule.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Schedule,EPROM_Schedule.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Schedule: Stuct Size Change CRC mismatch: %d, %d, %d\r\n",EPROM_Schedule.checkbyte,EPROM_Schedule.SizeOfStuct,sizeof(EPROM_Schedule));
		    WriteLog(1, tDebug, 1);
//			Print_Flash();
		}
	}
	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
		// if fail than default flash case else below steps
		// varify all parameter in external flash
		// increament bootcount
		// calulate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_Schedule.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Schedule,sizeof(EPROM_Schedule)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Schedule:size same but CRC mitchmatch: %d, %d, %d\r\n",EPROM_Schedule.checkbyte,EPROM_Schedule.SizeOfStuct,sizeof(EPROM_Schedule));
		    WriteLog(1, tDebug, 1);
//			Print_Flash();
		}
	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_Schedule.checkbyte = 0xAB;
		EPROM_Schedule.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_Schedule,sizeof(EPROM_Schedule));
		EPROM_Schedule.SizeOfStuct = sizeof(EPROM_Schedule);

		EPROM_Schedule.Total_No_Schedule = DEFAULT_SCHEDULE_TOTAL_NO;

		for(i=0;i<84;i++)
		{
			EPROM_Schedule.Schedule[i].Sch_Id = i+1;
			EPROM_Schedule.Schedule[i].Sch_En_Di = DEFAULT_SCHEDULE_ENABLE;
			EPROM_Schedule.Schedule[i].Start_HH = DEFAULT_SCHEDULE_START_HH;
			EPROM_Schedule.Schedule[i].Start_Min = DEFAULT_SCHEDULE_START_MIN;
			EPROM_Schedule.Schedule[i].Stop_HH = DEFAULT_SCHEDULE_STOP_HH;
			EPROM_Schedule.Schedule[i].Stop_Min = DEFAULT_SCHEDULE_STOP_MIN;
		}

		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_EPROM_Schedule();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}

	}
	else
	{

	}


}

void ExtFlash_update_EPROM_Schedule()
{
	EPROM_Schedule.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_Schedule,sizeof(EPROM_Schedule));
	EPROM_Schedule.SizeOfStuct = sizeof(EPROM_Schedule);
	MX25L_Erase_Write_One_Sector((unsigned char *)&EPROM_Schedule,sizeof(EPROM_Schedule), EPROM_SCHEDULE_START_ADDRESS);
}

void ExtFlash_Read_EPROM_Dync(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_Dync, sizeof(EPROM_Dync), DYNC_PARA_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((EPROM_Dync.checkbyte != 0xAB)||(EPROM_Dync.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// Calculate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_EPROM_Dync:checkbyte default\r\n");
	    WriteLog(1, tDebug, 1);
	}
	else if(EPROM_Dync.SizeOfStuct<sizeof(EPROM_Dync))
	{
		if(EPROM_Dync.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Dync,EPROM_Dync.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Dync: Stuct Size Change CRC mismatch: %d, %d, %d\r\n",EPROM_Dync.checkbyte,EPROM_Dync.SizeOfStuct,sizeof(EPROM_Dync));
		    WriteLog(1, tDebug, 1);
		}
	}
	else
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// Verify checksum of external flash using sizeof
		// if fail than default flash case else below steps
		// Verify all parameter in external flash using sizeof
		// Calculate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_Dync.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Dync,sizeof(EPROM_Dync)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Dync:size same but CRC mitchmatch: %d, %d, %d\r\n",EPROM_Dync.checkbyte,EPROM_Dync.SizeOfStuct,sizeof(EPROM_Dync));
		    WriteLog(1, tDebug, 1);
		}
	}
	EPROM_Dync.rebootCount++;
	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_Dync.checkbyte = 0xAB;
		EPROM_Dync.rebootCount = EPROM_General.rebootCount;
	}
	else
		EPROM_General.rebootCount = EPROM_Dync.rebootCount;	// Keep a backup copy
	gFinalAnaValF[REBOOT_COUNT_gFinalAnaValF] = EPROM_Dync.rebootCount;
	sprintf(print, "Reboot Count %d\r\n", EPROM_Dync.rebootCount);
	WriteLog(1, print, 1);
	ExtFlash_update_EPROM_Dync();
}

void ExtFlash_update_EPROM_Dync()
{
	EPROM_Dync.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_Dync,sizeof(EPROM_Dync));
	EPROM_Dync.SizeOfStuct = sizeof(EPROM_Dync);
	MX25L_Erase_Write_One_Sector((unsigned char *)&EPROM_Dync,sizeof(EPROM_Dync), DYNC_PARA_START_ADDRESS);
}

void ExtFlash_Read_EPROM_Modbus_Quary_Detail(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0,i;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_Modbus_Quary_Detail, sizeof(EPROM_Modbus_Quary_Detail), EPROM_MODBUS_QUERY_DETAILS_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((EPROM_Modbus_Quary_Detail.checkbyte != 0xAB)||(EPROM_Modbus_Quary_Detail.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_EPROM_Modbus_Quary_Detail checkbytes default\r\n");
	    WriteLog(1, tDebug, 1);
	}
	else if(EPROM_Modbus_Quary_Detail.SizeOfStuct<sizeof(EPROM_Modbus_Quary_Detail))
	{
		if(EPROM_Modbus_Quary_Detail.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Modbus_Quary_Detail,EPROM_Modbus_Quary_Detail.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Modbus_Quary_Detail_1 Stuct size CRC mismatched: %d, %d, %d\r\n",EPROM_Modbus_Quary_Detail.checkbyte,EPROM_Modbus_Quary_Detail.SizeOfStuct,sizeof(EPROM_Modbus_Quary_Detail));
		    WriteLog(1, tDebug, 1);
//		    Print_Flash();
		}
	}
	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
		// if fail than default flash case else below steps
		// varify all parameter in external flash
		// increament bootcount
		// calulate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_Modbus_Quary_Detail.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Modbus_Quary_Detail,sizeof(EPROM_Modbus_Quary_Detail)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_Modbus_Quary_Detail_2: same size CRC mismatched: %d, %d, %d\r\n",EPROM_Modbus_Quary_Detail.checkbyte,EPROM_Modbus_Quary_Detail.SizeOfStuct,sizeof(EPROM_Modbus_Quary_Detail));
		    WriteLog(1, tDebug, 1);
//		    Print_Flash();
		}
	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_Modbus_Quary_Detail.checkbyte = 0xAB;
		EPROM_Modbus_Quary_Detail.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_Modbus_Quary_Detail,sizeof(EPROM_Modbus_Quary_Detail));
		EPROM_Modbus_Quary_Detail.SizeOfStuct = sizeof(EPROM_Modbus_Quary_Detail);

		EPROM_Modbus_Quary_Detail.TotalQuery = DEFAULT_MODBUS_QUARY_DETAIL_TOTAL_NO;
		EPROM_Modbus_Quary_Detail.RetryCount = DEFAULT_MODBUS_QUARY_DETAIL_RETRY_COUNT;
		EPROM_Modbus_Quary_Detail.TotalPara = DEFAULT_MODBUS_QUARY_DETAIL_TOTAL_PARA_NO;
		EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled = DEFAULT_MAX_DATA_TAG;

		for(i=0;i<100;i++)
		{
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].Mod_Quary_ID = i+1;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mFunctionCode = 0;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mDataType = 0;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection = 0;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId = 0;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr = 0;
			EPROM_Modbus_Quary_Detail.Mod_Quary[i].mNoOfRegister = 0;
		}
		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_EPROM_Modbus_Quary_Detail();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}
	}
	else
	{

	}
}


void ExtFlash_update_EPROM_Modbus_Quary_Detail()
{
   //sprintf(tDebug, "ExtFlash_update_EPROM_Modbus_Quary_Detail\r\n");
   // WriteLog(1, tDebug, 1);

	struct Save_Para_Modbus_Configuration tempEPROM_Modbus_Quary_Detail;
	memcpy(&tempEPROM_Modbus_Quary_Detail,&EPROM_Modbus_Quary_Detail,sizeof(tempEPROM_Modbus_Quary_Detail));

	tempEPROM_Modbus_Quary_Detail.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempEPROM_Modbus_Quary_Detail,sizeof(tempEPROM_Modbus_Quary_Detail));
	tempEPROM_Modbus_Quary_Detail.SizeOfStuct = sizeof(EPROM_Modbus_Quary_Detail);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempEPROM_Modbus_Quary_Detail,sizeof(tempEPROM_Modbus_Quary_Detail), EPROM_MODBUS_QUERY_DETAILS_START_ADDRESS);

}


void ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0;
	unsigned int i;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_PCBPLC_General_Reg, sizeof(EPROM_PCBPLC_General_Reg), EPROM_PCBPLC_GENERAL_REG_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((EPROM_PCBPLC_General_Reg.checkbyte != 0xAB)||(EPROM_PCBPLC_General_Reg.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG : Checkbyte default\r\n");
	    WriteLog(1, tDebug, 1);
	}
	else if(EPROM_PCBPLC_General_Reg.SizeOfStuct<sizeof(EPROM_PCBPLC_General_Reg))
	{
		if(EPROM_PCBPLC_General_Reg.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_PCBPLC_General_Reg,EPROM_PCBPLC_General_Reg.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG_1: Stuct size CRC mismatched: %d, %d, %d\r\n",EPROM_PCBPLC_General_Reg.checkbyte,EPROM_PCBPLC_General_Reg.SizeOfStuct,sizeof(EPROM_PCBPLC_General_Reg));
		    WriteLog(1, tDebug, 1);
//		    Print_Flash();
		}
	}
	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
		// if fail than default flash case else below steps
		// varify all parameter in external flash
		// increament bootcount
		// calulate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_PCBPLC_General_Reg.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_PCBPLC_General_Reg,sizeof(EPROM_PCBPLC_General_Reg)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
		    sprintf(tDebug, "ExtFlash_Read_EPROM_PCBPLC_GENERAL_REG_2: same size CRC mismatched: %d, %d, %d\r\n",EPROM_PCBPLC_General_Reg.checkbyte,EPROM_PCBPLC_General_Reg.SizeOfStuct,sizeof(EPROM_PCBPLC_General_Reg));
		    WriteLog(1, tDebug, 1);
//		    Print_Flash();
		}
	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_PCBPLC_General_Reg.checkbyte = 0xAB;
		for(i=0;i<1000;i++)
		{
			EPROM_PCBPLC_General_Reg.General_Reg[i]=0.0;
		}
		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}
	}

	for(i=0;i<MAX_GENERAL_AI;i++)
	{
		gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+i]=EPROM_PCBPLC_General_Reg.General_Reg[i];
		signal_arr[i] = gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF + i];
	}
}

void ExtFlash_update_EPROM_PCBPLC_GENERAL_REG()
{
	struct Save_Para_PCBPLC_General_Reg tempEPROM_PCBPLC_General_Reg;
	memcpy(&tempEPROM_PCBPLC_General_Reg,&EPROM_PCBPLC_General_Reg,sizeof(tempEPROM_PCBPLC_General_Reg));

	tempEPROM_PCBPLC_General_Reg.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempEPROM_PCBPLC_General_Reg,sizeof(tempEPROM_PCBPLC_General_Reg));
	tempEPROM_PCBPLC_General_Reg.SizeOfStuct = sizeof(tempEPROM_PCBPLC_General_Reg);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempEPROM_PCBPLC_General_Reg,sizeof(tempEPROM_PCBPLC_General_Reg), EPROM_PCBPLC_GENERAL_REG_START_ADDRESS);
}


void ExtFlash_Read_EPROM_Frequent(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &EPROM_Frequent, sizeof(EPROM_Frequent), EPROM_FREQUENT_DATA_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}


	if((EPROM_Frequent.checkbyte != 0xAB)||(EPROM_Frequent.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;

		sprintf((char *)print,"ExtFlash_Read_EPROM_Frequent:EPROM_Frequent.checkbyte:%d, %d\r\n",EPROM_Frequent.checkbyte,EPROM_Frequent.SizeOfStuct);
		WriteLog(1, print, 1);
//		Print_Flash();
	}
	else if(EPROM_Frequent.SizeOfStuct<sizeof(EPROM_Frequent))
	{
		if(EPROM_Frequent.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Frequent,EPROM_Frequent.SizeOfStuct))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;

			sprintf((char *)print,"Stuct Size Change CRC mismatch : %d,%d,%d\r\n",EPROM_Frequent.checkbyte,EPROM_Frequent.SizeOfStuct,sizeof(EPROM_Frequent));
			WriteLog(1, print, 1);
			//Print_Flash();
		}
	}
	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
	{
		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
		// Note : ==============================================
		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
		// if fail than default flash case else below steps
		// varify all parameter in external flash
		// increament bootcount
		// calulate checksum for stored parameter , size of struct
		// write checksum and size of struct in internal flash
		// write external flash with stored parameter
		// ======================================================
		if(EPROM_Frequent.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&EPROM_Frequent,sizeof(EPROM_Frequent)))
		{
			Set_default_Flash=0;
		}
		else
		{
			Set_default_Flash=1;
			sprintf((char *)print,"same size but CRC mismatch : %d,%d,%d\r\n",EPROM_Frequent.checkbyte,EPROM_Frequent.SizeOfStuct,sizeof(EPROM_Frequent));
			WriteLog(1, print, 1);
//			Print_Flash();
		}
	}


	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		EPROM_Frequent.checkbyte = 0xAB;
		EPROM_Frequent.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&EPROM_Frequent,sizeof(EPROM_Frequent));
		EPROM_Frequent.SizeOfStuct = sizeof(EPROM_Frequent);
		EPROM_Frequent.DI1_Pulse = 0;
		EPROM_Frequent.DI2_Pulse = 0;
		EPROM_Frequent.Pulse_DI_Interrupt_Type = 0;  // 0 for falling 1 for rising
		EPROM_Frequent.Pulse_DI_frequency_Method = 1;// 0 for counter based 1 for wavelength based
		EPROM_Frequent.Pulse_DI_frequency_time = 50000;

		ExtFlash_update_EPROM_Frequent();
	}
	else
	{
		EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_count = EPROM_Frequent.DI1_Pulse;
		EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_count = EPROM_Frequent.DI2_Pulse;
		DI1_Pulse_Count = EPROM_Frequent.DI1_Pulse;
		DI2_Pulse_Count = EPROM_Frequent.DI2_Pulse;
	}
	actualPulse_DI_frequency_time = EPROM_Frequent.Pulse_DI_frequency_time*EPROM_Frequent.Pulse_DI_frequency_Method;
}

void ExtFlash_update_EPROM_Frequent()
{
	struct Save_Para_Frequent tempEPROM_Frequent;
	memcpy(&tempEPROM_Frequent,&EPROM_Frequent,sizeof(EPROM_Frequent));

	tempEPROM_Frequent.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempEPROM_Frequent,sizeof(tempEPROM_Frequent));
	tempEPROM_Frequent.SizeOfStuct = sizeof(tempEPROM_Frequent);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempEPROM_Frequent,sizeof(tempEPROM_Frequent), EPROM_FREQUENT_DATA_START_ADDRESS);
}

void syncExtFlashVariableWithPCBPLCVariable()
{
	unsigned int i=0;
	lwgsm_ip_t ip_t;
	unsigned char *buf_t;

	gFinalAnaValF[LOG_RATE_gFinalAnaValF] = EPROM_General.LogRate;		// Min
	gFinalAnaValF[RTU_ID_gFinalAnaValF] = EPROM_General.Rtu_Detail.RTUId;

	gFinalAnaValF[CLIENT_ID_gFinalAnaValF] = EPROM_General.Cust_Detail.Client_Id;
	gFinalAnaValF[READER_ID_gFinalAnaValF] = EPROM_General.Cust_Detail.Reader_Id;

	gFinalAnaValF[LORA_RETRY_COUNT_gFinalAnaValF] = EPROM_General.LoraRetryCount;
	gFinalAnaValF[LORA_RETRY_DELAY_gFinalAnaValF] = EPROM_General.LoraRetryDelay;
	gFinalAnaValF[READ_CMD_WAIT_TIME_gFinalAnaValF] = EPROM_General.readCmdDelay;
	gFinalAnaValF[LOG_SEND_DELAY_gFinalAnaValF] = EPROM_General.LogSendDelay;
	gFinalAnaValF[LOTA_MSG_INTERVAL_gFinalAnaValF] = EPROM_General.LOta_MsgInterval;

	gFinalAnaValF[GPS_LAT_gFinalAnaValF] = EPROM_General.Cust_Detail.Lattitude;
	gFinalAnaValF[GPS_Log_gFinalAnaValF] = EPROM_General.Cust_Detail.Longitude;

	gFinalAnaValF[MAX_DO_gFinalAnaValF] = EPROM_General.AI_DI_DO_Detail.Total_Do;
	gFinalAnaValF[MAX_DI_gFinalAnaValF] = EPROM_General.AI_DI_DO_Detail.Total_Di;
	gFinalAnaValF[MAX_AI_gFinalAnaValF] = EPROM_General.AI_DI_DO_Detail.Total_Ai;
	gFinalAnaValF[SAMPLE_TIME_TO_Collect_AI_gFinalAnaValF] = EPROM_General.AI_DI_DO_Detail.Sample_time_to_collect_AI;
	i=0;
	for(i=0;i<DEFAULT_AI;i++)
	{
		gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5) + 0 ] = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type;
		gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5) + 1 ] = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calZ;
		gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5) + 2 ] = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].calS;
		gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5) + 3 ] = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleLo;
		gFinalAnaValF[SCALING_PARA_AI_gFinalAnaValF + (i*5) + 4 ] = EPROM_General.AI_DI_DO_Detail.AI_Detail[i].scaleHi;
	}

	gFinalAnaValF[RS232_1_MODBUS_RTU_ASCII_gFinalAnaValF] = EPROM_General.S_Comm.Rs232_1_Info.S_Protocol;
	gFinalAnaValF[RS232_1_BAUDRATE_gFinalAnaValF] = EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate;
	gFinalAnaValF[RS232_1_MODBUS_POLL_FRQ_gFinalAnaValF] = EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq;

	buf_t = (unsigned char*)&EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP;
    lwgsmi_parse_ip((const char**)&buf_t, &ip_t);

    gFinalAnaValF[MODEM_MQTT_BROKER_IP_0_gFinalAnaValF] = ip_t.ip[0];
    gFinalAnaValF[MODEM_MQTT_BROKER_IP_1_gFinalAnaValF] = ip_t.ip[1];
    gFinalAnaValF[MODEM_MQTT_BROKER_IP_2_gFinalAnaValF] = ip_t.ip[2];
    gFinalAnaValF[MODEM_MQTT_BROKER_IP_3_gFinalAnaValF] = ip_t.ip[3];
    gFinalAnaValF[MODEM_MQTT_BROKER_PORT_gFinalAnaValF] = EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port;
    
	gFinalAnaValF[COMM_MODE_ETHER_GPRS_gFinalAnaValF] = EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode;
    gFinalAnaValF[TIMEZONE_SIGN_gFinalAnaValF] = EPROM_General.Cust_Detail.Tz_sign;
    gFinalAnaValF[TIMEZONE_HOUR_gFinalAnaValF] = EPROM_General.Cust_Detail.Tz_hours;
    gFinalAnaValF[TIMEZONE_MIN_gFinalAnaValF] = EPROM_General.Cust_Detail.Tz_min;
    gFinalAnaValF[DEVICE_REBOOT_gFinalAnaValF] = 0.0;
    gFinalAnaValF[DEVICE_REBOOT_TIME_DAY_NIGHT_gFinalAnaValF] = EPROM_General.Cust_Detail.reboot_day_night;

    /*
    if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"airtelgprs.com") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 1;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"internet") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 2;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"www") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 3;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"bsnlstatic") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 4;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"bsnlnet") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 5;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"bsnllive") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 6;
    }
    else if(FindSubstr((char *)EPROM_General.Mo_Comm.Mo_APN, (char *)"jionet") != -1)
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 7;
    }
    else
    {
    	gFinalAnaValF[MODEM_APN_NAME_gFinalAnaValF] = 255;
    }

    memcpy(PDP_Context_APN,EPROM_General.Mo_Comm.Mo_APN,strlen(EPROM_General.Mo_Comm.Mo_APN));
*/
    gFinalAnaValF[MODEM_MQTT_LIVE_FRQ_gFinalAnaValF] = EPROM_General.Mo_Comm.MQTT_LiveFreq;

	//EPROM_General.gpsDetails.GPS_Co_En_Di = DEFAULT_GPS_ENABLE;
	//EPROM_General.gpsDetails.GPS_Poll_Freq = DEFAULT_GPS_POLL_FRQ;

	//EPROM_General.sdCardDetails.SD_Card_Co_En_Di = DEFAULT_SD_ENABLE;
	//EPROM_General.sdCardDetails.SD_Card_Size = DEFAULT_SD_SIZE;

	gFinalAnaValF[MODE_AUTO_MANUAL_gFinalAnaValF] = EPROM_General.DoModeDetails.Do_Mode;


	for(i=0;i<MAX_SCHEDULE;i++)
	{
		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i*5) + 0 ] = EPROM_Schedule.Schedule[i].Sch_En_Di;
		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i*5) + 1 ] = EPROM_Schedule.Schedule[i].Start_HH;
		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i*5) + 2 ] = EPROM_Schedule.Schedule[i].Start_Min;
		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i*5) + 3 ] = EPROM_Schedule.Schedule[i].Stop_HH;
		gFinalAnaValF[SCHEDULE_gFinalAnaValF + (i*5) + 4 ] = EPROM_Schedule.Schedule[i].Stop_Min;
	}


	gFinalAnaValF[MODBUS_MAX_QUERY_gFinalAnaValF] = EPROM_Modbus_Quary_Detail.TotalQuery;
	gFinalAnaValF[MODBUS_MAX_NO_OF_SMSTAG_gFinalAnaValF] = EPROM_Modbus_Quary_Detail.mMaxDataTagEnabled;

	for(i=0;i<50;i++)
	{
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 0 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection;
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 1 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId;
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 2 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mRegStartAddr;
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 3 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mDataType;
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 4 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mNoOfRegister;
		gFinalAnaValF[MODBUS_READ_QUERY_gFinalAnaValF + (i*6) + 5 ] = EPROM_Modbus_Quary_Detail.Mod_Quary[i].mFunctionCode;
	}

	for(i=0;i<2;i++)
	{
		gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5) + 0 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_count;
		gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5) + 1 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Freq;
		gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5) + 2 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Const;
		gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Configured;
		gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + (i*5) + 4 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[i].Pulse_DI_Flow_Calculated;
	}

	for(i=0;i<20;i++)	// 26
	{
		gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5) + 0 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_type;
		gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5) + 1 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_polarity;
		gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5) + 2 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width;
		gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Count;
		gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (i*5) + 4 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[i].Pulse_DO_Width_scale;
	}

	for(i=0;i<15;i++)
	{
		gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (i*2) + 0 ] = Dual_DO_Pulse_Stage[i];
		gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + (i*2) + 1 ] = Dual_DO_actual_PulseWidth[i];
	}
    gFinalAnaValF[PULSE_DI_INTERRUPT_TYPE_gFinalAnaValF] = EPROM_Frequent.Pulse_DI_Interrupt_Type;
    gFinalAnaValF[PULSE_DI_FREQUENCY_METHOD_gFinalAnaValF] = EPROM_Frequent.Pulse_DI_frequency_Method;
    gFinalAnaValF[PULSE_DI_FREQUENCY_TIME_gFinalAnaValF] = EPROM_Frequent.Pulse_DI_frequency_time;

}



void ExtFlash_Read_gPlcRecFlash(unsigned char makeDefault)
{
	unsigned char Set_default_Flash=0;
	unsigned int i;


	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*) &gPlcRecFlash, sizeof(gPlcRecFlash), PCB_REC_INFO_FILE_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((gPlcRecFlash.checkbyte != 0xAB)||(gPlcRecFlash.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_gPlcRecFlash : Checkbyte default\r\n");
	    WriteLog(1, tDebug, 1);
//	    Print_Flash();
	}
//	else if(gPlcRecFlash.SizeOfStuct<sizeof(gPlcRecFlash))
//	{
//		if(gPlcRecFlash.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&gPlcRecFlash,gPlcRecFlash.SizeOfStuct))
//		{
//			Set_default_Flash=0;
//		}
//		else
//		{
//			Set_default_Flash=1;
//		}
//	}
//	else //if(SizeOf_QSPI_Store_para==sizeof(QSPI_Store_para))||(SizeOf_QSPI_Store_para>sizeof(QSPI_Store_para))
//	{
//		// "sizeof(QSPI_Store_para) always be greater than or Equal to Previous Version's sizeof(QSPI_Store_para)"
//		// Note : ==============================================
//		// varify checksum of externalflash using sizeof(QSPI_Store_para) (application's struct size)
//		// if fail than default flash case else below steps
//		// varify all parameter in external flash
//		// increament bootcount
//		// calulate checksum for stored parameter , size of struct
//		// write checksum and size of struct in internal flash
//		// write external flash with stored parameter
//		// ======================================================
//		if(gPlcRecFlash.ChecksumOfStuct==calculateCheckSumOfStruct((unsigned char*)&gPlcRecFlash,sizeof(gPlcRecFlash)))
//		{
//			Set_default_Flash=0;
//		}
//		else
//		{
//			Set_default_Flash=1;
//		}
//	}

	if((Set_default_Flash==1)||(makeDefault == 1))
	{
		gPlcRecFlash.checkbyte = 0xAB;
		gPlcRecFlash.extract_receipe = 0;
		gPlcRecFlash.mPlcFileLength = 0;
		gPlcRecFlash.mRecFileLength = 0;
		gPlcRecFlash.mPlcFileCRC = 0;
		gPlcRecFlash.mRecFileCRC = 0;
		for(i=0;i<MAX_PLCVAR;i++)
		{
			gPlcRecFlash.plcVarArr[i] = 0;
			gPlcRecFlash.mPlcVarTypeArr[i] = 0;
		}
		//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
		{
			ExtFlash_update_gPlcRecFlash();
			//xSemaphoreGive(sendExternalFlashSemaphore);
		}
	}
}


void ExtFlash_update_gPlcRecFlash()
{
	plcRecFlashInfo_t tempgPlcRecFlash;
	memcpy(&tempgPlcRecFlash,&gPlcRecFlash,sizeof(tempgPlcRecFlash));

	tempgPlcRecFlash.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempgPlcRecFlash,sizeof(tempgPlcRecFlash));
	tempgPlcRecFlash.SizeOfStuct = sizeof(tempgPlcRecFlash);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempgPlcRecFlash,sizeof(tempgPlcRecFlash), PCB_REC_INFO_FILE_START_ADDRESS);
}

void ExtFlash_Read_OTA_Data()
{
	unsigned char Set_default_Flash=0;
	//unsigned int i;

	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		MX25L_ReadRaw((u8_t*)&OTA_store_Data,sizeof(OTA_store_Data), OTA_UPDATE_DATA_START_ADDRESS);
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}

	if((OTA_store_Data.checkbyte != 0xAB)||(OTA_store_Data.SizeOfStuct==0xFFFF))
	{
		// Note : ==============================================
		// calulate checksum , size of struct for default parameters
		// write in write internal flash
		// write external flash with default para
		// ======================================================
		Set_default_Flash=1;
	    sprintf(tDebug, "ExtFlash_Read_OTA_Data : Checkbyte default\r\n");
	    WriteLog(1, tDebug, 1);
//	    Print_Flash();
	}
	if(Set_default_Flash == 1)
	{
		memset(&OTA_store_Data, 0, sizeof(OTA_store_Data));
		OTA_store_Data.checkbyte = 0xAB;
	}
	if(OTA_store_Data.OTApushTime == 0xFFFF)
	{
		OTA_store_Data.sessionId = 0;
		OTA_store_Data.OTApushTime = 0;
		OTA_store_Data.forVer = 0;
		OTA_store_Data.DeltaCRC = 0;
		OTA_store_Data.DeltaSize = 0;
		OTA_store_Data.FileType = 0;
		OTA_store_Data.TgtFile_Size = 0;
		OTA_store_Data.TgtHEX_Crc = 0;
	}
	//if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		ExtFlash_update_OTA_Data();
		//xSemaphoreGive(sendExternalFlashSemaphore);
	}
}

void ExtFlash_update_OTA_Data()
{
	struct OTA_hex_Data tempOTA_store_Data;
	//plcRecFlashInfo_t tempgPlcRecFlash;
	memcpy(&tempOTA_store_Data,&OTA_store_Data,sizeof(tempOTA_store_Data));

	tempOTA_store_Data.ChecksumOfStuct = calculateCheckSumOfStruct((unsigned char*)&tempOTA_store_Data,sizeof(tempOTA_store_Data));
	tempOTA_store_Data.SizeOfStuct = sizeof(tempOTA_store_Data);
	MX25L_Erase_Write_One_Sector((unsigned char *)&tempOTA_store_Data,sizeof(tempOTA_store_Data), OTA_UPDATE_DATA_START_ADDRESS);

}
JSON_ERROR_RESPONSE Response_Config_Read_JSON_frame(COM_TYPE com_mode , CMD_TYPE CMD, char cmdState, char file_CMD_type, RES_ACK iACK, char * ACK_Response)
{
	cJSON *response_json_Obejct = NULL;
	JSON_ERROR_RESPONSE ret = JSON_SUCCESS;

	response_json_Obejct = cJSON_CreateObject();
	if (response_json_Obejct == NULL)
	{
		response_json_Obejct = cJSON_CreateObject();
		if (response_json_Obejct == NULL)
			ret = FAILED_CREATE_JSON_OBJECT;
	}

	if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
	{
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CMD", CMD))
			ret = FAILED_ADD_JSON_OBJECT;
	}

	if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", cmdState))
	{
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDState", cmdState))
			ret = FAILED_ADD_JSON_OBJECT;
	}

	if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDType",file_CMD_type))
	{
		if(!cJSON_AddNumberToObject(response_json_Obejct, "CMDType",file_CMD_type))
			ret = FAILED_ADD_JSON_OBJECT;
	}

	switch(cmdState)
	{
		case RTU_INFO:
		{
			//To Do temp for check
			//strcpy(EPROM_General.Rtu_Detail.HW_Version,"iRTU6000.HW_V1.0.0.0");
			//strcpy(EPROM_General.Rtu_Detail.Hex_Version,"iRTU6000.HEX_V1.0.0.0");
			//strcpy(EPROM_General.Rtu_Detail.PLC_Version,"iRTU6000.PLC_V1.0.0.0");
			//strcpy(EPROM_General.Rtu_Detail.REC_Version,"iRTU6000.REC_V1.0.0.0");

			//{"CMD":1,"CMDState":1,"CMDType": 2,
			//"RTUID": 51,"HWVersion":"HW_1.1.2","PLCVersion":"PLC_3.1.2","RECVersion":"REC_1.1.2","HexVersion":"Hex_1.1.2"}
			//EPROM_General.Rtu_Detail.RTUId =1;
			if(!cJSON_AddNumberToObject(response_json_Obejct, "RTUID",EPROM_General.Rtu_Detail.RTUId))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "RTUID",EPROM_General.Rtu_Detail.RTUId))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddStringToObject(response_json_Obejct, "HWVersion", EPROM_General.Rtu_Detail.HW_Version))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "HWVersion", EPROM_General.Rtu_Detail.HW_Version))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddStringToObject(response_json_Obejct, "PLCVersion", EPROM_General.Rtu_Detail.PLC_Version))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "PLCVersion", EPROM_General.Rtu_Detail.PLC_Version))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddStringToObject(response_json_Obejct, "RECVersion", EPROM_General.Rtu_Detail.REC_Version))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "RECVersion", EPROM_General.Rtu_Detail.REC_Version))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddStringToObject(response_json_Obejct, "HexVersion",DEFAULT_FV_VERSION))// EPROM_General.Rtu_Detail.Hex_Version))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "HexVersion",DEFAULT_FV_VERSION)) //EPROM_General.Rtu_Detail.Hex_Version))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case CUST_INFO:
		{
			//{"CMD": 1,"CMDState": 2,"CMDType": 2,
			//"Project Code": "PNC9011","Site Name": "Chandigrah-Loc-11","Time zone": "UTC+05:30","Lattitude": 354.22,"Longitude": 910.33,
			//"RTU ID": 51,"Client ID": 1,"Reader ID": 1}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "RTUID",EPROM_General.Rtu_Detail.RTUId))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "RTUID",EPROM_General.Rtu_Detail.RTUId))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "ClientID",EPROM_General.Cust_Detail.Client_Id))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "ClientID",EPROM_General.Cust_Detail.Client_Id))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "ReaderID",EPROM_General.Cust_Detail.Reader_Id))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "ReaderID",EPROM_General.Cust_Detail.Reader_Id))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "ProjectCode",EPROM_General.Cust_Detail.Proj_Code))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "ProjectCode",EPROM_General.Cust_Detail.Proj_Code))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "SiteName",EPROM_General.Cust_Detail.Site_Name))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "SiteName",EPROM_General.Cust_Detail.Site_Name))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "Timezone",EPROM_General.Cust_Detail.Time_zone))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "Timezone",EPROM_General.Cust_Detail.Time_zone))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_sign",EPROM_General.Cust_Detail.Tz_sign))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_sign",EPROM_General.Cust_Detail.Tz_sign))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_hours",EPROM_General.Cust_Detail.Tz_hours))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_hours",EPROM_General.Cust_Detail.Tz_hours))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_minutes",EPROM_General.Cust_Detail.Tz_min))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Timezone_minutes",EPROM_General.Cust_Detail.Tz_min))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Lattitude",EPROM_General.Cust_Detail.Lattitude))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Lattitude",EPROM_General.Cust_Detail.Lattitude))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Longitude",EPROM_General.Cust_Detail.Longitude))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Longitude",EPROM_General.Cust_Detail.Longitude))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case AI_DI_DO_INFO:
		{
			//{"CMD": 1,"CMDState": 3,"CMDType": 2,
			//"DI": 16,"DO": 8,"TotalAI": 6,
			//"AI": [{"id": 1,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0},
			//		 {"id": 2,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0},
			//	     {"id": 3,"type": 1,"sLow": 12.3,"sHigh": 15.5,"CalZ": 0,"CalS": 0}]}

		    cJSON *Ais = NULL;
		   	int index=0;

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalDI",EPROM_General.AI_DI_DO_Detail.Total_Di))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalDI",EPROM_General.AI_DI_DO_Detail.Total_Di))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalDO",EPROM_General.AI_DI_DO_Detail.Total_Do))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalDO",EPROM_General.AI_DI_DO_Detail.Total_Do))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalAI",EPROM_General.AI_DI_DO_Detail.Total_Ai))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalAI",EPROM_General.AI_DI_DO_Detail.Total_Ai))
					ret = FAILED_ADD_JSON_OBJECT;
			}

		    Ais = cJSON_AddArrayToObject(response_json_Obejct, "AI");
		    if (Ais == NULL)
		    {
		    	Ais = cJSON_AddArrayToObject(response_json_Obejct, "AI");
		    	if (Ais == NULL)
		    		ret = FAILED_CREATE_ARRAY_JSON_OBJECT;
		    }
		    else
		    {
				for (index = 0; index < DEFAULT_AI; ++index)// max number of AI
				{
					cJSON *Ai = cJSON_CreateObject();
					if (Ai == NULL)
					{
						Ai = cJSON_CreateObject();
						if (Ai == NULL)
							ret = FAILED_CREATE_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "id",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].Id))
					{
						if(!cJSON_AddNumberToObject(Ai, "id",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].Id))
							ret = FAILED_ADD_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "type",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].AI_ch_Type))
					{
						if(!cJSON_AddNumberToObject(Ai, "type",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].AI_ch_Type))
							ret = FAILED_ADD_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "sLow",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleLo))
					{
						if(!cJSON_AddNumberToObject(Ai, "sLow",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleLo))
							ret = FAILED_ADD_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "sHigh",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleHi))
					{
						if(!cJSON_AddNumberToObject(Ai, "sHigh",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].scaleHi))
							ret = FAILED_ADD_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "CalZ",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calZ))
					{
						if(!cJSON_AddNumberToObject(Ai, "CalZ",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calZ))
							ret = FAILED_ADD_JSON_OBJECT;
					}

					if(!cJSON_AddNumberToObject(Ai, "CalS",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calS))
					{
						if(!cJSON_AddNumberToObject(Ai, "CalS",EPROM_General.AI_DI_DO_Detail.AI_Detail[index].calS))
							ret = FAILED_ADD_JSON_OBJECT;
					}
					cJSON_AddItemToArray(Ais, Ai);
				}
		    }
		}
		break;
		case SERIAL_INFO:
		{
			//{"CMD": 1,"CMDState": 4,"CMDType": 1,
			//  "RS232_1": {"Enable": 1,"Protocol": 3,"Master": 0,"PortID": 1,"BaudRate": 9600,"PollFreq": 1},
			//	"RS232_2": {"Enable": 1,"Protocol": 3,"Master": 0,"PortID": 1,"BaudRate": 9600,"PollFreq": 1},
			//	"RS485_1": {"Enable": 1,"Protocol": 3,"Master": 0,"PortID": 1,"BaudRate": 9600,"PollFreq": 1},
			//  "RS485_2": {"Enable": 1,"Protocol": 3,"Master": 0,"PortID": 1,"BaudRate": 9600,"PollFreq": 1}}

			cJSON *RS232_1_json_Obejct = NULL;
			cJSON *RS232_2_json_Obejct = NULL;
			cJSON *RS485_1_json_Obejct = NULL;
			cJSON *RS485_2_json_Obejct = NULL;
			//JSON_ERROR_RESPONSE ret;

			ret = JSON_SUCCESS;

			RS232_1_json_Obejct = cJSON_CreateObject();
			if (RS232_1_json_Obejct == NULL)
			{
				RS232_1_json_Obejct = cJSON_CreateObject();
				if (RS232_1_json_Obejct == NULL)
					ret = FAILED_CREATE_JSON_OBJECT;
			}

			RS232_2_json_Obejct = cJSON_CreateObject();
			if (RS232_2_json_Obejct == NULL)
			{
				RS232_2_json_Obejct = cJSON_CreateObject();
				if (RS232_2_json_Obejct == NULL)
					ret = FAILED_CREATE_JSON_OBJECT;
			}

			RS485_1_json_Obejct = cJSON_CreateObject();
			if (RS485_1_json_Obejct == NULL)
			{
				RS485_1_json_Obejct = cJSON_CreateObject();
				if (RS485_1_json_Obejct == NULL)
					ret = FAILED_CREATE_JSON_OBJECT;
			}

			RS485_2_json_Obejct = cJSON_CreateObject();
			if (RS485_2_json_Obejct == NULL)
			{
				RS485_2_json_Obejct = cJSON_CreateObject();
				if (RS485_2_json_Obejct == NULL)
					ret = FAILED_CREATE_JSON_OBJECT;
			}

			// ************************************** 	RS232_1   ***********************************************
			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Enable",EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Enable",EPROM_General.S_Comm.Rs232_1_Info.S_Co_En_Di ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Protocol",EPROM_General.S_Comm.Rs232_1_Info.S_Protocol ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Protocol",EPROM_General.S_Comm.Rs232_1_Info.S_Protocol ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Master",EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "Master",EPROM_General.S_Comm.Rs232_1_Info.S_Ma_sl_Cu ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "PortID",EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "PortID",EPROM_General.S_Comm.Rs232_1_Info.S_Port_Id ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "BaudRate",EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "BaudRate",EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "PollFreq",EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq ))
			{
				if(!cJSON_AddNumberToObject(RS232_1_json_Obejct, "PollFreq",EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq ))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			cJSON_AddItemToObject(response_json_Obejct, "RS232_1", RS232_1_json_Obejct);
		}
		break;
		case CELL_MODEM_INFO:
		{
			//{"CMD": 1,"CMDState": 6,"CMDType": 1,
			//"Enable": 1,"CellInterface": 0,"APN": "airtelgprs.com ","Protocol": 3,"MQTTBrokerIP": "199.199.51.23",
			//"MQTTBrokerPort": 1883,"MQTTClientId": "12346991","MQTTUserName": "admin","MQTTPWD": "cimcon@123",
			//"MQTTPubTopic": "pub_1","MQTTSubTopic": "sub_1","MQTTLiveFreq": "60","MQTTConnectionMode": 0}
			char IPV4PRINT[30];

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.Mo_Comm.Mo_Co_En_Di))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.Mo_Comm.Mo_Co_En_Di))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CellInterface",EPROM_General.Mo_Comm.Mo_Com_Int))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "CellInterface",EPROM_General.Mo_Comm.Mo_Com_Int))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "APN",EPROM_General.Mo_Comm.Mo_APN))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "APN",EPROM_General.Mo_Comm.Mo_APN))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Protocol",EPROM_General.Mo_Comm.Mo_Proto))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Protocol",EPROM_General.Mo_Comm.Mo_Proto))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			//memset(IPV4PRINT , 0, sizeof(IPV4PRINT));
			//sprintf(IPV4PRINT,"%d.%d.%d.%d",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP[0],EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP[1],EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP[2],EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP[3]);

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTBrokerIP",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTBrokerIP",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTBrokerPort",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTBrokerPort",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTClientId",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTClientId",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Cli_Id))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTUserName",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTUserName",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTPWD",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTPWD",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTPubTopic",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTPubTopic",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_PUB_Topic))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTSubTopic",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "MQTTSubTopic",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Sub_Topic))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTLiveFreq",EPROM_General.Mo_Comm.MQTT_LiveFreq))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTLiveFreq",EPROM_General.Mo_Comm.MQTT_LiveFreq))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTConnectionMode",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "MQTTConnectionMode",EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Comm_Mode))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case GPS_INFO:
		{
			//{"CMD": 1,"CMDState": 8,"CMDType": 2,
			//"Enable": 0,"PollFreq": 10}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.gpsDetails.GPS_Co_En_Di))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.gpsDetails.GPS_Co_En_Di))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "PollFreq",EPROM_General.gpsDetails.GPS_Poll_Freq))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "PollFreq",EPROM_General.gpsDetails.GPS_Poll_Freq))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case SD_CARD_INFO:
		{
			//{"CMD": 1,"CMDState": 9,"CMDType": 2,
			//"Enable": 0,�Size�:512}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.sdCardDetails.SD_Card_Co_En_Di))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.sdCardDetails.SD_Card_Co_En_Di))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Size",EPROM_General.sdCardDetails.SD_Card_Size))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Size",EPROM_General.sdCardDetails.SD_Card_Size))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case RTC_INFO:
		{
			//{"CMD": 1,"CMDState": 10,"CMDType": 2,
			//"RTCDateTime": "2023-01-13 14:29:37"}

//			if(!cJSON_AddStringToObject(response_json_Obejct, "RTCDateTime",Config.RTC_D_T))
//			{
//				if(!cJSON_AddStringToObject(response_json_Obejct, "RTCDateTime",Config.RTC_D_T))
//					ret = FAILED_ADD_JSON_OBJECT;
//			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "DD",gDate.Date))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "DD",gDate.Date))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "MM",gDate.Month))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "MM",gDate.Month))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "YY",gDate.Year+2000))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "YY",gDate.Year+2000))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "HH",gTime.Hours))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "HH",gTime.Hours))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "MN",gTime.Minutes))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "MN",gTime.Minutes))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "SS",gTime.Seconds))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "SS",gTime.Seconds))
					ret = FAILED_ADD_JSON_OBJECT;
			}

		}
		break;
		case HISTORY_LOGRATE_INFO:
		{
			//{"CMD": 1,"CMDState": 11,"CMDType": 2,
			//"lograte": 8,"Enable": 1,"maxLograteTimeSliceDelayS":900}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.History_En_Di))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "Enable",EPROM_General.History_En_Di))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "lograte",EPROM_General.LogRate))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "lograte",EPROM_General.LogRate))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case DO_MODE_INFO:
		{
			//{"CMD": 1,"CMDState": 12,"CMDType": 1,
			//"DOMode": 0,"DOValue": "1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "DOMode",EPROM_General.DoModeDetails.Do_Mode))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "DOMode",EPROM_General.DoModeDetails.Do_Mode))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			unsigned char temp_DO_key_String[35];

			for(unsigned index_i=0;index_i<12;index_i++)  // TODO : replace 10 with variable
			{
				if(EPROM_General.DoModeDetails.DO_Value[index_i] == 0)
				{
					temp_DO_key_String[index_i] = '0';//EPROM_General.DoModeDetails.DO_Value[index_i]=0;
				}
				else if(EPROM_General.DoModeDetails.DO_Value[index_i] == 1)
				{
					temp_DO_key_String[index_i] = '1';//EPROM_General.DoModeDetails.DO_Value[index_i]=1;
				}
				else
				{
					temp_DO_key_String[index_i] = '0';
				}
			}
			for(unsigned index_i=12;index_i<32;index_i++)  // TODO : replace 10 with variable
			{
				temp_DO_key_String[index_i] = '0';
			}

			if(!cJSON_AddStringToObject(response_json_Obejct, "DOValue",(const char * const)temp_DO_key_String))
			{
				if(!cJSON_AddStringToObject(response_json_Obejct, "DOValue",(const char * const)temp_DO_key_String))
					ret = FAILED_ADD_JSON_OBJECT;
			}
		}
		break;
		case SCHEDULE_INFO:
		{
			//{"CMD": 1,"CMDState": 13,"CMDType": 2,
			//"TotalSch": 5,"Schedule": [
			//{"SchID": 1,"Enable": 1,"StartHH": 11,"StartMin": 16,"StopHH": 21,"StopMin": 3},
			//{"SchID": 2,"Enable": 0,"StartHH": 12,"StartMin": 17,"StopHH": 22,"StopMin": 4},
			//{"SchID": 3,"Enable": 1,"StartHH": 13,"StartMin": 18,"StopHH": 1,"StopMin": 5},
			//{"SchID": 4,"Enable": 0,"StartHH": 14,"StartMin": 19,"StopHH": 2,"StopMin": 8},
			//{"SchID": 5,"Enable": 1,"StartHH": 15,"StartMin": 20,"StopHH": 22,"StopMin": 11}]}

			cJSON *Schedules = NULL;
			int index=0;

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalSch",EPROM_Schedule.Total_No_Schedule))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalSch",EPROM_Schedule.Total_No_Schedule))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalFrame",(((EPROM_Schedule.Total_No_Schedule % 8)!= 0)?((EPROM_Schedule.Total_No_Schedule/8)+1):(EPROM_Schedule.Total_No_Schedule/8))))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalFrame",(((EPROM_Schedule.Total_No_Schedule % 8)!= 0)?((EPROM_Schedule.Total_No_Schedule/8)+1):(EPROM_Schedule.Total_No_Schedule/8))))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "CurrentFrame",Fream_id))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "CurrentFrame",Fream_id))
					ret = FAILED_ADD_JSON_OBJECT;
			}

		    Schedules = cJSON_AddArrayToObject(response_json_Obejct, "Schedule");
		    if (Schedules == NULL)
		    {
		    	Schedules = cJSON_AddArrayToObject(response_json_Obejct, "Schedule");
				if (Schedules == NULL)
		    	   	ret = FAILED_CREATE_ARRAY_JSON_OBJECT;
		    }

//		    Max_No_Schedule = (8 * (Fream_id - 1)) +8;
//		    ((((8 * (Fream_id - 1)) + 8)<EPROM_Schedule.Total_No_Schedule)?(8 * (Fream_id - 1)) + 8):(8 * (Fream_id - 1)) + ))
		    for (index = (8 * (Fream_id - 1)) ; index <((((8 * (Fream_id - 1)) +8)<EPROM_Schedule.Total_No_Schedule)?(8 * (Fream_id - 1)) +8 :EPROM_Schedule.Total_No_Schedule); ++index)
		    {

		    	cJSON *Schedule = cJSON_CreateObject();
		        if (Schedule == NULL)
				{
		        	cJSON *Schedule = cJSON_CreateObject();
					if (Schedule == NULL)
						ret = FAILED_CREATE_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "SchID",EPROM_Schedule.Schedule[index].Sch_Id))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "SchID",EPROM_Schedule.Schedule[index].Sch_Id))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "Enable",EPROM_Schedule.Schedule[index].Sch_En_Di))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "Enable",EPROM_Schedule.Schedule[index].Sch_En_Di))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "StartHH",EPROM_Schedule.Schedule[index].Start_HH))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "StartHH",EPROM_Schedule.Schedule[index].Start_HH))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "StartMin",EPROM_Schedule.Schedule[index].Start_Min))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "StartMin",EPROM_Schedule.Schedule[index].Start_Min))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "StopHH",EPROM_Schedule.Schedule[index].Stop_HH))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "StopHH",EPROM_Schedule.Schedule[index].Stop_HH))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		    	if(!cJSON_AddNumberToObject(Schedule, "StopMin",EPROM_Schedule.Schedule[index].Stop_Min))
				{
		    		if(!cJSON_AddNumberToObject(Schedule, "StopMin",EPROM_Schedule.Schedule[index].Stop_Min))
		    			ret = FAILED_ADD_JSON_OBJECT;
				}

		        cJSON_AddItemToArray(Schedules, Schedule);
		    }
		}
		break;
		case MODBUS_INFO:
		{
			//{"CMD": 1,"CMDState": 14,"CMDType": 2,
			//"TotalQuery": 10,"TotalPara": 16,"RetryCount": 8,"ModbusQuery": [
			//{"QueryID": 1,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 2,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 3,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 4,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 5,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 6,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 7,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 8,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 9,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0},
			//{"QueryID": 10,"MasterPortID": 1,"SlaveID": 1,"StartAdd": 0,"Length": 0,"QueryType": 0,"DataType": 0}]}

			cJSON *ModbusQuerys = NULL;
			int index=0;

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalQuery",EPROM_Modbus_Quary_Detail.TotalQuery))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalQuery",EPROM_Modbus_Quary_Detail.TotalQuery))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalPara",EPROM_Modbus_Quary_Detail.TotalPara))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalPara",EPROM_Modbus_Quary_Detail.TotalPara))
					ret = FAILED_ADD_JSON_OBJECT;
			}

			if(!cJSON_AddNumberToObject(response_json_Obejct, "RetryCount",EPROM_Modbus_Quary_Detail.RetryCount))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "RetryCount",EPROM_Modbus_Quary_Detail.RetryCount))
					ret = FAILED_ADD_JSON_OBJECT;
			}
			if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalFrame",(((EPROM_Modbus_Quary_Detail.TotalQuery % 8)!= 0)?((EPROM_Modbus_Quary_Detail.TotalQuery/8)+1):(EPROM_Modbus_Quary_Detail.TotalQuery/8))))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "TotalFrame",(((EPROM_Modbus_Quary_Detail.TotalQuery % 8)!= 0)?((EPROM_Modbus_Quary_Detail.TotalQuery/8)+1):(EPROM_Modbus_Quary_Detail.TotalQuery/8))))
					ret = FAILED_ADD_JSON_OBJECT;
			}


			if(!cJSON_AddNumberToObject(response_json_Obejct, "CurrentFrame",Fream_id))
			{
				if(!cJSON_AddNumberToObject(response_json_Obejct, "CurrentFrame",Fream_id))
					ret = FAILED_ADD_JSON_OBJECT;
			}


			ModbusQuerys = cJSON_AddArrayToObject(response_json_Obejct, "ModbusQuery");
			if (ModbusQuerys == NULL)
			{
				ModbusQuerys = cJSON_AddArrayToObject(response_json_Obejct, "ModbusQuery");
				if (ModbusQuerys == NULL)
					ret = FAILED_CREATE_ARRAY_JSON_OBJECT;
			}

			for (index = (8 * (Fream_id - 1)) ; index <((((8 * (Fream_id - 1)) +8)<EPROM_Modbus_Quary_Detail.TotalQuery)?(8 * (Fream_id - 1)) +8 :EPROM_Modbus_Quary_Detail.TotalQuery); ++index)
			{
				cJSON *ModbusQuery = cJSON_CreateObject();
				if (ModbusQuery == NULL)
				{
					ModbusQuery = cJSON_CreateObject();
					if (ModbusQuery == NULL)
						ret = FAILED_CREATE_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "QueryID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].Mod_Quary_ID))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "QueryID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].Mod_Quary_ID))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "MasterPortID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mPortSelection))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "MasterPortID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mPortSelection))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "SlaveID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mSlaveId))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "SlaveID",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mSlaveId))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "StartAdd",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mRegStartAddr))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "StartAdd",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mRegStartAddr))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "Length",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mNoOfRegister))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "Length",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mNoOfRegister))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "QueryType",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mFunctionCode))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "QueryType",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mFunctionCode))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				if(!cJSON_AddNumberToObject(ModbusQuery, "DataType",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mDataType))
				{
					if(!cJSON_AddNumberToObject(ModbusQuery, "DataType",EPROM_Modbus_Quary_Detail.Mod_Quary[index].mDataType))
						ret = FAILED_ADD_JSON_OBJECT;
				}

				cJSON_AddItemToArray(ModbusQuerys, ModbusQuery);
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
			char msg_payload[1024]={0};

			if(!(cJSON_PrintPreallocated(response_json_Obejct, msg_payload,sizeof(msg_payload),0)))//false
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
		default:
		{

		}
		break;
	}
	return ret;
}

