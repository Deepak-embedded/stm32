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
#include "COM_PORT_RS232_1.h"

/**************************************************************************//**
 * Variable
 *****************************************************************************/
unsigned int count_RS232_1 = 0;
unsigned char COM_RS232_1_RX = 0;
osThreadId COM_PORT_RS232_1_TaskHandle;
uint8_t rxBuffer[1500];  // Adjust size as needed
char ResponseBuffer[1500];
void Print_Memory_RTOS_Stack(void);
/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/
extern UART_HandleTypeDef huart1;
extern char crcMatch;
extern CMD_TYPE current_cmd;
extern osThreadId defaultTaskHandle;
extern unsigned short int tHistoriDatalength;
extern unsigned char historyDataPacket[700];

extern unsigned char historyDataPackettemp[100];
extern unsigned char gStopHistoricalDataStoreCounter;
extern unsigned char gStopHistoricalDataStore;
extern int8_t timeoutCounter;
extern JSON_ERROR_RESPONSE response_ACK_JSON_frame1(COM_TYPE com_mode ,CMD_TYPE CMD, OTA_FILE_ACK iACK,char * ACK_Response); // For test only to remove check crc

/**************************************************************************//**
 * Function name 	: MX_USART1_UART_Init
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: USART1 Initialization Function
 *****************************************************************************/

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;//EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate;//9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
  HAL_UART_ReceiverTimeout_Config(&huart1,100);
  HAL_UART_EnableReceiverTimeout(&huart1);
  /* USER CODE END USART1_Init 2 */

}

/**************************************************************************//**
 * Function name 	: COM_PORT_RS232_1_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void COM_PORT_RS232_1_start()
{
	osThreadDef(COM_PORT_RS232_1Task, StartCOM_PORT_RS232_1Task, osPriorityNormal, 0, 512*10); //512
	COM_PORT_RS232_1_TaskHandle = osThreadCreate(osThread(COM_PORT_RS232_1Task), NULL);
}

///**************************************************************************//**
// * Function name 	: StartCOM_PORT_RS232_1Task
// * arguments		: 1)
// * return 		 	:
// * Note				:
// * 					:
// * 					:
// *****************************************************************************/

void StartCOM_PORT_RS232_1Task(void const * argument)
{
	osDelay(5000);
	HAL_UART_Receive_IT(&huart1, &ModbusH[COM_RS232_1].u8RxBuffer[0], sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
	for(;;)
	{
		#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
			HAL_IWDG_Refresh(&hiwdg1);
		#endif
		//count_RS232_1++;
		count_RS232_1=0;
//		Print_Memory_RTOS_Stack();
		if(ModbusH[COM_RS232_1].uModbusType == MB_MASTER)
		{
			for(unsigned int QueryNo=0;QueryNo<EPROM_Modbus_Quary_Detail.TotalQuery;QueryNo++)
			//for(unsigned int QueryNo=0;QueryNo<gNoofQueryStored;QueryNo++)
			{
				if(telegram[QueryNo].u8Validation == 0)
				{
					if(telegram[QueryNo].uPortNo==COM_RS232_1)
					{
						memset(ModbusH[COM_RS232_1].u8RxBuffer,0,sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
						Master_Send_Modbus_Query(&ModbusH[COM_RS232_1],&telegram[QueryNo]);
						if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)))  //  to increase response time || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-29
						{
							osDelay(100);
							Master_Parse_Modbus_Responce(&ModbusH[COM_RS232_1],&telegram[QueryNo]);
							osDelay(EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq);

							//sprintf((char *)print,"RS232_1 Take if(500)r\n");
							//WriteLog(1, print, 1);
						}
						else
						{
							osDelay(300);
							sprintf((char *)print,"RS232_1 port read query slave is not response in 2000 ms\r\n");
							WriteLog(1, (const char *)print, 1);
						}
					}
				}
				else
				{
					sprintf((char *)print,"COM_RS232_1 Port not Validate for Query u8Validation:%d\r\n",telegram[QueryNo].u8Validation);
					WriteLog(1, (const char *)print, 1);
				}
			}

			if(flagWriteQueryAvailabe == 1)
			{
				if(MODBUS_Write[0].mPortSelection_write == COM_RS232_1)
				{
					flagWriteQueryAvailabe = 0;//MODBUS_Write[i].mPortSelection_write
					//telegram[127].uQueryNo = i;
					telegram[126].uPortNo = MODBUS_Write[0].mPortSelection_write;//EPROM_Modbus_Quary_Detail.Mod_Quary[i].mPortSelection;
					telegram[126].u8id = MODBUS_Write[0].mSlaveId_write;//EPROM_Modbus_Quary_Detail.Mod_Quary[i].mSlaveId;          /*!< Slave address between 1 and 247. 0 means broadcast */
					telegram[126].u8fct = MODBUS_Write[0].mFunctionCode_write;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
					telegram[126].u16RegAdd = MODBUS_Write[0].mRegStartAddr_write;    /*!< Address of the first register to access at slave/s */
					telegram[126].u16CoilsNo = MODBUS_Write[0].mNoOfRegister_write;   /*!< Number of coils or registers to access */
					telegram[126].u16reg[0] = MODBUS_Write[0].mValue_write;
					telegram[126].uDataType = MODBUS_Write[0].mDataType_write;
					telegram[126].uQueryNo = MODBUS_Write[0].mWriteQueryNumber;

					memset(ModbusH[COM_RS232_1].u8RxBuffer,0,sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
					Master_Send_Modbus_Query(&ModbusH[COM_RS232_1],&telegram[126]);

					if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000))) //  to increase response time || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-29
					{
						osDelay(100);
						Master_Parse_Modbus_Responce(&ModbusH[COM_RS232_1],&telegram[126]);
						osDelay(EPROM_General.S_Comm.Rs232_1_Info.S_Poll_Freq);
					}
					else
					{
						sprintf((char *)print,"RS232_1 port write query slave is not response in 2000 ms\r\n");
						WriteLog(1, print, 1);
					}
				}
				sprintf((char *)print,"COM_RS232_1 Port Write Query\r\n");
				WriteLog(1, print, 1);
			}
			osDelay(1000);
		}
		else
		{
			if(ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000)))
			{
				osDelay(50);
				if(UART_OTAflag==1 && Pro_Application_flag==0)
				{
					UART_OTAflag=0;
					JSON_ERROR_RESPONSE JSON_ret;
					char * ACK_response;
					//UART_OTAflag=0;
					memset(ResponseBuffer,0,sizeof(ResponseBuffer));
					ACK_response = ResponseBuffer;
					char exitflag=0;
					while(exitflag < 2)
					{
						if(HAL_UART_Receive_IT(&huart1, rxBuffer, sizeof(rxBuffer)) == HAL_OK)
						{
							JSON_ret = parse_JSON_frame(UART,(char *)rxBuffer, (char *)ACK_response);
							if(JSON_ret!=JSON_SUCCESS)
							{
								JSON_ret = parse_JSON_frame(UART,(char *)rxBuffer ,(char *)ACK_response);
							}
							if((OTA_ACK_Data.CMDState == 3)&&(OTA_ACK_Data.FileType == HEX))
							{
								JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
								if(JSON_ret!=JSON_SUCCESS)
								{
									JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
								}
								int len = strlen((const char *)ACK_response);
								HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
								//netconn_write(newconn, ACK_response, len, NETCONN_COPY);// send message to client
								crcMatch =0;
								OTA_ACK_Data.CMDState = 0;
								ExtFlash_update_OTA_Data();
							}
							switch(current_cmd)
							{
								case CMD_OTA :
								{
									int len = strlen((const char *)ACK_response);
									HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
									if( (1 == crcMatch) && (OTA_ACK_Data.CMDState == 2) && (current_cmd == CMD_OTA ) )   // send Ack message after file received done
									{// this is only used for plc and rec file
										osDelay(500);
										if(OTA_ACK_Data.FileType == HEX)
										{
											JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											if(JSON_ret!=JSON_SUCCESS)
											{
												JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK,ACK_response);
											}
										len = strlen((const char *)ACK_response);
										HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
										crcMatch =0;
										reboot_device_func();
										osDelay(10000);
										//HAL_NVIC_SystemReset();
										}
										else
										{
											JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK, (char *)ACK_response);
											if(JSON_ret!=JSON_SUCCESS)
											{
												JSON_ret = response_ACK_JSON_frame1(UART , CMD_OTA, OTA_ACK_Data.OtaACK,(char *)ACK_response);
											}
											osDelay(500);
											len = strlen((const char *)ACK_response);
											HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
											crcMatch =0;
										}
									}
								}
								break;
								default:
								{

								}
								break;
							}
							osDelay(200);
						}
						else
						{
							UART_OTAflag=0;
							osDelay(200);
							if(UART_OTA_ACKflag==1)
							{
								UART_OTA_ACKflag=0;
								int len = strlen((const char *)ACK_response);
								HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
								exitflag++;
							}
						}
					}
				}

				else if(Pro_Application_flag == 1 || AI_CALflag ==1)
				{
					JSON_ERROR_RESPONSE JSON_ret;
					char * ACK_response;
					//UART_OTAflag=0;
					memset(ResponseBuffer,0,sizeof(ResponseBuffer));
					ACK_response = ResponseBuffer;

					JSON_ret = parse_JSON_frame(TCP,(char *)ModbusH[COM_RS232_1].u8RxBuffer ,ACK_response);
					if(JSON_ret!=JSON_SUCCESS)
					{
						JSON_ret = parse_JSON_frame(TCP,(char *)ModbusH[COM_RS232_1].u8RxBuffer ,ACK_response);
					}

					switch(current_cmd)
					{
						case CMD_PRODUCTION :
						{
							if(flagTCP_ID_First == 1)    				 //  "CMD": 2,"CMDState": 1
							{
								flagTCP_ID_First = 0;
								buildProIdFrameJson(2,0);
							}
							else if(flagTCP_ID_afterPowerCycle == 1)     //  "CMD": 2,"CMDState": 4
							{
								flagTCP_ID_afterPowerCycle = 0;
								buildProIdFrameJson(2,1);
							}
							else if(flagTCP_TestMethod_1_ACK == 1)  	//  "CMD": 2,"CMDState": 2
							{
								flagTCP_TestMethod_1_ACK = 0;
								buildTestMethodAckJson(2,1);
							}
							else if(flagTCP_TestMethod_1_Result == 1)	//  "CMD": 2,"CMDState": 3
							{
								flagTCP_TestMethod_1_Result = 0;
								buildTestMethodResultJson(2,1,1);
							}
							else if(flagTCP_TestMethod_2_ACK == 1)	    //  "CMD": 2,"CMDState": 5
							{
								flagTCP_TestMethod_2_ACK = 0;
								buildTestMethodAckJson(2,2);
							}
							else if(flagTCP_TestMethod_2_BAT_Fault == 1)	    //  "CMD": 2,"CMDState": 51
							{
								flagTCP_TestMethod_2_BAT_Fault = 0;
								pro_DO_DI_TestFinish=1;
								ScanDI();
								buildTestMethodAckJson_BAT(2,1);
							}
							else if(flagTCP_TestMethod_2_BAT_Charging == 1)	    //  "CMD": 2,"CMDState": 52
							{
								flagTCP_TestMethod_2_BAT_Charging = 0;
								pro_DO_DI_TestFinish=1;
								ScanDI();
								buildTestMethodAckJson_BAT(2,2);
							}
							else if(flagTCP_TestMethod_2_BAT_Full == 1)	    //  "CMD": 2,"CMDState": 53
							{
								flagTCP_TestMethod_2_BAT_Full = 0;
								pro_DO_DI_TestFinish=1;
								ScanDI();
								buildTestMethodAckJson_BAT(2,3);
							}
							else if(flagTCP_TestMethod_2_BAT_Use == 1)	    //  "CMD": 2,"CMDState": 54
							{
								flagTCP_TestMethod_2_BAT_Use = 0;
								pro_DO_DI_TestFinish=1;
								ScanDI();
								buildTestMethodAckJson_BAT(2,4);
							}
							else if(flagTCP_TestMethod_2_Result == 1)    //  "CMD": 2,"CMDState": 6
							{
								flagTCP_TestMethod_2_Result = 0;
								buildTestMethodResultJson(2,2,2);

							}
							int len = strlen((const char *)ACK_response);
							HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);
							if(key_LED_flag)
							{
								pro_checkDIDOState();
							}
						}
						break;
						case CMD_AI_CAL :
						{
							if(flagTCP_ID_AI_CALI_App == 1)
							{
								buildProIdFrameJson(2,2);
								flagTCP_ID_AI_CALI_App = 0;
							}
							else if(flagTCP_AI_Channel_CaliResponse)
							{
								buildAICaliJson(2,0,(flagTCP_AI_Channel_CaliResponse-1));
								flagTCP_AI_Channel_CaliResponse = 0;
							}
							else if(flagTCP_AI_Channel_Test_Result)
							{
								buildAICaliJson(2,1,(flagTCP_AI_Channel_Test_Result-1));
								flagTCP_AI_Channel_Test_Result = 0;
							}
							else if(flagTCP_ID_First_Modem == 1)    	 //  "CMD": 3,"CMDState": 4
							{
								flagTCP_ID_First_Modem = 0;
								buildProIdFrameJson_Modem(2,0);
							}
							else if(flagTCP_ID_afterPowerCycle_Modem == 1)     //  "CMD": 3,"CMDState": 7
							{
								flagTCP_ID_afterPowerCycle_Modem = 0;
								buildProIdFrameJson_Modem(2,1);
							}
							else if(flagTCP_TestMethod_1_ACK_Modem == 1)  	//  "CMD": 3,"CMDState": 5
							{
								flagTCP_TestMethod_1_ACK_Modem = 0;
								buildProIdFrameJson_Modem(2,2);
							}
							else if(flagTCP_TestMethod_1_Result_Modem == 1)	//  "CMD": 3,"CMDState": 6
							{
								flagTCP_TestMethod_1_Result_Modem = 0;
								buildTestMethodResultJson_Modem(2,1);
							}
							else if(flagTCP_TestMethod_2_ACK_Modem == 1)	    //  "CMD": 3,"CMDState": 8
							{
								flagTCP_TestMethod_2_ACK_Modem = 0;
								buildProIdFrameJson_Modem(2,3);
							}
							else if(flagTCP_TestMethod_2_Result_Modem == 1)    //  "CMD": 3,"CMDState": 9
							{
								flagTCP_TestMethod_2_Result_Modem = 0;
								buildTestMethodResultJson_Modem(2,2);

							}
							int len = strlen((const char *)ACK_response);
							HAL_UART_Transmit(&huart1, (uint8_t *)ACK_response, len, HAL_MAX_DELAY);

						}
						break;
						case CMD_SET_RTC :
						{

						}
						break;
						default:
						{

						}
						break;
					}
					memset(&ModbusH[COM_RS232_1].u8RxBuffer[0], 0,300);
					HAL_UART_Receive_IT(&huart1, &ModbusH[COM_RS232_1].u8RxBuffer[0], sizeof(ModbusH[COM_RS232_1].u8RxBuffer));

				}
				else if(History_READflag)
				{
				    HistoryFrameParser((char*)ModbusH[COM_RS232_1].u8RxBuffer,strlen((char*)ModbusH[COM_RS232_1].u8RxBuffer));
				    // Send history data via UART instead of UDP
				    if(tHistoriDatalength > 0)
				    {
				        // Transmit history data packet via serial port
				        HAL_UART_Transmit(&huart1,(uint8_t*)&historyDataPacket,tHistoriDatalength, 5000);  // 5 second timeout
				    }
				    else
				    {
				    	History_READflag = 0;  // Clear flag if no data
				    }


				    if (timeoutCounter > 0)
				    {
				        timeoutCounter--;

				        if (timeoutCounter == 0)
				        {
				        	osDelay(200);
				        	History_READflag = 0;            // timeout completed
				            timeoutCounter = -1; // disable timer
				        }
				    }
				    // Clear buffer and re-enable UART receive
				    memset(&ModbusH[COM_RS232_1].u8RxBuffer[0], 0,sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
				    HAL_UART_Receive_IT(&huart1, &ModbusH[COM_RS232_1].u8RxBuffer[0], sizeof(ModbusH[COM_RS232_1].u8RxBuffer));
				}
				else
					ProcessModbusSlave(COM_RS232_1);
				osDelay(50);
			}
		}
		osDelay(500);
	}
}

void Print_Memory_RTOS_Stack(void)
{
	unsigned long int uxHighWaterMark[8]={0,};

	uxHighWaterMark[0] = uxTaskGetStackHighWaterMark( defaultTaskHandle );
	uxHighWaterMark[1] = uxTaskGetStackHighWaterMark( EC200U_TaskHandle );
	uxHighWaterMark[2] = uxTaskGetStackHighWaterMark( RxRingProcess_TaskHandle );
	uxHighWaterMark[3] = uxTaskGetStackHighWaterMark( Modem_MQTT_TaskHandle );
	uxHighWaterMark[4] = uxTaskGetStackHighWaterMark( pcbplc_TaskHandle );
	uxHighWaterMark[5] = uxTaskGetStackHighWaterMark( COM_PORT_RS232_1_TaskHandle );
	uxHighWaterMark[6] = uxTaskGetStackHighWaterMark( DIDO_TaskHandle );
	uxHighWaterMark[7] = xPortGetFreeHeapSize();

	sprintf((char*)print, "Time:%02d:%02d:%02d\r\n\r\n",gTime.Hours, gTime.Minutes, gTime.Seconds);
	WriteLog(1, print, 1);

	sprintf((char *)print,"Default:%ld\r\nEC200U:%ld\r\nRing:%ld\r\n",uxHighWaterMark[0],uxHighWaterMark[1],uxHighWaterMark[2]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"MQTT:%ld\r\nPCBPLC:%ld\r\nRS232_1:%ld\r\n",uxHighWaterMark[3],uxHighWaterMark[4],uxHighWaterMark[5]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"DIDO:%ld\r\n",uxHighWaterMark[6]);
	WriteLog(1, print, 1);

	sprintf((char *)print,"HEAP:%ld\r\n\r\n===============END============\r\n",uxHighWaterMark[7]);
	WriteLog(1, print, 1);
}
