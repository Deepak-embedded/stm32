/*
 * RAK_Lora.c
 *
 *  Created on: Sep 16, 2025
 *      Author: Admin
 */

#include "Lora_AT_Types.h"
#include "main.h"
#include "Modem_MQTT.h"
#include"timers.h"

/**************************************************************************//**
 * Macros
 *****************************************************************************/
#define BUTTON_DEBOUNCE_MS 50           // 50ms debounce time
#define PENDING_ACK_TIMEOUT_MS	3000
#define AT_PORT_SEND_STR(str)       lora_serial_send_fn((const void*)(str), (size_t)strlen(str))
#define AT_PORT_SEND_CONST_STR(str) lora_serial_send_fn((const void*)(str), (size_t)(sizeof(str) - 1))
#define AT_PORT_SEND_CHR(ch)        lora_serial_send_fn((const void*)(ch), (size_t)1)
#define AT_PORT_SEND_FLUSH()        lora_serial_send_fn(NULL, 0)
#define AT_PORT_SEND(d, l)          lora_serial_send_fn((const void*)(d), (size_t)(l))
// Time intervals (in milliseconds)
#define ON_DURATION_MS     			(1 * 60 * 1000)   // 1 minute
#define OFF_DURATION_MS    			(14 * 60 * 1000)  // 14 minutes
#define LORA_OTA_CHUNK_SIZE			64 // bytes

/* Beginning and end of every AT command */
#define AT_PORT_SEND_BEGIN_AT()                                                                                        \
    do {                                                                                                               \
        AT_PORT_SEND_CONST_STR("AT");                                                                                  \
    } while (0)
#define AT_PORT_SEND_END_AT()                                                                                          \
    do {                                                                                                               \
        AT_PORT_SEND(CRLF, CRLF_LEN);                                                                                  \
        AT_PORT_SEND(NULL, 0);                                                                                         \
    } while (0)


/**************************************************************************//**
 * Variable
 *****************************************************************************/
typedef enum {
    LORA_LED_OFF = 0,
    LORA_LED_TX_FAST,     // Fast blink for TX (5 fast blinks)
    LORA_LED_RX_SLOW,     // Slow blink for RX (3 slow blinks)
    LORA_LED_IDLE,        // Single blink for idle/status
    LORA_LED_ERROR        // Very fast blink for error
} lora_led_pattern_t;

int16_t Lora_AT_Command;
uint8_t lora_msgID;
uint8_t loraChannel;
uint8_t retry_count;
uint8_t ack_received;
uint8_t flagLoRA_txDone;
uint8_t flagLoraRx;
uint8_t flagExLog = 1;
char ack_payload[30][23];
uint8_t ack_rd_index;
uint8_t lora_led_state = 0;
uint8_t lora_led_counter = 0;
lora_led_pattern_t current_led_pattern = LORA_LED_OFF;
static uint8_t led_blink_delay = 0;
static uint8_t delay_counter = 0;
LoraOta_t LoraOTA;
nodeEntry_t nodesCon[MAX_NODES];
uint8_t NoConnNodes;
uint8_t pro_flagLoRA_txDone;

JANPATCH_STREAM source = {
	.offset = HEX_FILE_START_ADDRESS,
	.size = (size_t)HEX_FILE_TOTAL_SEC*SECTOR_SIZE,
};

JANPATCH_STREAM patch = {
	.offset = DELTA_HEX_3100_START_ADD,
	.size = (size_t)DELTA_HEX_3100_TOTAL_SEC*SECTOR_SIZE,
};
JANPATCH_STREAM target = {
	.offset = TGT_HEX_FILE_START_ADDRESS,
	.size = (size_t)TGT_HEX_FILE_TOTAL_SEC*SECTOR_SIZE,
};

osSemaphoreId lora_PortBlockSemaphore;
osSemaphoreId lora_SequenceBlockSemaphore;
osThreadId Lora_TaskHandle;
TimerHandle_t ackTimer, otaTimer;

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/
extern uint8_t Lora_RX_Buff[525];        	//LoRA buffer to fill from Rx interrupt
extern lwrb_t lora_rx_rb;  				//LoRA Ring buffer instance for RX data
extern uint8_t lora_rx_rb_data[1000];		//LoRA Ring buffer data array for RX DMA
extern CMD_TYPE current_cmd;
extern char crcMatch, flagSendACK;
extern char uplink_data[UPLINK_BUF_LEN][200];
extern signed char uplink_RSSI[];
extern char uplink_SNR[];
extern uint8_t wr_index;
extern uint32_t LoraSendACKtime;
extern unsigned char flagMqttPubLogData;
extern uint32_t lastToggleTime ;
extern uint8_t  outputState ; // 0 = OFF period, 1 = ON period
extern unsigned char flagSendData;
extern uint8_t rd_index;
extern TimerHandle_t sendTimer;
extern uint8_t ack_wr_index;
extern dlEntry_t dlTable[DL_TBL_SIZE];
extern char flagSyncNodeTime;
extern janpatch_ctx ctx;
extern struct OTA OTA_Data;
extern UART_HandleTypeDef huart1;
extern RTC_TimeTypeDef gTime;

extern void set_pulse_do_polarity();
extern float swap_float(float input);
extern int janpatch(janpatch_ctx , JANPATCH_STREAM *source, JANPATCH_STREAM *patch, JANPATCH_STREAM *target);

/**************************************************************************//**
 * Function Prototype
 *****************************************************************************/
void lora_modem_network_mode_check(void);
void ack_timer_callback(TimerHandle_t xTimer);
void PT_GPIO_Timed_Control(void);
void LoRa_LED_Init(void);
void LoRa_LED_Set_Pattern(lora_led_pattern_t pattern);
void LoRa_LED_Process(void);
void LoRa_LED_TX_Indicate(void);
void LoRa_LED_RX_Indicate(void);
void LoRa_LED_Error_Indicate(void);
void LoRa_LED_Off(void);

void Lora_initiate_cmd(lora_cmd_t cmd,void *argument)
{
	switch (cmd)
	{
        // LORA AT Commands
        case RAK_LORA_DEVICE_AT:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATE:
		{
			AT_PORT_SEND_CONST_STR("ATE");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_ATZ:
		{
			AT_PORT_SEND_CONST_STR("ATZ");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_SN_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+SN=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_FIRMVARE_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+VER=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_HWMODEL_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+HWMODEL=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_HWID_GET:
		{
			/* Enable detailed error messages */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+HWID=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAUD_GET:
		{
			/* Get manufacturer */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAUD=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CLASS_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CLASS=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BGW_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BGW=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAND_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAND=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_BAND_SET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+BAND=");
//			Lora_initiate_cmd(LWGSM_U32(EPROM_LoRa_Modem.lora_active_region_set), 0, 0);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_MODE_SET:
		{
			/* SET modem to P2P mode */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWM=0");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_MODE_GET:
		{
			/* Get modem P2P mode */
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+NWM=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PRECV_SET:
		{
			char command[14] = "+PRECV=";
			if(argument != NULL)
			{
				strcat(command, (char *) argument);
			}
			else
				strcat(command, "0");

			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_PARAM_SET:
		{
			char command[30] = "+P2P=";
			strcat(command, (char *) argument);

			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_STR(command);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_P2P_PARAM_GET:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+P2P=?");
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PSEND:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+PSEND=");
			AT_PORT_SEND_STR((const void*)argument);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PENCRYPT_EN:
		{
			char command[] = "+ENCRY=";	// Enable Encryption
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_STR(command);
			AT_PORT_SEND_STR((const void*)argument);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_PENCRYPT_KEY:
		{
			char command[] = "+ENCKEY=01020304050607080102030405060708";	// Set Encryption Key
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_STR(command);
			AT_PORT_SEND_END_AT();
			HAL_Delay(100);
			char command1[] = "+CRYPIV=00112233445566770011223344556677";	// Set Encryption IV
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_STR(command1);
			AT_PORT_SEND_END_AT();
			break;
		}
		case RAK_LORA_DEVICE_CAD_EN:
		{
			AT_PORT_SEND_BEGIN_AT();
			AT_PORT_SEND_CONST_STR("+CAD=1");
			AT_PORT_SEND_END_AT();
			break;
		}
	}
}

/**************************************************************************//**
 * Function name 	: lora_modem_init
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_init()
{
	unsigned char res=0;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
		HAL_GPIO_WritePin(GPIOC, LORA_RST_Pin, GPIO_PIN_RESET);
		osDelay(200);
		HAL_GPIO_WritePin(GPIOC, LORA_RST_Pin, GPIO_PIN_SET);

		osDelay(1000);
		lora_AT_check=2;
		lora_AT_ok_check=2;
		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(500);

		if((lora_AT_ok_check == 1)&&(lora_AT_check == 1))
		{
			lora_AT_check = 0;
			Lora_AT_Command = RAK_LORA_DEVICE_ATE;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(500);
		}

		lora_AT_check=2;
		lora_AT_ok_check=2;
		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(1000);

		if((lora_AT_ok_check == 1)&&(lora_AT_check == 2))
		{
			Lora_AT_Command = RAK_LORA_DEVICE_SN_GET;
			Lora_initiate_cmd(Lora_AT_Command,0);
			osDelay(100);
			lora_set_P2P_mode();
			loraChannel = Get_Lora_Channel();
		}

		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_send_msg
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_send_msg(const char *msg)
{
	unsigned char res=0, waitCount;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
		LoRa_LED_TX_Indicate();				// Indicate TX on LoRa LED
		Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
		Lora_initiate_cmd(Lora_AT_Command, 0);
		osDelay(150);
		Lora_AT_Command = RAK_LORA_DEVICE_PSEND;
		Lora_initiate_cmd(Lora_AT_Command, msg);
		flagLoRA_txDone = 0;
		waitCount = 0;
		do
		{
			osDelay(50);
			waitCount++;
		}while(!flagLoRA_txDone && waitCount < 40);
		res = flagLoRA_txDone;

		Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
		Lora_initiate_cmd(Lora_AT_Command, "65534");
		osDelay(150);
		Lora_AT_Command = RAK_LORA_DEVICE_AT;
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}

/**************************************************************************//**
 * Function name 	: lora_modem_send_cont
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
unsigned char lora_modem_send_cont(const char *msg)
{
	unsigned char res=0, waitCount;

	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
		Lora_AT_Command = RAK_LORA_DEVICE_PSEND;
		Lora_initiate_cmd(Lora_AT_Command, msg);
		flagLoRA_txDone = 0;
		waitCount = 0;
		do
		{
			osDelay(50);
			waitCount++;
		}while(!flagLoRA_txDone && waitCount < 40);
		res = flagLoRA_txDone;

		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}

	return res;
}
/**************************************************************************//**
 * Function name 	: ack_timer_callback
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ack_timer_callback(TimerHandle_t xTimer) {
    if(!ack_received)
    {
        if(retry_count < EPROM_General.LoraRetryCount)
        {
            retry_count++;
            if(LoraOTA.ota_state < LORA_OTA_BEGIN)	// OTA not running
            	flagLORAPubLogData = 1;  // Retry
        }
        else
        {
        	retry_count = 0;  	// Reset retry count
        	NoConnNodes = 0;	// node NOT connected with GW
        }
    }
    WriteLog(1, "ACK Timer Expired", 1);
}

/**************************************************************************//**
 * Function name 	: init_ack_retry_timer
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void init_ack_retry_timer(void) {
    ackTimer = xTimerCreate(
        "AckTimer",
        pdMS_TO_TICKS(EPROM_General.LoraRetryDelay*1000),
        pdFALSE,          // One-shot timer
        NULL,
        ack_timer_callback
    );

    if (ackTimer == NULL) {
        WriteLog(1, "Failed to create ACK timer!\n", 1);
    }
}

/**************************************************************************//**
 * Function name 	: ack_timer_callback
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ota_timer_callback(TimerHandle_t xTimer) {
	if(EPROM_General.Modem_EC200_presence)
	{
		if(LoraOTA.ota_state == LORA_OTA_BEGIN &&
				LoraOTA.index > 3)	// 3 times sent & NO ACK
		{
			LoraOTA.ota_state = 0;	// Stop OTA
			LoraOTA.index = 0;
		}
		else
			LoraOTA.ready = 1;
	}
	else	// Node didn't rxd OTA msg after OTA Start
	{
		if(LoraOTA.totalChunks && (LoraOTA.curentChunk == LoraOTA.totalChunks))
		{
			LoraOTA.ota_state = LORA_OTA_VERIFY;	// All chunck rxd
			LoraOTA.ready = 1;
		}
		else
		{
			LoraOTA.ota_state = 0;	// Come out of OTA
			NoConnNodes = 0;	// Lost connection with GW
		}
	}
    WriteLog(1, "OTA Timer Expired\r\n", 1);
}

/**************************************************************************//**
 * Function name 	: init_ack_retry_timer
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void init_ota_timer(void) {
	uint32_t ota_timeout = EPROM_General.LOta_MsgInterval*1000;

	if(!EPROM_General.Modem_EC200_presence)	// Node
		ota_timeout *= 60;

    otaTimer = xTimerCreate(
        "otaTimer",
        pdMS_TO_TICKS(ota_timeout),
        pdFALSE,          // One-shot timer
        NULL,
        ota_timer_callback
    );

    if (otaTimer == NULL) {
        WriteLog(1, "Failed to create OTA timer!\n", 1);
    }
}
/**************************************************************************//**
 * Function name 	: StartLoraTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void StartLoraTask(void const * argument)
{
	osDelay(5000);
	unsigned int networkCheckTime = 60;
	char LED_Lora_blinking, LED_counter=0;

	// Initialize LoRa LED control
	LoRa_LED_Init();
	HAL_UART_Receive_IT(&huart3, &Lora_RX_Buff[0], sizeof(Lora_RX_Buff));
	lora_PortBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(lora_PortBlockSemaphore);

	lora_SequenceBlockSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(lora_SequenceBlockSemaphore);

	set_pulse_do_polarity();
	osDelay(10);

	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	if(Pro_Application_flag == 1)
	{
		lcd_clear();
		lcd_set_cursor(1,0);
		lcd_display_string("   PRODUCTION   ");
		lcd_set_cursor(2,0);
		lcd_display_string("      MODE      ");
	}

	float ADC_VAL_accumalate[MAX_AI_CHANNEL];
	unsigned char sample_count = 0;
	for(int i=0;i<MAX_AI_CHANNEL;i++)
	{
		ADC_VAL_accumalate[i] = 0;
	}

	lora_modem_init();

	init_ack_retry_timer();
	xTimerStop(ackTimer, 0);
	init_ota_timer();
	xTimerStop(otaTimer, 0);

	if(Pro_Application_flag)
	{
		while(ProductionModeIDFrameReceived == 0)
		{
			osDelay(100);
		}
	}

	for(;;)
	{
		if((networkCheckTime++ > 3000)||(LoRa_Modem.lora_restart_request == 1))	// check approx 5 min
		{
			networkCheckTime = 0;
			LoRa_Modem.lora_network_Mode = 1;	// To make sure chip is in P2P mode
			lora_modem_network_mode_check();
			osDelay(100);
			if(LoRa_Modem.lora_network_Mode != 0 || LoRa_Modem.lora_restart_request == 1)
			{
				if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
				{
					LoRa_Modem.lora_restart_request = 0;
					HAL_GPIO_WritePin(GPIOC, LORA_RST_Pin, GPIO_PIN_RESET);
					osDelay(100);
					HAL_GPIO_WritePin(GPIOC, LORA_RST_Pin, GPIO_PIN_SET);
					osDelay(100);
					lora_set_P2P_mode();
					loraChannel = Get_Lora_Channel();
					Lora_AT_Command = RAK_LORA_DEVICE_AT;
					xSemaphoreGive(lora_SequenceBlockSemaphore);
				}
			}
		}

		if(ReceivedDataOfLoRaClient == 1)
		{
			if(!EPROM_General.Modem_EC200_presence)	// Node
			{
				LoRa_LED_RX_Indicate();    		// Indicate RX on LoRa LED
			}
			ReceivedDataOfLoRaClient = 0;
			ProcessModbusSlave(COM_LORA);

			if(ModbusH[COM_LORA].u8RxBuffer[1]==03)
		    {
				flagLORAPubLogData=1;
				flagExLog = 1;
		    }
			else	// Send Dowlink ACK - MAC + msg ID + msg type
			{
				strcpy(ack_payload[ack_wr_index], LoRa_Modem.lora_serial_no);
				HexToAscii(&lora_msgID, &ack_payload[ack_wr_index][MAC_LEN], sizeof(lora_msgID));
				ack_payload[ack_wr_index][MAC_LEN+MSG_ID_LEN] = '0';
				ack_payload[ack_wr_index][MAC_LEN+MSG_ID_LEN+1] = '0' + LORA_DL_ACK;
				ack_payload[ack_wr_index][MAC_LEN+MSG_ID_LEN+2] = '\0';
				if(++ack_wr_index > 9)
					ack_wr_index = 0;
			}
		}

		if(ack_received && !EPROM_General.Modem_EC200_presence)
		{
			ack_received = 0;
			retry_count = 0;  // Reset retry count
			xTimerStop(ackTimer, 0);
			LoRa_LED_RX_Indicate();    		// Indicate RX on LoRa LED
		}
		if(flagLoraRx == 1)		// GW received log rate or event from node
		{
			flagLoraRx = 0;
			LoRa_LED_RX_Indicate();    		// Indicate RX on LoRa LED
		}

		if (flagLORAPubLogData == 1 && LoraOTA.ota_state < LORA_OTA_BEGIN)	// During OTA log data stopped
		{
		    flagLORAPubLogData = 0;
		    LoraPublish();

		    if (Pro_Application_flag == 1)	// Production mode
		    {
		    	lora_send_with_retry();
		    	pro_flagLoRA_txDone = 1;
		    }
		    else	// Run Mode
		    {
		        if (!EPROM_General.Modem_EC200_presence)	// Node
		        {
		            char msgID_str[3], msgType_str[3];
		            sprintf(msgID_str,  "%02X", lora_msgID++);
		            sprintf(msgType_str, "%02X", LORA_LOGRATE);
		            strcat(lora_tx_buf, msgID_str);
		            strcat(lora_tx_buf, msgType_str);

		            // ACK management
		            ack_received = 0;
		            xTimerStart(ackTimer, 0);
		            lora_send_with_retry();
		        }
		        else	// Gateway: send via cellular
		        {
		            strcpy(uplink_data[wr_index], lora_tx_buf);
		            uplink_RSSI[wr_index] = Modem_gsm_rssi;
		            uplink_SNR[wr_index]  = 0;
		            flagExLog = 0;
		            flagMqttPubLogData = 1;
		            xTimerStart(sendTimer, 0);

		            if (++wr_index >= UPLINK_BUF_LEN)
		                wr_index = 0;
		            if (wr_index == rd_index)
		                flagSendData = 1;
		        }
		    }

		    // LED feedback
		    LED_Lora_blinking = 1;
		    LED_counter = 4;
		}
		// If ack payload available
		if(flagSendACK ||
				(LoraSendACKtime && (HAL_GetTick() - LoraSendACKtime > PENDING_ACK_TIMEOUT_MS)))
		{
			LoRa_LED_TX_Indicate();				// Indicate TX on LoRa LED
			Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
			Lora_initiate_cmd(Lora_AT_Command, 0);
			osDelay(200);
			do {
				if(ack_payload[ack_rd_index][0] != '\0')
				{
					osDelay(100);
					if(!lora_modem_send_cont(ack_payload[ack_rd_index]))
					{
						osDelay(100);
						if(!lora_modem_send_cont(ack_payload[ack_rd_index]))
						{
							LoRa_Modem.lora_restart_request = 1;	// reset chip if still not able to send
							break;
						}
						else
							LoRa_Modem.lora_restart_request = 0;
					}
				}
				ack_payload[ack_rd_index][0] = '\0';
				if(++ack_rd_index > 29)
					ack_rd_index = 0;
			}while(ack_rd_index != ack_wr_index);
			if(!LoRa_Modem.lora_restart_request)
				flagSendACK = 0;
			LoraSendACKtime = 0;

			Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
			Lora_initiate_cmd(Lora_AT_Command, "65534");
			osDelay(50);
		}
		if(LoraOTA.ready)
		{
			LoraOTA.ready = 0;
			xTimerStart(otaTimer, 0);	// Start or Reset timer

			switch(LoraOTA.ota_state)
			{
				case LORA_OTA_BEGIN:
					if(EPROM_General.Modem_EC200_presence)	// GW
					{
						LoraOTA.ota_type = OTA_store_Data.FileType;	//OTA_DELTA_H7;
						LoraOTA.sessionId = OTA_store_Data.sessionId;
						LoraOTA.forVer = OTA_store_Data.forVer;
						LoraOTA.fileCrc = OTA_store_Data.TgtHEX_Crc;
						LoraOTA.fileSize = OTA_store_Data.TgtFile_Size;
						uint32_t to_add = (256 - (OTA_store_Data.DeltaSize & 0xFF)) & 0xFF;	// roundup to multiple of 256
						LoraOTA.totalChunks = (OTA_store_Data.DeltaSize+to_add)/LORA_OTA_CHUNK_SIZE;
						LoraOTA.curentChunk = 0;
						LoraOTA.chunkSize = LORA_OTA_CHUNK_SIZE;
						BuildStartPayload();
						LoraOTA.index++; // Send 3 times
						memset(nodesCon, 0, sizeof(nodesCon));	// reset mac table
						NoConnNodes = maxConnNode = 0;
					}
					else	// Node send SN+ID+ACK
					{
						osDelay(1000);	// Let GW come to Rx mode
						strcpy(lora_tx_buf,LoRa_Modem.lora_serial_no);
						HexToAscii(&LoraOTA.sessionId, &lora_tx_buf[MAC_LEN], sizeof(LoraOTA.sessionId));
						char temp= LORA_OTA_ACK_MSG;
						HexToAscii(&temp, &lora_tx_buf[18], sizeof(temp));
						lora_tx_buf[20] = '\0';
					}
					lora_send_with_retry();
					break;
				case LORA_OTA_PUSHING:
					if(EPROM_General.Modem_EC200_presence)
					{
						BuildPushChunk();
						lora_send_with_retry();
						// If NACK received then transition again to this state
						if(++LoraOTA.curentChunk >= LoraOTA.totalChunks)
						{
							LoraOTA.ota_state = LORA_OTA_VERIFY;
							LoraOTA.index = 0;
						}
					}
					break;
				case LORA_OTA_DOWNLOADING:
					if(LoraOTA.ota_type == OTA_DELTA_H7)	// Node
					{
						if(ack_payload[0][0] != '\0')
						{
							osDelay(1000);	// Let GW come to Rx mode
							lora_modem_send_msg(ack_payload[0]);
							osDelay(1000);	// send again
							lora_modem_send_msg(ack_payload[0]);
							ack_payload[0][0] = '\0';
						}

						if(LoraOTA.curentChunk%64 == 0 || LoraOTA.curentChunk == 1)	// 64*64 = 4K
						{
							unsigned int address = LoraOTA.curentChunk*LoraOTA.chunkSize;

							if(LoraOTA.curentChunk == 1)
								address = DELTA_HEX_3100_START_ADD;
							else
								address += DELTA_HEX_3100_START_ADD;

							Erase_RECsector(address, 1);
							sprintf(print, "Erasing 3100 Delta sector address: %0x \r\n", address);
							WriteLog(1, print, 1);
							/*	For 1100
							{
								if(LoraOTA.curentChunk == 1)
									address = DELTA_HEX_1100_START_ADD;
								else
									address += DELTA_HEX_1100_START_ADD;
								Erase_RECsector(address, 1);
								sprintf(print, "Erasing 1100 Delta sector address: %0x \r\n", address);
							}*/
						}

						if(LoraOTA.index >= 256)
						{
							unsigned int address = (LoraOTA.curentChunk-4)*LoraOTA.chunkSize;
							if(LoraOTA.ota_type == OTA_DELTA_H7)
							{
								address += DELTA_HEX_3100_START_ADD;
								sprintf(print, "3100 Delta Write address: %0x \r\n", address);
							}
							/* For 1100
							{
								address += DELTA_HEX_1100_START_ADD;
								sprintf(print, "1100 Delta Write address: %0x \r\n", address);
							}*/
							WriteLog(1, print, 1);

							W25Q_Write_continous((uint8_t*)LoraOTA.bin, 256, address);
							LoraOTA.index -= 256;
						}
					}
					break;
				case LORA_OTA_VERIFY:
					uint8_t createFullFile = 0;

					if(EPROM_General.Modem_EC200_presence)
					{
						static uint8_t noOTAnodes = 0;

						if(!LoraOTA.index)
						{
							noOTAnodes = NoConnNodes;
							memset(nodesCon, 0, sizeof(nodesCon));
							NoConnNodes = 0;
							maxConnNode = 0;
						}

						if((NoConnNodes < noOTAnodes) && LoraOTA.index < 2)	// Let all nodes send ACK
						{
							BuildVerifyChunk();
							lora_send_with_retry();
							LoraOTA.index++;
						}
						else
						{
							xTimerStop(otaTimer, 0);
							OTA_store_Data.OTApushTime = 0;	//Don't repeat Delta OTA
							ExtFlash_update_OTA_Data();
							osDelay(100);
							if(LoraOTA.ota_type == OTA_DELTA_H7)
								createFullFile = 1;
							else
								reboot_device_func();
						}
					}
					else	// Node
					{
						osDelay(1000);	// Let GW come to Rx mode
						strcpy(lora_tx_buf,LoRa_Modem.lora_serial_no);
						HexToAscii(&LoraOTA.sessionId, &lora_tx_buf[MAC_LEN], sizeof(LoraOTA.sessionId));
						char temp= LORA_OTA_ACK_MSG;
						HexToAscii(&temp, &lora_tx_buf[18], sizeof(temp));
						lora_tx_buf[20] = '\0';
						lora_send_with_retry();
						if(LoraOTA.ota_type == OTA_DELTA_H7)
						{
							//validate CRC of delta, GW already validated, just required at Nodes
							if(!verify_OTA_CRC(OTA_store_Data.DeltaCRC, OTA_store_Data.DeltaSize, DELTA_HEX_3100_START_ADD))
								createFullFile = 1;
							else
							{
								WriteLog(1, "Delta CRC mismatch. Rebooting....\r\n", 1);
								osDelay(100);
								reboot_device_func();
							}
						}
					}

					if(createFullFile)	// if 3100 delta hex then
					{
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
						sprintf(print, "Running version %d\r\n", curVer);
						WriteLog(1, print, 1);
						if(LoraOTA.forVer == curVer && 		//delta is for current ver
								OTA_store_Data.File_Size)	// full file available
						{
							source.size = HEX_FILE_START_ADDRESS + OTA_store_Data.File_Size;
							patch.size = DELTA_HEX_3100_START_ADD + OTA_store_Data.DeltaSize;
							target.size = TGT_HEX_FILE_START_ADDRESS + LoraOTA.fileSize;

							if(!janpatch(ctx, &source, &patch, &target))
							{
								WriteLog(1, "Full OTA file successfully created from Delta\r\n", 1);
								if(!verify_OTA_CRC(LoraOTA.fileCrc, LoraOTA.fileSize, TGT_HEX_FILE_START_ADDRESS))
								{
									WriteLog(1, "Target File verified, now copying\r\n", 1);
									if(copyTargetToSource(LoraOTA.fileSize))
									{
										if(!verify_OTA_CRC(LoraOTA.fileCrc, LoraOTA.fileSize, HEX_FILE_START_ADDRESS))
										{
											OTA_store_Data.File_Size = LoraOTA.fileSize;
											OTA_store_Data.HEX_Crc = LoraOTA.fileCrc;
											OTA_store_Data.OTA_State = 2;
											ExtFlash_update_OTA_Data();
											WriteLog(1, "Full OTA File verified. Rebooting....\r\n", 1);
										}
										else
											WriteLog(1, "CRC mismatch after copy. Rebooting....\r\n", 1);
									}
									else
										WriteLog(1, "Copy target to source FAILED. Rebooting....\r\n", 1);
								}
								else
									WriteLog(1, "Target CRC failed. Rebooting....\r\n", 1);
							}
							else
								WriteLog(1, "ERROR: Full OTA File NOT created\r\n", 1);
						}
						else
							WriteLog(1, "OTA not for current Ver, or Full file not available skipping\r\n", 1);
						osDelay(100);
						reboot_device_func();
					}
					break;
				default:
					WriteLog(1, "Wrong Delta OTA state\r\n", 1);
					LoraOTA.ota_state = 0;	// Come out of OTA
					xTimerStop(otaTimer, 0);
					break;
			}
		}
		else if(OTA_store_Data.OTApushTime && !LoraOTA.ota_state	// Push time available & not in OTA
				&& (gTime.Minutes % 2) && (gTime.Seconds == 0))		// check every 2 min
		{
			if(EPROM_General.Modem_EC200_presence && isTimeForDeltaOTA())	// Time to do Delta OTA
			{
				LoraOTA.ready = 1;
				LoraOTA.ota_state = LORA_OTA_BEGIN;
				WriteLog(1, "Time to perform Delta OTA", 1);
			}
		}

		if(flagSyncNodeTime)
		{
			sendTimeSyncCmd();
			flagSyncNodeTime = 0;
		}

		if(LED_counter > 0)
		{
			LED_counter--;
			if(LED_Lora_blinking == 1)
			{
				LED_Lora_blinking = 0;
			}
			else
			{
				LED_Lora_blinking = 1;
			}
		}
		CheckTimeouts();

		gFinalAnaValF[LORA_PHY_STATUS_gFinalAnaValF]= lora_AT_ok_check;
		gFinalAnaValF[LORA_RSSI_gFinalAnaValF]= LoRa_Modem.lora_RSSI;

		gFinalAnaValF[LORA_ACTIVE_REGION_gFinalAnaValF]= LoRa_Modem.lora_network_Mode;

		count_DO = 0;
		flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter++;

		if((proTestRequest) & (0x1<<PRODUCTION_TEST_BIT_WATCHDOG))
		{

		}
		if(Pro_Application_flag == 1)
		{
			pro_checkDIDOState();
			if(gTime.Seconds%15 == 0)
				flagLORAPubLogData = 1;
        	osDelay(100);
		}

		ScanDI();
		scanKey();

		if(flag_flashUpdateEPROM_General == 1)
		{
			flag_flashUpdateEPROM_General_WaitCounter--;
			if(flag_flashUpdateEPROM_General_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_General = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_General();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_General = 1;
					flag_flashUpdateEPROM_General_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Frequent == 1)
		{
			flag_flashUpdateEPROM_Frequent_WaitCounter--;
			if(flag_flashUpdateEPROM_Frequent_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Frequent = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Frequent();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Frequent = 1;
					flag_flashUpdateEPROM_Frequent_WaitCounter = 1;
				}
			}
		}

		if(flag_flashUpdateEPROM_AI_Calibration == 1)
		{
			flag_flashUpdateEPROM_AI_Calibration_WaitCounter--;
			if(flag_flashUpdateEPROM_AI_Calibration_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_AI_Calibration = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_AI_Calibration();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_AI_Calibration = 1;
					flag_flashUpdateEPROM_AI_Calibration_WaitCounter = 1;
				}
			}

		}

		if(flag_flashUpdateEPROM_Schedule == 1)
		{
			flag_flashUpdateEPROM_Schedule_WaitCounter--;
			if(flag_flashUpdateEPROM_Schedule_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Schedule = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Schedule();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Schedule = 1;
					flag_flashUpdateEPROM_Schedule_WaitCounter = 1;
				}
			}
		}

		if(flag_flashUpdateEPROM_Modbus_Quary_Detail == 1)
		{
			flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter--;
			if(flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter == 0)
			{
				flag_flashUpdateEPROM_Modbus_Quary_Detail = 0;
				if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
					ExtFlash_update_EPROM_Modbus_Quary_Detail();
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
				else
				{
					flag_flashUpdateEPROM_Modbus_Quary_Detail = 1;
					flag_flashUpdateEPROM_Modbus_Quary_Detail_WaitCounter = 1;
				}
			}
		}

		if(flag_flashSaveRecipe == 1)
		{
			flag_flashSaveRecipe_WaitCounter--;
			if(flag_flashSaveRecipe_WaitCounter==0)
			{
				flag_flashSaveRecipe = 0;
		        if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE )
				{
		        	WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
					xSemaphoreGive(sendExternalFlashSemaphore);
				}
		        else
		        {
		        	flag_flashSaveRecipe = 1;
		        	flag_flashSaveRecipe_WaitCounter = 1;
		        }
			}
		}

		if(flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter == 600)
		{
			flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 0;
			if(xSemaphoreTake(sendExternalFlashSemaphore, 1000) == pdTRUE)
			{
				for(int i=0;i<MAX_GENERAL_AI;i++)
				{
					EPROM_PCBPLC_General_Reg.General_Reg[i]=gFinalAnaValF[GENERAL_PURPOSE_AI_gFinalAnaValF+i];
				}
				ExtFlash_update_EPROM_PCBPLC_GENERAL_REG();

				EPROM_Frequent.DI1_Pulse = DI1_Pulse_Count;
				EPROM_Frequent.DI2_Pulse = DI2_Pulse_Count;
				ExtFlash_update_EPROM_Frequent();
				xSemaphoreGive(sendExternalFlashSemaphore);
			}
			else
			{
				flag_flashUpdateEPROM_PCBPLC_GENERAL_REG_WaitCounter = 299;
			}
		}

		count_ADC = 0;
		sample_count++;
		print_time();

        PT_GPIO_Timed_Control();  // check timing

		for(int i=0;i<MAX_AI_CHANNEL;i++)
		{
			ADC1_calibration_CH(EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type,i);
			ADC_calculation(AO_VAL_float[i],i);
			ADC_VAL_accumalate[i] = (float)ADC_Temp[i]+(float)ADC_VAL_accumalate[i];
		}

		if(sample_count >= 1)
		{
			for(int i=0;i<MAX_AI_CHANNEL;i++)
			{
				AI_Final_value[i] = ADC_VAL_accumalate[i]/sample_count;
				ADC_VAL_accumalate[i] = 0;
			}
			sample_count = 0;
		}
		LoRa_LED_Process();
		osDelay(55);
	}
}

void Lora_start()
{
	osThreadDef(LoraTask, StartLoraTask, osPriorityHigh, 0, 512*12); //512*10
	Lora_TaskHandle = osThreadCreate(osThread(LoraTask), NULL);
}

/**************************************************************************//**
 * Function name 	: MX_USART3_UART_Init
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: USART1 Initialization Function
 *****************************************************************************/

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;//EPROM_General.S_Comm.Rs232_1_Info.S_Baudrate;//9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  HAL_UART_ReceiverTimeout_Config(&huart3,10);
  HAL_UART_EnableReceiverTimeout(&huart3);
  /* USER CODE END USART3_Init 2 */

}
/**************************************************************************//**
 * Function name 	: AT_Serial_Send_fn
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

uint16_t lora_serial_send_fn(const void* data, size_t len)
{
    const uint8_t* d = data;

    HAL_UART_Transmit(&huart3,d,len,5000);

    return len;
}

void lora_send_with_retry(void)
{
    if (!lora_modem_send_msg(lora_tx_buf))
    {
    	osDelay(100);
        if (!lora_modem_send_msg(lora_tx_buf))   // retry
            LoRa_Modem.lora_restart_request = 1;
    }
    memset(lora_tx_buf, 0, sizeof(lora_tx_buf));
}

void lora_modem_network_mode_check(void)
{
	if(xSemaphoreTake(lora_SequenceBlockSemaphore, 1200000))
	{
		Lora_AT_Command = RAK_LORA_DEVICE_P2P_MODE_GET;
		Lora_initiate_cmd(Lora_AT_Command,0);
		osDelay(100);
		xSemaphoreGive(lora_SequenceBlockSemaphore);
	}
}

void lora_set_P2P_mode(void)
{
	Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
	Lora_initiate_cmd(Lora_AT_Command,0);
	osDelay(100);

	Lora_AT_Command = RAK_LORA_DEVICE_P2P_MODE_SET;
	Lora_initiate_cmd(Lora_AT_Command,0);
	osDelay(100);

	// 865985000 frequency, spreading factor 12, 125kHz bandwidth, coding rate 4/5, preamble length 8 and TX power of 22dBm:
	Lora_AT_Command = RAK_LORA_DEVICE_P2P_PARAM_SET;
	char lora_param[100];
	sprintf(lora_param, "%u:%u:%u:%u:%u:%u", EPROM_General.Lora_Frequency,
		EPROM_General.Lora_Spreading_Factor, EPROM_General.Lora_Bandwidth,
		EPROM_General.Lora_Code_Rate, EPROM_General.Lora_Preamble_Length,
		EPROM_General.Lora_TX_Power);
	Lora_initiate_cmd(Lora_AT_Command, lora_param);
	osDelay(100);

	Lora_AT_Command = RAK_LORA_DEVICE_CAD_EN;
	Lora_initiate_cmd(Lora_AT_Command,0);
	osDelay(100);
	Lora_AT_Command = RAK_LORA_DEVICE_PENCRYPT_KEY;
	Lora_initiate_cmd(Lora_AT_Command,"0");
	osDelay(100);
	Lora_AT_Command = RAK_LORA_DEVICE_PENCRYPT_EN;
	Lora_initiate_cmd(Lora_AT_Command,"1");
	osDelay(100);
	Lora_AT_Command = RAK_LORA_DEVICE_PRECV_SET;
	Lora_initiate_cmd(Lora_AT_Command,"65534");	// continuous RX mode
	osDelay(100);
}

// Function to determine LoRa channel number
uint8_t Get_Lora_Channel(void)
{
    uint8_t channel = 0;
    unsigned int Lora_Frequency = EPROM_General.Lora_Frequency/1000;
    switch (EPROM_General.Lora_Spreading_Factor)
    {
        case 9:
            if (Lora_Frequency == 865062)
            	channel = 1;
            else if (Lora_Frequency == 865402)
            	channel = 2;
            else if (Lora_Frequency == 865985)
            	channel = 3;
            break;

        case 8:
            if (Lora_Frequency == 865062)
            	channel = 4;
            else if (Lora_Frequency == 865402)
            	channel = 5;
            else if (Lora_Frequency == 865985)
            	channel = 6;
            break;

        case 7:
            if (Lora_Frequency == 865062)
            	channel = 7;
            else if (Lora_Frequency == 865402)
            	channel = 8;
            else if (Lora_Frequency == 865985)
            	channel = 9;
            break;

        default:
            channel = 0; // Invalid SF
            break;
    }
    return channel;
}
void PT_GPIO_Timed_Control(void)
{
    uint32_t now = HAL_GetTick();  // current time in ms since boot

    if (outputState == 0)
    {
        // Currently OFF
        if ((now - lastToggleTime) >= OFF_DURATION_MS)
        {
            // 14 minutes passed → turn ON
            HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_SET);
            outputState = 1;
            lastToggleTime = now;
//            HAL_UART_Transmit(&huart1,(uint8_t *)"onPT\n", sizeof("onPT\n"), 1000);
        }
    }
    else if(outputState == 1)
    {
    	if ((now - lastToggleTime) >= ON_DURATION_MS/2)
		{
			// 0.5 minute passed → send event
			outputState = 2;
			flagLORAPubLogData = 1;
			WriteLog(1, "Sending PT EVENT\r\n", 1);
		}
    }
    else
    {
        // Currently ON
        if ((now - lastToggleTime) >= ON_DURATION_MS)
        {
            // 1 minute passed → turn OFF
            HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_RESET);
            outputState = 0;
            lastToggleTime = now;
//            HAL_UART_Transmit(&huart1,(uint8_t *)"offPT\n", sizeof("offPT\n"), 1000);
        }
    }
}

/**
 * @brief Initialize LoRa LED control
 */
void LoRa_LED_Init(void)
{
    current_led_pattern = LORA_LED_OFF;
    lora_led_counter = 0;
    lora_led_state = 0;
    delay_counter = 0;
    HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
}

/**
 * @brief Set LED pattern for LoRa operations
 * @param pattern: LED pattern to set
 */
void LoRa_LED_Set_Pattern(lora_led_pattern_t pattern)
{
    current_led_pattern = pattern;
    delay_counter = 0;

    switch (pattern) {
        case LORA_LED_TX_FAST:
			lora_led_counter = 2;   // Single blink (on/off = 2 cycles)
  			led_blink_delay = 2;   // ~1 second ON (2 * 100ms = 200ms)

            break;

        case LORA_LED_RX_SLOW:
			 lora_led_counter = 8;   // 4 fast blinks (on/off = 8 cycles)
   			 led_blink_delay = 2;    // Fast blink (2 * 100ms = 200ms per state)
            break;

        case LORA_LED_IDLE:
            lora_led_counter = 2;   // Single blink (on/off = 2 cycles)
            led_blink_delay = 3;    // Medium blink (300ms cycles)
            break;

        case LORA_LED_ERROR:
            lora_led_counter = 20;  // 10 very fast blinks for error
            led_blink_delay = 0;    // Very fast blink (immediate)
            break;

        case LORA_LED_OFF:
        default:
            lora_led_counter = 0;
            HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
            break;
    }

    lora_led_state = 0;
}

/**
 * @brief Process LED blinking - call this in main loop every 100ms
 */
void LoRa_LED_Process(void)
{
    if (current_led_pattern == LORA_LED_OFF || lora_led_counter == 0) {
        return;
    }

    // Handle delay for different blink speeds
    if (led_blink_delay > 0) {
        if (delay_counter < led_blink_delay) {
            delay_counter++;
            return;
        }
        delay_counter = 0;
    }

    // Toggle LED state
    if (lora_led_state == 0) {
        HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
        lora_led_state = 1;
    } else {
        HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
        lora_led_state = 0;
    }

    lora_led_counter--;

    // When pattern is complete, turn off LED
    if (lora_led_counter == 0) {
        current_led_pattern = LORA_LED_OFF;
        HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief Indicate LoRa TX operation (fast blinks)
 */
void LoRa_LED_TX_Indicate(void)
{
    LoRa_LED_Set_Pattern(LORA_LED_TX_FAST);
}

/**
 * @brief Indicate LoRa RX operation (slow blinks)
 */
void LoRa_LED_RX_Indicate(void)
{
    LoRa_LED_Set_Pattern(LORA_LED_RX_SLOW);
}

/**
 * @brief Indicate LoRa error (very fast blinks)
 */
void LoRa_LED_Error_Indicate(void)
{
    LoRa_LED_Set_Pattern(LORA_LED_ERROR);
}

/**
 * @brief Turn off LoRa LED
 */
void LoRa_LED_Off(void)
{
    LoRa_LED_Set_Pattern(LORA_LED_OFF);
}

void BuildStartPayload(void)
{
    uint8_t index = 4;

    if(LoraOTA.ota_type == OTA_DELTA_H7)
	{
		strcpy(lora_tx_buf,"DE31");
	}
	else if(LoraOTA.ota_type == OTA_DELTA_G0)
	{
		strcpy(lora_tx_buf,"DE11");
	}

	// magic suffix AC 1F 09 05
	strcat(&lora_tx_buf[index], MAGIC_TAIL);
	index += 8;
    // Total chunks → 2 bytes
    HexToAscii(&LoraOTA.totalChunks, &lora_tx_buf[index], sizeof(LoraOTA.totalChunks));
    index += 4;
    // Session ID → 1 byte
    HexToAscii(&LoraOTA.sessionId, &lora_tx_buf[index], sizeof(LoraOTA.sessionId));
    index += 2;
    // For Release version → 2 byte
    HexToAscii(&LoraOTA.forVer, &lora_tx_buf[index], sizeof(LoraOTA.forVer));
    index += 4;
    // 1byte CRC
    HexToAscii(&LoraOTA.fileCrc, &lora_tx_buf[index], sizeof(LoraOTA.fileCrc));
    index += 2;
    // 4-byte target file size
    HexToAscii(&LoraOTA.fileSize, &lora_tx_buf[index], sizeof(LoraOTA.fileSize));
    index += 8;
    // 2-bytes msg CRC
	LoraOTA.chunkCrc = calcCRC((uint8_t *)lora_tx_buf, index);
	HexToAscii(&LoraOTA.chunkCrc, &lora_tx_buf[index], sizeof(LoraOTA.chunkCrc));
	index += 4;
    // 1-byte OTA signature
    lora_tx_buf[index++] = '0';
    lora_tx_buf[index++] = LORA_OTA_START_MSG + '0';
    lora_tx_buf[index] = '\0';
}

void BuildPushChunk(void)
{
	uint16_t index = 2;
	// session ID → 1 byte
	HexToAscii(&LoraOTA.sessionId, lora_tx_buf, sizeof(LoraOTA.sessionId));
	// magic suffix AC 1F 09
	strcat(&lora_tx_buf[index], MAGIC_TAIL);
	index += 6;
	// Current chunk → 2 bytes
	HexToAscii(&LoraOTA.curentChunk, &lora_tx_buf[index], sizeof(LoraOTA.curentChunk));
	index += 4;
	// Chunk Size → 1 byte
	HexToAscii(&LoraOTA.chunkSize, &lora_tx_buf[index], sizeof(LoraOTA.chunkSize));
	index += 2;
	// Binary data → 64 byte
	if(LoraOTA.ota_type == OTA_DELTA_H7)
	{
		uint32_t address = DELTA_HEX_3100_START_ADD + LoraOTA.curentChunk*LoraOTA.chunkSize;
		MX25L_ReadRaw(LoraOTA.bin, LoraOTA.chunkSize, address);
	}
	else
	{
		uint32_t address = DELTA_HEX_1100_START_ADD + LoraOTA.curentChunk*LoraOTA.chunkSize;
		MX25L_ReadRaw(LoraOTA.bin, LoraOTA.chunkSize, address);
	}
	HexToAscii(LoraOTA.bin, &lora_tx_buf[index], LoraOTA.chunkSize);
	index += 2*LoraOTA.chunkSize;

	// 2-byte CRC
	LoraOTA.chunkCrc = calcCRC((uint8_t *)lora_tx_buf, index);
	HexToAscii(&LoraOTA.chunkCrc, &lora_tx_buf[index], sizeof(LoraOTA.chunkCrc));
	index += 4;
	lora_tx_buf[index++] = '0';
	lora_tx_buf[index++] = LORA_OTA_CHNK_MSG + '0';
	lora_tx_buf[index] = '\0';
}

void BuildNackChunk(void)	// MACID SID CC NAK
{
	strcpy(ack_payload[0], LoRa_Modem.lora_serial_no);
	HexToAscii(&LoraOTA.sessionId, &ack_payload[0][MAC_LEN], sizeof(LoraOTA.sessionId));
	HexToAscii(&LoraOTA.curentChunk, &ack_payload[0][MAC_LEN+MSG_ID_LEN], sizeof(LoraOTA.curentChunk));
	uint8_t index = MAC_LEN+MSG_ID_LEN + 2*sizeof(LoraOTA.curentChunk);
	ack_payload[0][index++] = '0';
	ack_payload[0][index++] = LORA_OTA_NACK_MSG + '0';
	ack_payload[0][index] = '\0';
}

void BuildVerifyChunk(void)
{
    uint8_t index = 0;

    // Session ID → 1 byte
    HexToAscii(&LoraOTA.sessionId, &lora_tx_buf[index], sizeof(LoraOTA.sessionId));
    index += 2;
	// magic suffix AC 1F 09 05
	strcat(&lora_tx_buf[index], MAGIC_TAIL);
	index += 8;
    // Current chunks → 2 bytes
    HexToAscii(&LoraOTA.curentChunk, &lora_tx_buf[index], sizeof(LoraOTA.curentChunk));
    index += 4;
    // 1 byte Delta CRC
    HexToAscii(&OTA_store_Data.DeltaCRC, &lora_tx_buf[index], sizeof(OTA_store_Data.DeltaCRC));
    index += 2;
    // 1 byte Delta CRC
    HexToAscii(&OTA_store_Data.DeltaSize, &lora_tx_buf[index], sizeof(OTA_store_Data.DeltaSize));
    index += 8;
    // 2-bytes msg CRC
	LoraOTA.chunkCrc = calcCRC((uint8_t *)lora_tx_buf, index);
	HexToAscii(&LoraOTA.chunkCrc, &lora_tx_buf[index], sizeof(LoraOTA.chunkCrc));
	index += 4;
	// 1-byte OTA signature
    char temp = LORA_OTA_VERIFY_MSG;
    HexToAscii(&temp, &lora_tx_buf[index], sizeof(temp));
    index += 2;
    lora_tx_buf[index] = '\0';
}

void BuildSendDlPayload(char *mac, uint8_t msg_id, char *payload)
{
	char msg_str[150];//basavaraj_13_03_26
	char msgID_str[3];  // 2 chars + null terminator
	// cmd string + mac + msg ID + msg type
	sprintf(msgID_str, "%02X", msg_id);
	strncpy(msg_str, payload, sizeof(msg_str)-MAC_LEN-MSG_ID_LEN-MSG_TYPE_LEN);
	strcat(msg_str, mac);
	strcat(msg_str,msgID_str);
	uint8_t idx = strlen(msg_str);
	msg_str[idx++] = '0';
	msg_str[idx++] = '0' + LORA_CMD;
	msg_str[idx] = '\0';

	if(!lora_modem_send_msg(msg_str))
	{
		osDelay(100);
		if(!lora_modem_send_msg(msg_str))	// retry if can't send
			LoRa_Modem.lora_restart_request = 1;	// reset chip if still not able to send
	}
}

void CheckTimeouts(void)
{
	uint8_t idx;
	uint32_t currTick = HAL_GetTick();

    for (idx = 0; idx < DL_TBL_SIZE; idx++)
    {
        if (dlTable[idx].used && !dlTable[idx].ackReceived)
        {
            if((currTick - dlTable[idx].timeoutTick) > DL_TIMEOUT_MS)
            {
				if (dlTable[idx].retries < EPROM_General.LoraRetryCount)
				{
					BuildSendDlPayload(dlTable[idx].mac, dlTable[idx].msgId, dlTable[idx].payload);
					dlTable[idx].retries++;
					dlTable[idx].timeoutTick = HAL_GetTick();
				}
				else
				{
					dlTable[idx].used = 0;
					WriteLog(1, "Max retries reached for downlink", 1);
				}
            }
        }
    }

    if(!LoraOTA.ota_state)	// No need to check during OTA
    {
		for (idx = 0; idx < MAX_NODES; idx++)	// During OTA log data stopped
		{
			if(nodesCon[idx].macId[0] != '\0')
			{
				if((currTick - nodesCon[idx].timeoutTick) > HOUR_MS) // no msg received since an hour
				{
					sprintf(print, "No msg from Node %s, since an hour", nodesCon[idx].macId);
					WriteLog(1, print, 1);
					nodesCon[idx].macId[0] = '\0';	// remove entry
					NoConnNodes--;
					sprintf(print, "Now, total connected nodes: %d\r\n", NoConnNodes);
					WriteLog(1, print, 1);
				}
			}
		}
    }
}

void sendTimeSyncCmd(void)
{
	float dateTime;
	char tempString[10];
	strcpy(lora_tx_buf,"01100066000C18");	// Modbus cmd for Nodes

	dateTime = swap_float(gDate.Date);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	dateTime = swap_float(gDate.Month);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	dateTime = swap_float(gDate.Year+2000);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	dateTime = swap_float(gTime.Hours);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	dateTime = swap_float(gTime.Minutes);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	dateTime = swap_float(gTime.Seconds);
	HexToAscii(&dateTime, tempString, sizeof(dateTime));
	strcat(lora_tx_buf, tempString);

	uint16_t cmdCRC = calcCRC(lora_tx_buf,strlen(lora_tx_buf));
	HexToAscii(&cmdCRC, tempString, sizeof(cmdCRC));
	strcat(lora_tx_buf, tempString);

	strcat(lora_tx_buf, MAGIC_TAIL);
	uint8_t len = strlen(lora_tx_buf);
	sprintf(&lora_tx_buf[len], "%02X", lora_msgID++);
	len += 2;
	lora_tx_buf[len++] = '0';
	lora_tx_buf[len++] = LORA_RTC_SYNC + '0';
	lora_tx_buf[len] = '\0';

	lora_send_with_retry();
}

void scanKey(void)
{
	uint8_t button_state = HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin);
	uint32_t current_time = HAL_GetTick();
	static uint32_t last_button_press = 0;
	static uint8_t prev_button_state = 1;

	// Detect button press (falling edge) with debouncing
	if (prev_button_state != button_state)
	{
			// Check if enough time has passed since last button press (debouncing)
		if ((current_time - last_button_press) > BUTTON_DEBOUNCE_MS)
		{
			if (button_state == GPIO_PIN_RESET)
			{
				Backlight_Button_Pressed();  // Turn on backlight
				last_button_press = current_time;
			}
		}
	}
	prev_button_state = button_state;
}

char isTimeForDeltaOTA()
{
    // Decode OTApushTime (Format: HHMM, e.g., 2230 for 10:30 PM)
    uint16_t pushHours = OTA_store_Data.OTApushTime / 100;
    uint16_t pushMinutes = OTA_store_Data.OTApushTime % 100;

    // Convert both to "Minutes since Midnight"
    uint16_t currentTotalMin = (gTime.Hours * 60) + gTime.Minutes;
    uint16_t pushTotalMin = (pushHours * 60) + pushMinutes;

    // Define the window (30 minutes)
    uint16_t endTotalMin = pushTotalMin + 30;

    // window within the same day
    if (endTotalMin < 1440) // 1440 = 24 * 60
    {
        return (currentTotalMin >= pushTotalMin && currentTotalMin <= endTotalMin);
    }
    // Window crosses midnight (e.g., Push at 23:50, ends 00:20)
    else
    {
        uint16_t rolloverEndMin = endTotalMin - 1440;
        // True if we are between push time and midnight OR between midnight and the end time
        return (currentTotalMin >= pushTotalMin || currentTotalMin <= rolloverEndMin);
    }
}
