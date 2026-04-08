/*
 * COM_PORT_RS232_1.c
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "Modem_MQTT.h"
#include "string.h"
#include "pcbplccomm.h"
#include"projdefs.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
volatile uint8_t sleep_enable;
unsigned int count_Modem_MQTT = 0 , Modem_MQTT_check_sec = 0 , Modem_MQTT_reconnectCount=0;
unsigned int modem_MQTT_publish_sec = 0 ,modem_MQTT_publish_count=0,modem_MQTT_publish_success_count=0,modem_MQTT_publish_success_timer=0,storeLogdataSec=0,LiveDatapubCounter=0;
unsigned int modem_MQTT_Sub_receive_success_count=0;

uint8_t MqttBorker[100] = "203.88.128.141";//"203.88.135.128";
uint16_t MqttBrokerPort = 1884;
char MqttClientId[100] = "v1/devices/1/1/1";
uint8_t MqttClientUser[100] = "12";
uint8_t MqttClientPass[100] = "12";
uint8_t MqttSubTopic[100] = "v1/devices/2/3/5";
uint8_t MqttPubTopic[100] = "v1/devices/me/lora";
uint8_t MqttPubBuf[CIM_MAX_SIZE_OF_MQTT_PAYLOAD];
uint8_t *ACK_string;
uint8_t rd_index;
dlEntry_t dlTable[DL_TBL_SIZE];
TimerHandle_t sendTimer, readCmdTimer;
osThreadId Modem_MQTT_TaskHandle;
modem_mqtt_client_t mqtt[1];
uint8_t flag_modem_MQTT_Reconnect=0;
uint8_t flagSendData;

extern char uplink_data[UPLINK_BUF_LEN][200];
extern signed char uplink_RSSI[];
extern char uplink_SNR[];
extern uint8_t lora_msgID;
extern int MQTT_send_OTA_status;

extern unsigned char lora_modem_send_msg(const char *msg);
extern JSON_ERROR_RESPONSE response_ACK_JSON_frame1(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response); // For test only to remove check crc
extern void BuildSendDlPayload(char *mac, uint8_t msg_id, char *payload);

void parse_downlink(char* json_data)
{
    char *ptr;
    char hex_data[200] = {'\0'};
    char devSN[20] = {'\0'};  // Buffer to store devEUI

	// Extract devSN from "devSN" field
	ptr = strstr(json_data, "\"devSN\":\"");
	if(ptr != NULL)
	{
		ptr += 9; // Skip "devSN":"
		char *end_ptr = strchr(ptr, '\"');
		if(end_ptr != NULL)
		{
			int devSN_len = end_ptr - ptr;
			if(devSN_len < sizeof(devSN))
			{
				strncpy(devSN, ptr, devSN_len);
				devSN[devSN_len] = '\0';

				sprintf((char *)print,"Device SN: %s\r\n", devSN);
				WriteLog(1, (const char *)print, 1);
			}
		}
	}

	// Extract hex data from "data" field
	ptr = strstr(json_data, "\"data\":\"");
	if(ptr != NULL)
	{
		ptr += 8; // Skip "data":"
		char *end_ptr = strchr(ptr, '\"');
		if(end_ptr != NULL)
		{
			int hex_len = end_ptr - ptr;
			if(hex_len < sizeof(hex_data))
			{
				strncpy(hex_data, ptr, hex_len);
				hex_data[hex_len] = '\0';

				sprintf((char *)print,"Hex Data: %s\r\n", hex_data);
				WriteLog(1, (const char *)print, 1);
			}
		}
	}

	if(devSN[0] != '\0' && hex_data[0] != '\0')
	{
		if(!strcmp(devSN, LoRa_Modem.lora_serial_no))	// command is for GW
		{
			ReceivedDataOfLoRaClient = 1;
			convert_OTA_HextoAsciiString((char *)hex_data,(char*) lora_rx_buf_ascii);
			memcpy((void *)ModbusH[COM_LORA].u8RxBuffer,lora_rx_buf_ascii, strlen(hex_data)/2);
			ModbusH[COM_LORA].u8BufferSize = strlen((const char *)hex_data)/2;
			flagSendData = 1;
		}
		else		// command is for node
		{
			if(hex_data[2] != '1' && hex_data[3] != '0' && hex_data[6] != '6' && hex_data[7] != '6')
			{	// check ack for all downlink except RTC sync
				for(uint8_t idx = 0; idx < DL_TBL_SIZE; idx++)
				{
					if(!dlTable[idx].used)	// Free slot found
					{
						dlTable[idx].used = true;
						dlTable[idx].ackReceived = false;
						strcpy(dlTable[idx].mac,devSN);
						strncpy(dlTable[idx].payload, hex_data, sizeof(dlTable[idx].payload));
						dlTable[idx].msgId = lora_msgID;
						dlTable[idx].retries = 0;
						dlTable[idx].timeoutTick = HAL_GetTick();
						break;
					}
				}
			}
			BuildSendDlPayload(devSN, lora_msgID, hex_data);
			lora_msgID++;

			if(hex_data[2] == '0' && hex_data[3] == '3')	// Send quick response for read data request
			{
				sprintf(print, "Setting send timer to %d sec\r\n", EPROM_General.readCmdDelay);
				WriteLog(1, print, 1);
				xTimerStart(readCmdTimer, 0);
			}
			sleep_enable = 0;
		}

	}
}

/**************************************************************************//**
 * Function name 	: Modem_MQTT_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void Modem_MQTT_start()
{
	osThreadDef(Modem_MQTTTask, StartModem_MQTTTask, osPriorityNormal, 0, 512*12); //1024
	Modem_MQTT_TaskHandle = osThreadCreate(osThread(Modem_MQTTTask), NULL);
}

/**************************************************************************//**
 * Function name 	: send_timer_callback
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void send_timer_callback(TimerHandle_t xTimer) {
	flagSendData = 1;  // Ready to send data to server over MQTT
	sleep_enable = 0;
	WriteLog(1, "SendTimer Expired!\r\n", 1);
}
/**************************************************************************//**
 * Function name 	: init_send_timer
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void init_send_timer(void) {
    sendTimer = xTimerCreate(
        "SendTimer",
        pdMS_TO_TICKS(EPROM_General.LogSendDelay*1000),
        pdFALSE,          // One-shot timer
        NULL,
        send_timer_callback
    );

    if (sendTimer == NULL) {
        WriteLog(1, "Failed to create SendTimer!\r\n", 1);
    }
}

/**************************************************************************//**
 * Function name 	: init_send_timer
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void init_read_cmd_timer(void) {
    readCmdTimer = xTimerCreate(
        "Read Data Command Timer",
        pdMS_TO_TICKS(EPROM_General.readCmdDelay*1000),
        pdFALSE,          // One-shot timer
        NULL,
        send_timer_callback
    );

    if (readCmdTimer == NULL) {
        WriteLog(1, "Failed to create Read data command Timer!\r\n", 1);
    }
}

/**************************************************************************//**
 * Function name 	: StartCOM_PORT_RS232_1Task
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char senddata=0;
void StartModem_MQTTTask(void const * argument)
{
	osDelay(3000);
	ACK_string = &MqttPubBuf[0];
	fillMQttClient();
	init_send_timer();
	init_read_cmd_timer();
	xTimerStop(readCmdTimer, 0);

	for(;;)
	{
		if(sleep_enable != 1)
		{
			while(!EPROM_General.Modem_EC200_presence)
				osDelay(5000);
			//count_Modem_MQTT++;
			count_Modem_MQTT =0;
			Modem_MQTT_check_sec++;
			modem_MQTT_publish_sec++;
			//storeLogdataSec++;
			LiveDatapubCounter++;
			OTA_resetCouner++;

			if(flag_modem_MQTT_Reconnect == 1)
			{
				flag_modem_MQTT_Reconnect = 0;
				modem_MQTT_disconnect(&mqtt[0]);
				mqtt[0].status = MQTT_STATE_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT ;
				mqtt[0].connect.state = MQTT_CONNECTION_STATE_DEFAULT;
				sprintf((char *)print,"flag_modem_MQTT_Reconnect:%d\r\n",flag_modem_MQTT_Reconnect);
				WriteLog(1, print, 1);
			}

			if(Modem_gsm_network_GACT_Status == 1)
			{
				if(modem_check_mqtt_status(&mqtt[0]) == 0)
				{
					if(modem_make_mqttConnection(&mqtt[0]))
					{
						memset(MqttSubTopic,0,sizeof(MqttSubTopic));
						sprintf((char *)MqttSubTopic, "v1/downlink/%s", LoRa_Modem.lora_serial_no);
						modem_MQTTsubscribe(&mqtt[0],(const char*)&MqttSubTopic, 1);
						osDelay(100);
						modem_MQTTsubscribe(&mqtt[0],"v1/downlink/FFFFFFFFFFFFFFFF", 1);
						osDelay(100);
						sprintf((char *)MqttSubTopic, "v1/LoRAdeltaOTA/%s", LoRa_Modem.lora_serial_no);
						modem_MQTTsubscribe(&mqtt[0],(const char*)&MqttSubTopic, 1);
						Modem_MQTT_reconnectCount = 0;
					}
					else if(Modem_MQTT_reconnectCount++ > 10)	// If reconnect fails 10 times then reboot modem
					{
						flag_modem_reboot = 1;
						Modem_MQTT_reconnectCount = 0;
					}
					Modem_MQTT_check_sec = 120;
				}
				else
				{
					if(mqtt[0].mqtt_rx_state == MQTT_RX_STATE_DATA_READY)
					{
						JSON_ERROR_RESPONSE JSON_ret;

						modem_MQTT_Sub_receive_success_count++;
						if(!strncmp(mqtt[0].mqtt_rx_Topic, "v1/LoRAdeltaOTA", 15))
						{
							JSON_ret = parse_JSON_frame(MQTT, (char*)mqtt[0].mqtt_rx_data, (char*)MqttPubBuf);
							if(JSON_ret!=JSON_SUCCESS)
							{
								JSON_ret = parse_JSON_frame(MQTT, (char*)mqtt[0].mqtt_rx_data, (char*)MqttPubBuf);
							}
							MQTT_send_OTA_status = 1;
						}
						else
							parse_downlink((char*)&mqtt[0].mqtt_rx_data);
						mqtt[0].mqtt_rx_state = MQTT_RX_STATE_FREE;
					}
					MQTT_PUB_Routine();

					if(Modem_MQTT_check_sec >= 120)
					{
						Modem_MQTT_check_sec = 0;
						modem_MQTT_Check(&mqtt[0]);

					}
					if(mqtt[0].status == MQTT_STATE_CONNECTION_STATE_CONNECTED && !flagSendData)
					{
						HAL_GPIO_WritePin(MODEM_DTR_GPIO_Port, MODEM_DTR_Pin, GPIO_PIN_SET);
						modem_sleep_mode();
						osDelay(100);
					}
				}
			}
		}
		else if(flagSendData)
		{
			HAL_GPIO_WritePin(MODEM_DTR_GPIO_Port, MODEM_DTR_Pin, GPIO_PIN_RESET);
			osDelay(100);
			sleep_enable = 0;
		}

		if((OTA_START==1)&&(OTA_resetCouner>OTA_RESET_TIME))
		{
			OTA_START=0;
		}
		osDelay(60);
	}
}

void fillMQttClient()
{
	mqtt[0].client_idx = 0;

	mqtt[0].cfg.keepalive.keep_alive_time = 120;
	mqtt[0].cfg.recv_mode.msg_recv_mode = 0;
	mqtt[0].cfg.recv_mode.msg_len_enable = 1;

	mqtt[0].open.Broker = (const char*)&MqttBorker;
	mqtt[0].open.Broker_Port = MqttBrokerPort;

	mqtt[0].open.open_result = MQTT_OPEN_RESULT_DEFAULT;
	mqtt[0].open.open_result_temp = MQTT_OPEN_RESULT_DEFAULT;

	mqtt[0].close.result = MQTT_CLOSE_RESULT_DEFAULT;
	mqtt[0].close.result_temp = MQTT_CLOSE_RESULT_DEFAULT;

	mqtt[0].connect.client_id = (const char*)&MqttClientId;
	mqtt[0].connect.client_user = (const char*)&MqttClientUser;
	mqtt[0].connect.client_pass = (const char*)&MqttClientPass;
	mqtt[0].connect.ret_code = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;
	mqtt[0].connect.ret_code_temp = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;
	mqtt[0].connect.state = MQTT_CONNECTION_STATE_DEFAULT;
	mqtt[0].connect.state_temp = MQTT_CONNECTION_STATE_DEFAULT;

	mqtt[0].disconnect.result = MQTT_DISCONNECTION_RESULT_DEFAULT;
	mqtt[0].disconnect.result_temp = MQTT_DISCONNECTION_RESULT_DEFAULT;

	mqtt[0].sub.QOS = 1;
	mqtt[0].sub.topic = (const char*)&MqttSubTopic;

	//mqtt[0].pub.QOS = 1;
	//mqtt[0].pub.retain = 0;
	//mqtt[0].pub.topic = &MqttPubTopic;
	//mqtt[0].pub.msg_len = &0;


	mqtt[0].msgId = 0;
	mqtt[0].sent_result = MQTT_PACKET_SENT_RESULT_DEFAULT;
	mqtt[0].Error_code = MQTT_DISCONNECT_REASON_DEFAULT;
	mqtt[0].status = MQTT_STATE_CONNECTION_STATE_DISCONNECTED;

	mqtt[0].mqtt_rx_state = MQTT_RX_STATE_FREE;

}

unsigned char modem_check_mqtt_status(void *argument)
{
	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
	if(_mqtt->status != MQTT_STATE_CONNECTION_STATE_CONNECTED )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
/**************************************************************************//**
 * Function name 	: modem_make_mqttConnection
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

unsigned char modem_make_mqttConnection(void *argument)
{
	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;

	unsigned char res=0;
	unsigned int Delay_50=0;
	if(Pro_Application_flag)
	{
		strcpy((char *)MqttBorker,(const char *)pro_MQTT_Broker_IP);
		MqttBrokerPort = pro_MQTT_Broker_Port;
		memset(MqttClientId,0,sizeof(MqttClientId));
		sprintf(MqttClientId, "v1/devices/%d/%d/%d", EPROM_General.Cust_Detail.Client_Id, EPROM_General.Cust_Detail.Reader_Id, EPROM_General.Rtu_Detail.RTUId);
		strcpy((char *)MqttClientId,(const char *)pro_MQTT_Client_ID);
		strcpy((char *)MqttClientUser,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name);
		strcpy((char *)MqttClientPass,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass);
	}
	else
	{
		strcpy((char *)MqttBorker,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP);
		MqttBrokerPort = EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_Port;
		memset(MqttClientId,0,sizeof(MqttClientId));
		if(EPROM_General.Rtu_Detail.RTUId > 1)
			sprintf((char *)MqttClientId, "v1/devices/%d/%d/%d", EPROM_General.Cust_Detail.Client_Id, EPROM_General.Cust_Detail.Reader_Id, EPROM_General.Rtu_Detail.RTUId);
		else
			sprintf((char *)MqttClientId, "%s", ModemInfo.model_serial_number);
		strcpy((char *)MqttClientUser,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Name);
		strcpy((char *)MqttClientPass,(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Us_Pass);

		sprintf((char *)print,"modem_make_mqttConnection IP :%s, ClientID: %s\r\n",(const char *)EPROM_General.Mo_Comm.MQTT_Conn.MQTT_Bro_IP, MqttClientId);
		WriteLog(1, print, 1);
	}


	modem_MQTT_disconnect(_mqtt);

//	LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET, 	/*!< set MQTT Receive mode */
//	LWGSM_CMD_MQTT_QMTOPEN, 				/*!< set MQTT OPEN Link */
//	LWGSM_CMD_MQTT_QMTCONN_SET, 			/*!< set MQTT make Connection with broker */
//	LWGSM_CMD_MQTT_QMTSUB_SET, 				/*!< set MQTT subscribe */
//	LWGSM_CMD_MQTT_QMTPUBEX, 				/*!< MQTT Publish */
//	LWGSM_CMD_MQTT_PUB_MSG,
//	LWGSM_CMD_MQTT_QMTUNS, 					/*!< set MQTT unsubscribe */
//	LWGSM_CMD_MQTT_QMTDISC, 				/*!< Disconnect a Client from MQTT Server*/
//	LWGSM_CMD_MQTT_QMTCLOSE, 				/*!< Close a Network for MQTT Client*/

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_INIT;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET;
			_mqtt->cfg.recv_mode.msg_recv_mode = 0;
			_mqtt->cfg.recv_mode.msg_len_enable = 1;
			lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);

			///////////////////////////////////////////////////////////////////
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				Modem_AT_Command = LWGSM_CMD_MQTT_QMTCLOSE;
				_mqtt->close.result_temp = MQTT_CLOSE_RESULT_DEFAULT;
				lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
				osDelay(2000);
			}
			//////////////////////////////////////////////////////////////////

			if(xSemaphoreTake(modem_PortBlockSemaphore, 5000))
			{
				if(_mqtt->open.open_result != MQTT_OPEN_RESULT_OPEN_SUCCESS) // if already open than don't open and wait
				{
					Modem_AT_Command = LWGSM_CMD_MQTT_QMTOPEN_SET;
					_mqtt->open.Broker = (const char*)&MqttBorker;
					_mqtt->open.Broker_Port = MqttBrokerPort;
					_mqtt->open.open_result_temp = MQTT_OPEN_RESULT_DEFAULT;
					lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
				}
				else
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}

				if(xSemaphoreTake(modem_PortBlockSemaphore, 5000))
				{
					if(_mqtt->open.open_result != MQTT_OPEN_RESULT_OPEN_SUCCESS)   // if already open than don't open and wait
					{
						Delay_50 = (unsigned int)(1000/50);
						while(Delay_50)
						{
							if(_mqtt->open.open_result_temp != MQTT_OPEN_RESULT_DEFAULT)
							{
								break;
							}
							else
							{
								Delay_50--;
								osDelay(50);
							}
						}
						if(_mqtt->open.open_result_temp != _mqtt->open.open_result)
						{
							_mqtt->open.open_result = _mqtt->open.open_result_temp;
						}
					}

					if(_mqtt->open.open_result == MQTT_OPEN_RESULT_OPEN_SUCCESS)   // After Wait to get responce check for result
					{
						Modem_AT_Command = LWGSM_CMD_MQTT_QMTCONN_SET;
						_mqtt->connect.client_id = (const char*)&MqttClientId;
						_mqtt->connect.client_user = (const char*)&MqttClientUser;
						_mqtt->connect.client_pass = (const char*)&MqttClientPass;
						_mqtt->connect.ret_code_temp = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;
						_mqtt->sent_result = MQTT_PACKET_SENT_RESULT_DEFAULT;
						lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
						if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
						{
							Delay_50 = (unsigned int)(3000/50);
							while(Delay_50)
							{
								if((_mqtt->connect.ret_code_temp != MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT)||(_mqtt->sent_result != MQTT_PACKET_SENT_RESULT_DEFAULT))
								{
									break;
								}
								else
								{
									Delay_50--;
									osDelay(50);
								}
							}
							if(_mqtt->sent_result == MQTT_PACKET_SENT_RESULT_RETRANSMISSION)
							{
								Delay_50 = (unsigned int)(5000/50);
								while(Delay_50)
								{
									if((_mqtt->connect.ret_code_temp != MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT)||(_mqtt->sent_result != MQTT_PACKET_SENT_RESULT_RETRANSMISSION))
									{
										break;
									}
									else
									{
										Delay_50--;
										osDelay(50);
									}
								}
							}
							if(_mqtt->sent_result == MQTT_PACKET_SENT_RESULT_SUCCESS_ACK_RECEIVED)
							{
								//if(_mqtt->connect.ret_code_temp != MQTT_CONNECTION_RET_CODE_ACCEPTED)
								if(_mqtt->connect.ret_code_temp != MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT)
								{
									_mqtt->connect.ret_code = _mqtt->connect.ret_code_temp;
									if(_mqtt->connect.ret_code == MQTT_CONNECTION_RET_CODE_ACCEPTED)
									{
										goto End3;
									}
									else
									{
										sprintf((char *)print,"MQTT connect msg  sent successfully but not connect to broker:%d\r\n",_mqtt->connect.ret_code);
										WriteLog(1, print, 1);

										goto End2;//mqtt connect msg  sent successfully but not connect to broker
									}
								}
								else
								{
									sprintf((char *)print,"MQTT connect msg  sent successfully but not connect to broker:%d\r\n",_mqtt->connect.ret_code_temp);
									WriteLog(1, print, 1);

									goto End2;//mqtt connect msg  sent successfully but not connect to broker

								}
							}
							else
							{
								sprintf((char *)print,"MQTT connect msg not sent successfully:%d\r\n",_mqtt->sent_result);
								WriteLog(1, print, 1);

								goto End2;//mqtt connect msg not sent successfully

							}
						}
						else
						{
							sprintf((char *)print,"MQTT not get semaphore after LWGSM_CMD_MQTT_QMTCONN_SET\r\n");
							WriteLog(1, print, 1);

							goto End1;//not get semaphore after LWGSM_CMD_MQTT_QMTCONN_SET

						}
					}
					else
					{
						sprintf((char *)print,"MQTT not open successfully:%d\r\n",_mqtt->open.open_result);
						WriteLog(1, print, 1);

						goto End1;//mqtt not open success fully

					}
				}
				else
				{
					sprintf((char *)print,"MQTT not get semaphore after LWGSM_CMD_MQTT_QMTOPEN\r\n");
					WriteLog(1, print, 1);

					goto End1;//not get semaphore after LWGSM_CMD_MQTT_QMTOPEN

				}
			}
			else
			{
				sprintf((char *)print,"MQTT not get semaphore after LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET\r\n");
				WriteLog(1, print, 1);

				goto End1;//not get semaphore after LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET

			}

			End1:
			_mqtt->open.open_result = MQTT_OPEN_RESULT_DEFAULT;
			End2:
			_mqtt->connect.ret_code = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;
			End3:
			Modem_AT_Command = LWGSM_CMD_IDLE;
			xSemaphoreGive(modem_PortBlockSemaphore);

			if((_mqtt->open.open_result == MQTT_OPEN_RESULT_DEFAULT)||(_mqtt->connect.ret_code == MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT))
			{
				res = 0;

			}
			else
			{
				res = 1;
			}
		}
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}
	sprintf((char *)print,"MQTT modem_make_mqttConnection res:%d\r\n",res);
	WriteLog(1, print, 1);

	return res;
}

/**************************************************************************//**
 * Function name 	: modem_MQTT_publish
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char modem_MQTT_publish(void *argument,const char* _topic,const char* _msg,unsigned int _msg_len,unsigned char _QOS,unsigned char _retain)
{

	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;
	unsigned char res=0;
	unsigned int Delay_50 = 0;

	if(_QOS>2)
	{
		_QOS = 0;
	}

	if(_retain > 1)
	{
		_retain = 0;
	}

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_PUBLISH;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_MQTT_QMTPUBEX;
			_mqtt->pub.QOS = _QOS;
			_mqtt->pub.retain = _retain;
			_mqtt->pub.topic = _topic;
			_mqtt->pub.msg_len = _msg_len;
			_mqtt->sent_result = MQTT_PACKET_SENT_RESULT_DEFAULT;
			_mqtt->msgId++;
			lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000/portTICK_PERIOD_MS))
			{
				osDelay(10);
				Modem_AT_Command = LWGSM_CMD_MQTT_PUB_MSG;
//				WriteLog(1, _msg, 1);
				_mqtt->pub.msg = _msg;
				_mqtt->pub.msg_len = _msg_len;
				//count_2++;
				lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
				//count_3++;
				if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
				{
					//count_4++;
					Delay_50 = (unsigned int)(5000/50);
					while(Delay_50)
					{
						if((_mqtt->sent_result != MQTT_PACKET_SENT_RESULT_DEFAULT))
						{
							break;
						}
						else
						{
							Delay_50--;
							osDelay(50);
						}
					}
					if(_mqtt->sent_result != MQTT_PACKET_SENT_RESULT_SUCCESS_ACK_RECEIVED)
					{
						res = 0;
					}
					else
					{
						res = 1;
					}
				}
			}
			else
			{
				res = 0;
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		xSemaphoreGive(modem_PortBlockSemaphore);
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}
	else
	{
		res = 0;
	}
	return res;
}
/**************************************************************************//**
 * Function name 	: modem_MQTT_disconnect
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char modem_MQTT_disconnect(void *argument)
{

	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_DISCONNECT;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_MQTT_QMTDISC;
			lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}
			}
		}

		Modem_AT_Command = LWGSM_CMD_IDLE;
		xSemaphoreGive(modem_PortBlockSemaphore);
		_mqtt->open.open_result = MQTT_OPEN_RESULT_DEFAULT;
		_mqtt->connect.ret_code = MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT;

		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);

		sprintf((char *)print,"modem_MQTT_disconnect:%d\r\n",Modem_AT_Command);
		WriteLog(1, print, 1);
	}
	return 1;
}
/**************************************************************************//**
 * Function name 	: Modem_MQTT_Check
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

unsigned char modem_MQTT_Check(void *argument)
{
	lwgsmr_t res=0;
	unsigned int Delay_50=0;
	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_CHECK;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_MQTT_QMTCONN_GET;
			_mqtt->connect.state_temp = MQTT_CONNECTION_STATE_DEFAULT;
			lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				Delay_50 = (unsigned int)(5000/50);
				while(Delay_50)
				{
					if((_mqtt->connect.state_temp != MQTT_CONNECTION_STATE_DEFAULT))
					{
						break;
					}
					else
					{
						Delay_50--;
						osDelay(50);
					}
				}
				if(_mqtt->connect.state_temp != _mqtt->connect.state)
				{
					_mqtt->connect.state = _mqtt->connect.state_temp;
				}
			}
			else
			{
				// semaphore not release after LWGSM_CMD_MQTT_QMTCONN_GET
			}
		}
	}
	Modem_AT_Command = LWGSM_CMD_IDLE;
	xSemaphoreGive(modem_PortBlockSemaphore);

	modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
	xSemaphoreGive(modem_SequenceBlockSemaphore);

	sprintf((char *)print,"MQTT modem_MQTT_Check res:%d\r\n",res);
	WriteLog(1, print, 1);

	return res;
}
/**************************************************************************//**
 * Function name 	: Modem_MQTT_Check
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char modem_MQTTsubscribe(void *argument,const char* _subTopic,unsigned char _QOS)
{
	lwgsmr_t res=0;
	unsigned int Delay_50=0;
	modem_mqtt_client_t *_mqtt =  (modem_mqtt_client_t *)argument;

	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_SUB;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_MQTT_QMTSUB_SET;
			_mqtt->sub.QOS = _QOS;
			_mqtt->sub.topic = _subTopic;
			_mqtt->msgId++;
			_mqtt->sent_result = MQTT_PACKET_SENT_RESULT_DEFAULT;
			lwgsmi_initiate_cmd(Modem_AT_Command,_mqtt);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				Delay_50 = (unsigned int)(5000/50);
				while(Delay_50)
				{
					if(_mqtt->sent_result != MQTT_PACKET_SENT_RESULT_DEFAULT)
					{
						break;
					}
					else
					{
						Delay_50--;
						osDelay(50);
					}
				}
				if(_mqtt->sent_result == MQTT_PACKET_SENT_RESULT_SUCCESS_ACK_RECEIVED)
				{
					res = 1;
				}
			}
			else
			{
				// semaphore not release after LWGSM_CMD_MQTT_QMTSUB_SET
				sprintf((char *)print,"MQTT semaphore not release after LWGSM_CMD_MQTT_QMTSUB_SET\r\n");
				WriteLog(1, print, 1);
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		xSemaphoreGive(modem_PortBlockSemaphore);

		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}

	sprintf((char *)print,"MQTT modem_MQTTsubscribe res:%d\r\n",res);
	WriteLog(1, print, 1);


	return res;
}

void MQTT_PUB_Routine(void)
{
	unsigned char pubTopic=0; // 0 = "v1/devices/Response" 1 = "v1/devices/me/lora"

	if(MQTT_send_OTA_status)
	{
		MQTT_send_OTA_status = 0;
		pubTopic = 3;
	}
	else if(flagMQTTPubSchedule)
	{
		flagMQTTPubSchedule = 0;
		buildGetScheduleJson(1,pubScheduleBlock);
		pubTopic = 0;
	}
	else if(flagMQTTPubGetMode)
	{
		flagMQTTPubGetMode = 0;
		buildGetModeResponseJson(1);
		pubTopic = 0;
	}
	else if(flagMqttPubLogData && flagSendData)
	{
		buildLograteDataJson(1);
		pubTopic = 1;
		if(strlen(uplink_data[rd_index]) == 0)	// If next data is not available then clear pub flag
		{
			flagMqttPubLogData = 0;
			flagSendData = 0;
			xTimerStop(sendTimer, 0);
		}
	}

	else
	{
		return;
	}

	if(pubTopic == 0)
	{
		modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)&MqttPubBuf),1,0);
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/Response\"\r\n");
		WriteLog(1, print, 1);
	}
	else if(pubTopic == 1)
	{
		modem_MQTT_publish(&mqtt[0],"v1/devices/me/lora",(const char*)MqttPubBuf,strlen((const char *)&MqttPubBuf),1,0);
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/me/lora\"\r\n");
		WriteLog(1, print, 1);
	}
	else if(pubTopic == 3)
	{
		extern char crcMatch;
		extern CMD_TYPE current_cmd;
		JSON_ERROR_RESPONSE JSON_ret;

		modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
		if( (1 == crcMatch) && (OTA_ACK_Data.CMDState == 2) && (current_cmd == CMD_OTA ) )   // send Ack message after file received done
		{
			JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
			if(JSON_ret!=JSON_SUCCESS)
			{
				JSON_ret = response_ACK_JSON_frame1(MQTT , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_string);
			}
			modem_MQTT_publish(&mqtt[0],"v1/devices/Response",(const char*)MqttPubBuf,strlen((const char *)MqttPubBuf),0,0);
			crcMatch =0;

			if(OTA_ACK_Data.FileType == HEX)
			{
				reboot_device_func();
				osDelay(10000);
			}
		}
		sprintf((char *)print,"MQTT modem_MQTT_publish \"v1/devices/Response\"\r\n");
		WriteLog(1, print, 1);

	}

}

void modem_sleep_mode(void)
{
		if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
		{
			modemCommandSequence = MODEM_CMD_MODEM_INFO;
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				osDelay(100);
				Modem_AT_Command = LWGSM_CMD_QSCLK;
				lwgsmi_initiate_cmd(Modem_AT_Command,0);
				osDelay(1000);

				xSemaphoreGive(modem_PortBlockSemaphore);
			}
			Modem_AT_Command = LWGSM_CMD_IDLE;
			modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
			xSemaphoreGive(modem_SequenceBlockSemaphore);
		}

}
