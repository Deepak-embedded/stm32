/*
 * COM_PORT_RS232_1.c
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include <RxRingProcess.h>
#include "main.h"
#include "ATmodemTypes.h"

/**************************************************************************//**
 * maCRO
 *****************************************************************************/
#define EC200U		0
#define LORA		1

/* Receive character macros */
#define RECV_ADD(ch, type)                                                                                                   \
    do {                                                                                                               \
        if (recv_buff[type].len < (sizeof(recv_buff[type].data)) - 1) {                                                            \
            recv_buff[type].data[recv_buff[type].len++] = ch;                                                                      \
            recv_buff[type].data[recv_buff[type].len] = 0;                                                                         \
        }                                                                                                              \
    } while (0)

#define RECV_RESET(type)                                                                                                   \
    do {                                                                                                               \
        recv_buff[type].len = 0;                                                                                             \
        recv_buff[type].data[0] = 0;                                                                                         \
    } while (0)
//#define RECV_LEN()                  ((size_t)recv_buff[type].len)
//#define RECV_IDX(index)             recv_buff[type].data[index]

/**************************************************************************//**
 * Variable
 *****************************************************************************/
static lwgsm_recv_t recv_buff[2];
uint8_t IP_ADDRESS[4];
unsigned int count_EC200U_RxRingProcess = 0;
uint8_t EC200U_RX_Buff[1200];        	//EC200U buffer to fill from Rx interrupt
lwrb_t EC200U_RX_rb;  				//EC200U Ring buffer instance for RX data
uint8_t EC200U_RX_rb_data[1500]; 	//EC200U Ring buffer data array for RX DMA

ModemInfo_t ModemInfo;
lwgsmr_t Modem_PHY_Status = lwgsmERRNODEVICE;
lwgsmr_t Modem_PHY_Status_t = lwgsmERRNODEVICE;

lora_Modem_t LoRa_Modem;
unsigned char lora_tx_port;
char lora_tx_buf[200];
unsigned char lora_rx_port;
unsigned char lora_rx_buf[200];
unsigned char lora_rx_buf_ascii[206];
char uplink_data[UPLINK_BUF_LEN][200];
signed char uplink_RSSI[UPLINK_BUF_LEN];
char uplink_SNR[UPLINK_BUF_LEN];
uint8_t wr_index;
uint8_t ack_wr_index;
uint8_t maxConnNode;
unsigned char PDP_Context_ID = 1;
//unsigned char PDP_Context_APN[30] = "www";
char PDP_Context_APN[30] = "jionet";
//unsigned char PDP_Context_APN[30] = "airtelgprs";

int16_t Modem_gsm_rssi;
lwgsm_sim_state_t Modem_gsm_sim_state;
lwgsm_sim_state_t Modem_gsm_sim_state_t;

lwgsm_network_reg_status_t Modem_gsm_network_status;
lwgsm_network_reg_status_t Modem_gsm_network_status_t;

unsigned char Modem_gsm_network_GATT_Status=0;
unsigned char Modem_gsm_network_GATT_Status_t=0;
unsigned char Modem_gsm_network_GACT_Status=0;
unsigned char Modem_gsm_network_GACT_Status_t=0;

char lora_AT_check;
char lora_AT_ok_check;
lwgsm_ip_t modem_ip;

int16_t modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;

unsigned int rx_dataCurrentPosition=0;        // use for positioning for RX data of mqtt or UDU or TCP
unsigned char ReceivedDataOfMQTTClient=0;
char Flag_Reset_CFUN=0, Flag_Reset_QRESET=0,Flag_QPWRBACKOFF=0, flagSendACK;
uint32_t LoraSendACKtime;

unsigned int lora_rx_dataCurrentPosition=0;
unsigned char ReceivedDataOfLoRaClient=0;
osThreadId RxRingProcess_TaskHandle;

uint8_t flag_SDCARD_ok=0;
extern uint8_t retry_count;
extern uint8_t ack_received;
extern uint8_t flagLoRA_txDone;
extern uint8_t flagLoraRx;
extern char ack_payload[30][23];
extern uint8_t ack_rd_index;
extern LoraOta_t LoraOTA;
extern char tDebug[500];
extern dlEntry_t dlTable[DL_TBL_SIZE];
extern uint8_t lora_msgID;
extern TimerHandle_t ackTimer;
extern TimerHandle_t sendTimer;

extern void BuildNackChunk(void);
/**************************************************************************//**
 * Function name 	: RxRingProcess_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void RxRingProcess_start()
{
	osThreadDef(RxRingProcessTask, Start_RxRingProcessTask, osPriorityHigh, 0, 512*10); //512
	RxRingProcess_TaskHandle = osThreadCreate(osThread(RxRingProcessTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartEC200U_RxRingProcess_startTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

//QueueHandle_t modem_rx_queue_id;

void Start_RxRingProcessTask(void const * argument)
{

	osDelay(6000);
	uint32_t ulNotifiedValue;

	for(;;)
	{
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10000));
		if(ulNotifiedValue>0)  // use notification as binary semaphore
		{
			count_EC200U_RxRingProcess = 0;
			if(EPROM_General.Modem_EC200_presence)
				RxRingProcess_buffer(&EC200U_RX_rb, EC200U);
			RxRingProcess_buffer(&lora_rx_rb, LORA);
		}

	}
}

/**************************************************************************//**
 * Function name 	: dataSendToRxRingFiller
 * arguments		: 1) callFrom [0 : timeout]  [1 : half transfer]  [2: full transfer]
 * return 		 	:
 * Note				:
 *
 * \brief           Check for new data received with DMA
 *
 * User must select context to call this function from:
 * - Only interrupts (DMA HT, DMA TC, UART IDLE) with same preemption priority level
 * - Only thread context (outside interrupts)
 *
 * If called from both context-es, exclusive access protection must be implemented
 * This mode is not advised as it usually means architecture design problems
 *
 * When IDLE interrupt is not present, application must rely only on thread context,
 * by manually calling function as quickly as possible, to make sure
 * data are read from raw buffer and processed.
 *
 * Not doing reads fast enough may cause DMA to overflow unread received bytes,
 * hence application will lost useful data.
 *
 * Solutions to this are:
 * - Improve architecture design to achieve faster reads
 * - Increase raw buffer size and allow DMA to write more data before this function is called
 *****************************************************************************/

void dataSendToRxRingFiller(lwrb_t *rb, uint8_t *buffer, size_t length, uint16_t RxXferCount, unsigned char callFrom)
{
    static size_t prev_pos[2];
    size_t old_pos;
    size_t pos=0;

    if(callFrom > 2)	// called from Lora INT (3, 4 & 5)
    	old_pos = prev_pos[1];
    else				// called from EC200U INT (0, 1 & 2)
    	old_pos = prev_pos[0];

    /* Calculate current position in buffer and check for new data available */

    if(callFrom % 3 == 0)    	// timeout
    {
    	pos = length - RxXferCount;
    }
    else if(callFrom % 3 == 1)  // half transfer
    {
    	pos = length - length/2;
    }
    else if(callFrom % 3 == 2)	// Full Transfer
    {
    	pos = length;
    }

    if (pos != old_pos) /* Check change in received data */
    {
        if (pos > old_pos) /* Current position is over previous one */
        {
            /*
             * Processing is done in "linear" mode.
             *
             * Application processing is fast with single data block,
             * length is simply calculated by subtracting pointers
             *
             * [   0   ]
             * [   1   ] <- old_pos |------------------------------------|
             * [   2   ]            |                                    |
             * [   3   ]            | Single block (len = pos - old_pos) |
             * [   4   ]            |                                    |
             * [   5   ]            |------------------------------------|
             * [   6   ] <- pos
             * [   7   ]
             * [ N - 1 ]
             */
        	RxRingFiller(rb, &buffer[old_pos], pos - old_pos);
        }
        else
        {
            /*
             * Processing is done in "overflow" mode..
             *
             * Application must process data twice,
             * since there are 2 linear memory blocks to handle
             *
             * [   0   ]            |---------------------------------|
             * [   1   ]            | Second block (len = pos)        |
             * [   2   ]            |---------------------------------|
             * [   3   ] <- pos
             * [   4   ] <- old_pos |---------------------------------|
             * [   5   ]            |                                 |
             * [   6   ]            | First block (len = N - old_pos) |
             * [   7   ]            |                                 |
             * [ N - 1 ]            |---------------------------------|
             */
        	RxRingFiller(rb, &buffer[old_pos], length - old_pos);
            if (pos > 0)
            {
            	RxRingFiller(rb, &buffer[0], pos);
            }
        }
        if(callFrom == 0)    	// EC200U timeout
        {
        	prev_pos[0] = 0;	//old_pos = 0;
        }
        else if(callFrom == 3) 	// LoRA timeout
        {
        	prev_pos[1] = 0;	//old_pos = 0;
        }
        else if(callFrom > 2)
        {
        	prev_pos[1] = pos;	//old_pos = pos;      /* Save current position as old for next transfers */
        }
        else
        	prev_pos[0] = pos;
    }
}

/**************************************************************************//**
 * Function name 	: RxRingFiller
 * arguments		: 1) data: Data to process
 * 					: 2) len: Length in units of bytes
 * return 		 	:
 * Note				: Process received data over UART
 * 					: Data are written to RX ringbuffer for application processing at latter stage
 * 					:
 *****************************************************************************/

void RxRingFiller(lwrb_t *rb, uint8_t*  data, size_t len)
{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	lwrb_write(rb, data, len);  /* Write data to receive buffer */

//	vTaskNotifyGiveFromISR(RxRingProcess_TaskHandle, &xHigherPriorityTaskWoken);
//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );


}

/**************************************************************************//**
 * Function name 	: RxRingProcess_buffer
 * arguments		:
 * 					:
 * return 		 	:
 * Note				: Process data from input buffer
 * 					:
 * 					:
 *****************************************************************************/

void RxRingProcess_buffer(lwrb_t * rx_rb, uint8_t type)
{
    void* data;
    size_t len;

    do {
        /*
         * Get length of linear memory in buffer
         * we can process directly as memory
         */
        len = lwrb_get_linear_block_read_length(rx_rb);
        if (len > 0) {
            /*
             * Get memory address of first element
             * in linear block of data to process
             */
            data = lwrb_get_linear_block_read_address(rx_rb);

//#ifdef SVC_DEBUG
//            printf("%.*s", len, (char *)data);
//#endif
            /* Process actual received data */
            RxRingProcess(data, len, type);

            /*
             * Once data is processed, simply skip
             * the buffer memory and start over
             */
            lwrb_skip(rx_rb, len);
        }
    } while (len);
}

/**************************************************************************//**
 * Function name 	: RxRingProcess
 * arguments		: data: Pointer to data to process
 * 					: data_len: Length of data to process in units of bytes
 * return 		 	:
 * Note				: Process input data received from GSM device
 * 					:
 * 					:
 *****************************************************************************/
unsigned int count_1=0,count_2=0,count_3=0,count_4=0;;
lwgsmr_t RxRingProcess(const void* data, size_t data_len, uint8_t type)
{
    uint8_t ch;
    const uint8_t* d = data;
    size_t d_len = data_len;
    static uint8_t ch_prev1, ch_prev2,semiColon_count;

    static lwgsm_unicode_t unicode;

    while (d_len > 0)
    { 					/* Read entire set of characters from buffer */
        ch = *d;        /* Get next character */
        ++d;            /* Go to next character, must be here as it is used later on */
        --d_len;        /* Decrease remaining length, must be here as it is decreased later too */

        if(mqtt[ReceivedDataOfMQTTClient].mqtt_rx_state == MQTT_RX_STATE_INPROGRESS)
		{
        	if(Modem_gsm_network_GACT_Status==0)
        	{
        		osDelay(10);
        		rx_dataCurrentPosition = 0;
        		mqtt[ReceivedDataOfMQTTClient].mqtt_rx_state = MQTT_RX_STATE_FREE;
        	}
        	else if(rx_dataCurrentPosition <= mqtt[ReceivedDataOfMQTTClient].mqtt_rx_data_len)
			{
        		mqtt[ReceivedDataOfMQTTClient].mqtt_rx_data[rx_dataCurrentPosition++]=ch;
        		mqtt[ReceivedDataOfMQTTClient].mqtt_rx_data[rx_dataCurrentPosition] = 0;
			}
        	else
        	{
        		osDelay(10);
        		rx_dataCurrentPosition = 0;
        		mqtt[ReceivedDataOfMQTTClient].mqtt_rx_state = MQTT_RX_STATE_DATA_READY;
        	}
		}
        else
        {
            /*
             * We are in command mode where we have to process byte by byte
             * Simply check for ASCII and unicode format and process data accordingly
             */
            lwgsmr_t res = lwgsmERR;
            if (LWGSM_ISVALIDASCII(ch))
            { /* Manually check if valid ASCII character */
                res = lwgsmOK;
                unicode.t = 1;                             /* Manually set total to 1 */
                unicode.r = 0;                             /* Reset remaining bytes */
            }
            else if (ch >= 0x80)
            {                       /* Process only if more than ASCII can hold */
                res = lwgsmi_unicode_decode(&unicode, ch); /* Try to decode unicode format */
            }

            if (res == lwgsmERR)
            {
            	/* In case of an ERROR */
                unicode.r = 0;
            }
            if (res == lwgsmOK)
            {     /* Can we process the character(s) */
                if (unicode.t == 1)
                {
                	/* Totally 1 character? */
                    RECV_ADD(ch, type);     /* Any ASCII valid character */

                    if (!strncmp((const char*)recv_buff[type].data, "+QMTRECV", 8))  //+QMTRECV: 0,1,"v1/devices/me/attributes",17,"{'hello':'world'}"
                    {
                    	if(ch == '"')
                    	{
                    		semiColon_count++;
                    	}
                    	if(semiColon_count==3)
                    	{
                    		lwgsmi_parse_qmtrecv((const char*)recv_buff[type].data); /* Parse +QMTRECV response  */
                    		semiColon_count = 0;
                    		RECV_RESET(type);
                    	}
                    }
                    else if (ch == '\n')
                    {
                        lwgsmi_parse_received(&recv_buff[type]); /* Parse received string */
                        RECV_RESET(type);                      /* Reset received string */
                    }

                    /*
                     * Do we have a special sequence "> "?
                     *
                     * Check if any command active which may expect that kind of response
                     */
                    if (ch_prev2 == '\n' && ch_prev1 == '>' && ch == ' ')
                    {

                    	if(CMD_IS_CUR(LWGSM_CMD_MQTT_QMTPUBEX))
                    	{

                    		 xSemaphoreGive(modem_PortBlockSemaphore); // release Port Semaphore
                    		 count_1++;
                    	}
                    }
                }
                else
                {
                	/* We have sequence of unicode characters */
                    /*
                     * Unicode sequence characters are not "meta" characters
                     * so it is safe to just add them to receive array without checking
                     * what are the actual values
                     */
                    for (uint8_t i = 0; i < unicode.t; ++i)
                    {
                        RECV_ADD(unicode.ch[i], type); /* Add character to receive array */
                    }
                }
            }
            else if (res != lwgsmINPROG)
            {
            	/* Not in progress? */
                RECV_RESET(type);                /* Invalid character in sequence */
            }
        }

        ch_prev2 = ch_prev1; /* Save previous character as previous previous */
        ch_prev1 = ch;       /* Set current as previous */
    }
    return lwgsmOK;
}



/**************************************************************************//**
 * Function name 	: lwgsmi_unicode_decode
 * arguments		: s: Pointer to unicode decode control structure
 * 					: c: UTF-8 character sequence to test for device
 * return 		 	: lwgsmOK: Function succedded, there is a valid UTF-8 sequence
 * 					: lwgsmINPROG: Function continues well but expects some more data to finish sequence
 * 					: lwgsmERR: Error in UTF-8 sequence
 * Note				: Decode single character for unicode (UTF-8 only) format
 * 					:
 *****************************************************************************/

lwgsmr_t lwgsmi_unicode_decode(lwgsm_unicode_t* s, uint8_t c)
{
    if (s->r == 0) {    /* Are we expecting a first character? */
        s->t = 0;       /* Reset sequence */
        s->ch[0] = c;   /* Save current character */
        if (c < 0x80) { /* One byte only in UTF-8 representation */
            s->r = 0;   /* Remaining bytes */
            s->t = 1;
            return lwgsmOK; /* Return OK */
        }
        if ((c & 0xE0) == 0xC0) { /* 1 additional byte in a row = 110x xxxx */
            s->r = 1;
        } else if ((c & 0xF0) == 0xE0) { /* 2 additional bytes in a row = 1110 xxxx */
            s->r = 2;
        } else if ((c & 0xF8) == 0xF0) { /* 3 additional bytes in a row = 1111 0xxx */
            s->r = 3;
        } else {
            return lwgsmERR; /* Error parsing unicode byte */
        }
        s->t = s->r + 1;             /* Number of bytes is 1 byte more than remaining in sequence */
        return lwgsmINPROG;          /* Return in progress status */
    } else if ((c & 0xC0) == 0x80) { /* Next character in sequence */
        --s->r;                      /* Decrease character */
        s->ch[s->t - s->r - 1] = c;  /* Save character to array */
        if (s->r == 0) {             /* Did we finish? */
            return lwgsmOK;          /* Return OK, we are ready to proceed */
        }
        return lwgsmINPROG; /* Still in progress */
    }
    return lwgsmERR; /* An error, unknown UTF-8 character entered */
}


/**
 * \brief           Process received string from GSM
 * \param[in]       rcv: Pointer to \ref lwgsm_recv_t structure with input string
 */
void lwgsmi_parse_received(lwgsm_recv_t* rcv)
{
    uint8_t is_ok = 0;
    uint16_t is_error = 0;
//	sprintf((char*)tDebug,"%s",(char*)rcv->data);
//    WriteLog(1, tDebug, 1);
    /* Try to remove non-parsable strings */
    if (rcv->len == 2 && rcv->data[0] == '\r' && rcv->data[1] == '\n')
    {
        return;
    }

    /* Check OK response */
    is_ok = rcv->len == (2 + CRLF_LEN) && !strcmp(rcv->data, "OK" CRLF); /* Check if received string is OK */
    if (!is_ok)
    {                                                        /* Check for SHUT OK string */
        is_ok = rcv->len == (7 + CRLF_LEN) && !strcmp(rcv->data, "SEND OK" CRLF);
    }

    /* Check error response */
    if (!is_ok)
    {                                                                /* If still not ok, check if error? */
        is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CME ERROR", 10); /* First check +CME coded errors */
        if (!is_error)
        {                                                         /* Check basic error aswell */
            is_error = rcv->data[0] == '+' && !strncmp(rcv->data, "+CMS ERROR", 10); /* First check +CME coded errors */
            if (!is_error)
            {
                is_error = !strcmp(rcv->data, "ERROR" CRLF) || !strcmp(rcv->data, "FAIL" CRLF);
            }
        }
    }
    /* check AT ECHO mode or LoRA AT error */
    if(rcv->data[0] == 'A' && !strncmp(rcv->data, "AT", 2))
	{
    	lora_AT_ok_check = 1;
    	lora_AT_check = 1;
    	if(!strncmp(rcv->data, "AT_PARAM_ERROR", 14))
    		LoRa_Modem.lora_restart_request = 1;
    	else if(!strncmp(rcv->data, "AT_BUSY_ERROR", 13))
    		LoRa_Modem.lora_restart_request = 1;
	}

    /* check ATE ECHO mode    */
    if(rcv->data[0] == 'O' && !strncmp(rcv->data, "OK", 2))
	{
    	lora_AT_ok_check = 1;
	}

    /* Scan received strings which start with '+' */
    if (rcv->data[0] == '+')
    {
        if (!strncmp(rcv->data, "+CSQ", 4))
        {
            lwgsmi_parse_csq(rcv->data); /* Parse +CSQ response */
#if LWGSM_CFG_NETWORK
        }
        else if (!strncmp(rcv->data, "+PDP: DEACT", 11))
        {
            /* PDP has been deactivated */
            lwgsm_network_check_status(NULL, NULL, 0); /* Update status */
#endif                                                 /* LWGSM_CFG_NETWORK */
#if LWGSM_CFG_CONN
        }
        else if (!strncmp(rcv->data, "+RECEIVE", 8))
        {
            lwgsmi_parse_ipd(rcv->data);                                            /* Parse IPD */
#endif                                                                              /* LWGSM_CFG_CONN */
        }
        else if (!strncmp(rcv->data, "+CGREG", 5))
        {
        	/* Check for +CREG indication */
            //lwgsmi_parse_creg(rcv->data, LWGSM_U8(CMD_IS_CUR(LWGSM_CMD_CREG_GET))); /* Parse +CREG response */
            lwgsmi_parse_creg(rcv->data, 1); /* Parse +CREG response */
        }
        else if (!strncmp(rcv->data, "+CPIN", 5))
        {
        	/* Check for +CPIN indication for SIM */
            lwgsmi_parse_cpin(rcv->data, 1 /* !CMD_IS_DEF(LWGSM_CMD_CPIN_SET) */); /* Parse +CPIN response */
        }
        else if (!strncmp(rcv->data, "+CGATT", 6))
        {
        	/* Check for +CPIN indication for SIM */
            lwgsmi_parse_cgatt(rcv->data); /* Parse +CPIN response */
        }
        else if (!strncmp(rcv->data, "+CGDCONT", 8))
        {
            lwgsmi_parse_cgatt(rcv->data);
        }
        else if (!strncmp(rcv->data, "+CGACT", 6))
        {
        	/* Check for +CPIN indication for SIM */
            lwgsmi_parse_cgact(rcv->data, 1 /* !CMD_IS_DEF(LWGSM_CMD_CPIN_SET) */); /* Parse +CPIN response */
        }
        else if (!strncmp(rcv->data, "+CGPADDR", 8))  // +CGPADDR: 1,"100.71.189.0"
        {
        	/* Check for +CPIN indication for SIM */
        	lwgsmi_parse_cgpaddr(rcv->data, 1); /* Parse +CPIN response */
        }
        //======================== MQTT ===================
        else if (!strncmp(rcv->data, "+QMTOPEN", 8))  //+QMTOPEN: <client_idx>,<result> //[+QMTOPEN: <client_idx>,<host_name>,<port>]
        {
        	lwgsmi_parse_qmtopen(rcv->data,CMD_IS_CUR(LWGSM_CMD_MQTT_QMTOPEN_SET)); /* Parse +QMTOPEN response  */
        }
        else if (!strncmp(rcv->data, "+QMTCLOSE", 9))
        {
        	lwgsmi_parse_qmtclose(rcv->data); /* Parse +QMTOPEN response  */
        }
        else if (!strncmp(rcv->data, "+QMTCONN", 8))
        {
        	lwgsmi_parse_qmtconn(rcv->data,CMD_IS_CUR(LWGSM_CMD_MQTT_QMTCONN_SET)); /* Parse +QMTCONN response  */
        }
        else if (!strncmp(rcv->data, "+QMTDISC", 8))
        {
        	lwgsmi_parse_qmtdisc(rcv->data); /* Parse +QMTDISC response  */
        }
        else if (!strncmp(rcv->data, "+QMTSUB", 7))
        {
        	lwgsmi_parse_qmtsub(rcv->data); /* Parse +QMTSUB response  */
        }
        else if (!strncmp(rcv->data, "+QMTUNS", 7))
        {
        	lwgsmi_parse_qmtuns(rcv->data); /* Parse +QMTUNS response  */
        }
        else if (!strncmp(rcv->data, "+QMTPUBEX", 9))
        {
        	lwgsmi_parse_qmtpubex(rcv->data); /* Parse +QMTPUBEX response  */
        }
        else if (!strncmp(rcv->data, "+QMTSTAT", 8))
        {
        	lwgsmi_parse_qmtstat(rcv->data); /* Parse +QMTPUBEX response  */
        }
        else if (!strncmp(rcv->data, "+QMTRECV", 8))
        {
        	lwgsmi_parse_qmtrecv(rcv->data); /* Parse +QMTRECV response  */
        }
        else if (!strncmp(rcv->data, "+QMTPING", 8))
        {
        	lwgsmi_parse_qmtping(rcv->data); /* Parse +QMTPING response  */
        }
        //======================== MQTT ===================
		//================================SD CARD =======================//
 	   else if (!strncmp(rcv->data, "+QFLST", 6))
 	   {
 		   lwgsmi_parse_qflst(rcv->data); /* Parse +QFOPEN response  */
 	   }
        //======================== GPS ====================
        else if (!strncmp(rcv->data, "+QGPSLOC", 8))
        {
        	lwgsmi_parse_qgpsloc(rcv->data); /* Parse +QGPSLOC response  */
        }
        else if (!strncmp(rcv->data, "+QGPS", 5))
        {
        	lwgsmi_parse_qgps(rcv->data); /* Parse +QGPSLOC response  */
        }
        //======================== GPS ====================
        //======================== LORA =====================
		else if (!strncmp(rcv->data, "+EVT:TXP2P", 10))
		{
			flagLoRA_txDone = 1;
			WriteLog(1, rcv->data, 1);
		}
        else if (!strncmp(rcv->data, "+EVT:RXP2P:",11))
		{
        	const char* p = rcv->data;
//        	WriteLog(1, rcv->data, 1);
        	p += 11;

        	LoRa_Modem.lora_RSSI = lwgsmi_parse_number(&p);
        	LoRa_Modem.lora_SNR = lwgsmi_parse_number(&p);

        	char lora_data[250];
        	uint16_t index = 0;
        	lwgsmi_parse_string(&p, lora_data, sizeof(lora_data), 1);
        	if(strstr(lora_data, "AC1F09"))	// validate received string has MAC
        		index = strlen(lora_data);

        	if(index >= (MAC_LEN + MSG_ID_LEN + MSG_TYPE_LEN))
        	{
        		uint8_t msgType; AsciiToHex(&lora_data[index-2], &msgType, sizeof(msgType));

        		if(EPROM_General.Modem_EC200_presence)	// For GW
        		{
					if(msgType == LORA_LOGRATE || msgType == LORA_EVENT)
					{
						flagMqttPubLogData = 1;
						flagLoraRx = 1;
						xTimerStart(sendTimer, 0);	// re-starts send timer
						p++;
						strncpy(uplink_data[wr_index], lora_data, index-4); // Last 4 bytes not required
						uplink_RSSI[wr_index] = LoRa_Modem.lora_RSSI;
						uplink_SNR[wr_index] = LoRa_Modem.lora_SNR;
						if(++wr_index >= UPLINK_BUF_LEN)
							wr_index = 0;

						/* Prepare LoRA ACK Payload */
						memcpy(ack_payload[ack_wr_index], &lora_data[index-20],MAC_LEN + MSG_ID_LEN);	// SN & msgID
						ack_payload[ack_wr_index][MAC_LEN + MSG_ID_LEN] = '0';
						ack_payload[ack_wr_index][MAC_LEN + MSG_ID_LEN+1] = LORA_ACK + '0';
						ack_payload[ack_wr_index++][MAC_LEN + MSG_ID_LEN + MSG_TYPE_LEN] = '\0';
						if(ack_wr_index > 29)
						{
							ack_wr_index = 0;
						}
						if(ack_wr_index == ack_rd_index)
							flagSendACK = 1;
						LoraSendACKtime = HAL_GetTick();
						WriteLog(1, ack_payload[ack_wr_index-1], 1);
						WriteLog(1, "\r\n", 1);
						// if log rate is due to read data cmd then stop dl retry
						uint8_t msgID;
						AsciiToHex(&lora_data[index-4], &msgID, sizeof(msgID));
						for(uint8_t idx = 0; idx < DL_TBL_SIZE; idx++)
						{
							if(dlTable[idx].used && dlTable[idx].msgId == msgID)
							{
								dlTable[idx].ackReceived = true;
								dlTable[idx].used = false;
								WriteLog(1, "Downlink ACK Rxd\r\n", 1);
								break;
							}
						}
						addNodeToTable(&lora_data[index-20]);	// add unique entries
					}
					else if(msgType == LORA_OTA_ACK_MSG)	// ACK on OTA start msg
					{
						uint8_t sessonID;
						AsciiToHex(&lora_data[MAC_LEN], &sessonID, sizeof(sessonID));
						if(sessonID == LoraOTA.sessionId)	// Compare session ID
						{
							addNodeToTable(lora_data);	// add unique entries
							if(LoraOTA.ota_state == LORA_OTA_BEGIN)
							{
								if(LoraOTA.index >= 2)	// 3 times sent
								{
									LoraOTA.index = 0;
									LoraOTA.ota_state = LORA_OTA_PUSHING;	// transition to PUSHING
								}
								WriteLog(1, "Start OTA ACK Rxd from ", 1);
								WriteLog(1, lora_data, 1);
							}
							if(LoraOTA.ota_state == LORA_OTA_VERIFY_MSG)
							{
								WriteLog(1, "OTA Finish ACK Rxd from ", 1);
								WriteLog(1, lora_data, 1);
							}
						}
					}
					else if(msgType == LORA_OTA_NACK_MSG)
					{
						uint16_t chunk;
						AsciiToHex(&lora_data[MAC_LEN+MSG_ID_LEN], &chunk, sizeof(chunk));
						WriteLog(1, "OTA NACK Rxd from ", 1);
						WriteLog(1, lora_data, 1);
						if(chunk < LoraOTA.curentChunk)
						{
							LoraOTA.curentChunk = chunk;	// re-send requested chunk
							LoraOTA.ota_state = LORA_OTA_PUSHING;
							sprintf(print, "Will push %dnd chunk again\r\n", LoraOTA.curentChunk);
							WriteLog(1, print, 1);
						}
						else if(chunk == LoraOTA.totalChunks)	// For last chunk
						{
							LoraOTA.ready = 1;	// Re-send Verify msg
						}
					}
					else if(msgType == LORA_DL_ACK)
					{
						uint8_t msgID;
						AsciiToHex(&lora_data[MAC_LEN], &msgID, sizeof(msgID));
						for(uint8_t idx = 0; idx < DL_TBL_SIZE; idx++)
						{
							if(dlTable[idx].used && dlTable[idx].msgId == msgID)
							{
								dlTable[idx].ackReceived = true;
								dlTable[idx].used = false;
								WriteLog(1, "Downlink ACK Rxd\r\n", 1);
								break;
							}
							if(idx == DL_TBL_SIZE-1)
								WriteLog(1, "Downlink ACK Entry not matched\r\n", 1);
						}
					}
        		}
				else	// For node
				{
					uint8_t i_index = index > 20 ? index - 20 : 0;
					char devSN[MAC_LEN+1] = {0}; // 16 chars + null terminator
					strncpy(devSN, &lora_data[i_index], MAC_LEN);
					devSN[MAC_LEN] = '\0'; // Null-terminate
					WriteLog(1, rcv->data, 1);
					if(msgType == LORA_OTA_START_MSG)
					{
						/*
						[0–1]   : "DE" → updateType
						[2–3]   : "31" or "11" → RTUType
						[4–6]   : 2 bytes totalChunk (binary bytes)
						[7]     : sessionId (binary byte)
						[8]     : lastRelease (binary byte)
						[9–10]  : CRC (2 binary bytes)
						 */
						memset(&LoraOTA, 0, sizeof(LoraOTA));	// Reset LoRA OTA struct members
						AsciiToHex(&lora_data[index-6], &LoraOTA.chunkCrc, sizeof(LoraOTA.chunkCrc));
						if(LoraOTA.chunkCrc != calcCRC((uint8_t *)lora_data, index-6))
						{
							WriteLog(1, "Invalid CRC\r\n", 1);
						}
						else
						{
							if(lora_data[0] == 'D' && lora_data[1] == 'E')
							{
								if(lora_data[2] == '3' && lora_data[3] == '1')
									LoraOTA.ota_type = OTA_DELTA_H7;
								else if(lora_data[2] == '1' && lora_data[3] == '1')
									LoraOTA.ota_type = OTA_DELTA_G0;
							}
							if(LoraOTA.ota_type >= OTA_DELTA_H7)
							{
								AsciiToHex(&lora_data[12], &LoraOTA.totalChunks, sizeof(LoraOTA.totalChunks));
								AsciiToHex(&lora_data[16], &LoraOTA.sessionId, sizeof(LoraOTA.sessionId));
								AsciiToHex(&lora_data[18], &LoraOTA.forVer, sizeof(LoraOTA.forVer));
								AsciiToHex(&lora_data[22], &LoraOTA.fileCrc, sizeof(LoraOTA.fileCrc));
								AsciiToHex(&lora_data[24], &LoraOTA.fileSize, sizeof(LoraOTA.fileSize));
								sprintf(print, "TC: %d, ID: %d, ReVer: %d, Crc: %d\r\n",
										LoraOTA.totalChunks, LoraOTA.sessionId, LoraOTA.forVer, LoraOTA.fileCrc);
								WriteLog(1, print, 1);
								if(LoraOTA.totalChunks && LoraOTA.sessionId && LoraOTA.fileCrc)
								{
									LoraOTA.ota_state = LORA_OTA_BEGIN;
									LoraOTA.ready = 1;	// Respond only if it is a valid message
								}
							}
						}
					}
					else if(msgType == LORA_OTA_CHNK_MSG)	// participating in OTA
					{
						uint8_t hex_byte;

						ack_payload[0][0] = '\0';	// First clear ACK payload
						AsciiToHex(lora_data, &hex_byte, sizeof(hex_byte));
						if(LoraOTA.sessionId == hex_byte)		// current session
						{
							uint16_t chunkNo;
							AsciiToHex(&lora_data[8], &chunkNo, sizeof(chunkNo));
							sprintf(print, "Rxd chunkNo: %u\r\n",chunkNo);
							WriteLog(1, print, 1);

							if(LoraOTA.ota_type == OTA_DELTA_G0)	// Just listen to OTA
							{
								LoraOTA.ota_state = LORA_OTA_DOWNLOADING;
								LoraOTA.curentChunk = chunkNo;
							}
							else	// OTA_DELTA_H7
							{
								if(LoraOTA.ota_state == LORA_OTA_DOWNLOADING)
								{
									if(chunkNo > LoraOTA.curentChunk)
									{
										WriteLog(1, "Invalid chunk in LoRA OTA DWNLD\r\n", 1);
										BuildNackChunk();
									}
									// If chunk is less then it is a repeat so just ignore
								}
								else if(LoraOTA.ota_state == LORA_OTA_BEGIN)
								{
									LoraOTA.ota_state = LORA_OTA_DOWNLOADING;
									if(chunkNo != 0)	// missed 0th chunk
									{
										WriteLog(1, "Invalid chunk\r\n", 1);
										BuildNackChunk();
									}
								}

								if(LoraOTA.curentChunk == chunkNo && LoraOTA.ota_state == LORA_OTA_DOWNLOADING)
								{
									AsciiToHex(&lora_data[12], &LoraOTA.chunkSize, sizeof(LoraOTA.chunkSize));
									AsciiToHex(&lora_data[index-6], &LoraOTA.chunkCrc, sizeof(LoraOTA.chunkCrc));
									if(LoraOTA.chunkCrc != calcCRC((uint8_t *)lora_data, index-6))
									{
										WriteLog(1, "Invalid CRC\r\n", 1);
										BuildNackChunk();
									}
									else
									{
										if(LoraOTA.index + LoraOTA.chunkSize < sizeof(LoraOTA.bin))
										{
											AsciiToHex(&lora_data[14], &LoraOTA.bin[LoraOTA.index], LoraOTA.chunkSize);
											LoraOTA.index += LoraOTA.chunkSize;
											LoraOTA.curentChunk++;
										}
										else
											WriteLog(1, "OTA data out of bound\r\n", 1);
									}
								}
							}
							LoraOTA.ready = 1;
						}
						else	//session ID is wrong, may be msg corrupt
						{
							WriteLog(1, "Invalid session ID during LoRA OTA DWNLD\r\n", 1);
							BuildNackChunk();
							LoraOTA.ready = 1;
						}
						sprintf(print, "Chunk: %u, Size: %u, Index: %u\r\n",LoraOTA.curentChunk, LoraOTA.chunkSize, LoraOTA.index);
						WriteLog(1,print,1);
					}
					else if(msgType == LORA_OTA_VERIFY_MSG)
					{
						AsciiToHex(&lora_data[index-6], &LoraOTA.chunkCrc, sizeof(LoraOTA.chunkCrc));
						if(LoraOTA.chunkCrc != calcCRC((uint8_t *)lora_data, index-6))
						{
							// GW will retry this msg, if no ACK sent
							WriteLog(1, "Invalid CRC in LoRA OTA Finish msg\r\n", 1);
						}
						else
						{
							uint8_t hex_byte;	// session ID
							AsciiToHex(lora_data, &hex_byte, sizeof(hex_byte));

							uint16_t chunkNo;	// current chunk
							AsciiToHex(&lora_data[10], &chunkNo, sizeof(chunkNo));
							if(LoraOTA.sessionId == hex_byte)
							{
								if(LoraOTA.curentChunk == chunkNo)
								{
									OTA_store_Data.DeltaCRC = 0;	// copy Delta CRC
									OTA_store_Data.DeltaSize = 0;	// copy Delta CRC
									AsciiToHex(&lora_data[14], &OTA_store_Data.DeltaCRC, sizeof(OTA_store_Data.DeltaCRC));
									AsciiToHex(&lora_data[16], &OTA_store_Data.DeltaSize, sizeof(OTA_store_Data.DeltaSize));
									LoraOTA.ota_state = LORA_OTA_VERIFY;
								}
							}
							else
							{
								BuildNackChunk();	// Not all chunks rxd, ota state not changed
							}
							LoraOTA.ready = 1;
						}
					}
					else if(msgType == LORA_RTC_SYNC)
					{
						ReceivedDataOfLoRaClient = 1;
						lora_data[index-(strlen(MAGIC_TAIL) + MSG_ID_LEN + MSG_TYPE_LEN)] = '\0';		// strip devSN, msgID and msgType
						convert_OTA_HextoAsciiString((char *)lora_data,(char*) lora_rx_buf_ascii);
						memcpy((void *)ModbusH[COM_LORA].u8RxBuffer,lora_rx_buf_ascii, strlen(lora_data)/2);
						ModbusH[COM_LORA].u8BufferSize = strlen((const char *)lora_data)/2;
					}
					else if(!strcmp(devSN, LoRa_Modem.lora_serial_no))	// addressed to this device
					{
						if(msgType == LORA_ACK)
						{
							ack_received = 1;
							NoConnNodes = 1;	// node connected with GW
							flagExLog = 0;
							if(xTimerStop(ackTimer, 0) == pdFAIL)
								WriteLog(1, "Timer can't be stopped", 1);
							sprintf(print, "ACK received %u %u", ack_received, retry_count);
							WriteLog(1, print, 1);
						}
						else if(msgType == LORA_CMD)
						{
							ReceivedDataOfLoRaClient = 1;
							AsciiToHex(&lora_data[index-4], &lora_msgID, sizeof(lora_msgID)); // set msg ID as rxd
							lora_data[index-(MAC_LEN + MSG_ID_LEN + MSG_TYPE_LEN)] = '\0';		// strip devSN, msgID and msgType
							convert_OTA_HextoAsciiString((char *)lora_data,(char*) lora_rx_buf_ascii);
							memcpy((void *)ModbusH[COM_LORA].u8RxBuffer,lora_rx_buf_ascii, strlen(lora_data)/2);
							ModbusH[COM_LORA].u8BufferSize = strlen((const char *)lora_data)/2;
						}
					}
				}
        	}
        	else	// improper length
        		WriteLog(1, rcv->data, 1);	// For debugging
		}
        //======================== LORA =====================

//        else if (CMD_IS_CUR(LWGSM_CMD_COPS_GET) && !strncmp(rcv->data, "+COPS", 5))
//        {
//            lwgsmi_parse_cops(rcv->data); /* Parse current +COPS */
//#if LWGSM_CFG_SMS
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CMGS) && !strncmp(rcv->data, "+CMGS", 5))
//        {
//            lwgsmi_parse_cmgs(rcv->data, &lwgsm.msg->msg.sms_send.pos); /* Parse +CMGS response */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CMGR) && !strncmp(rcv->data, "+CMGR", 5))
//        {
//            if (lwgsmi_parse_cmgr(rcv->data))
//            {   /* Parse +CMGR response */
//                lwgsm.msg->msg.sms_read.read = 2; /* Set read flag and process the data */
//            }
//            else
//            {
//                lwgsm.msg->msg.sms_read.read = 1; /* Read but ignore data */
//            }
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CMGL) && !strncmp(rcv->data, "+CMGL", 5))
//        {
//            if (lwgsmi_parse_cmgl(rcv->data))
//            {   /* Parse +CMGL response */
//                lwgsm.msg->msg.sms_list.read = 2; /* Set read flag and process the data */
//            }
//            else
//            {
//                lwgsm.msg->msg.sms_list.read = 1; /* Read but ignore data */
//            }
//        }
//        else if (!strncmp(rcv->data, "+CMTI", 5))
//        {
//            lwgsmi_parse_cmti(rcv->data, 1); /* Parse +CMTI response with received SMS */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET_OPT) && !strncmp(rcv->data, "+CPMS", 5))
//        {
//            lwgsmi_parse_cpms(rcv->data, 0); /* Parse +CPMS with SMS memories info */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPMS_GET) && !strncmp(rcv->data, "+CPMS", 5))
//        {
//            lwgsmi_parse_cpms(rcv->data, 1); /* Parse +CPMS with SMS memories info */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPMS_SET) && !strncmp(rcv->data, "+CPMS", 5))
//        {
//            lwgsmi_parse_cpms(rcv->data, 2); /* Parse +CPMS with SMS memories info */
//#endif                                       /* LWGSM_CFG_SMS */
//#if LWGSM_CFG_CALL
//        }
//        else if (!strncmp(rcv->data, "+CLCC", 5))
//        {
//            lwgsmi_parse_clcc(rcv->data, 1); /* Parse +CLCC response with call info change */
//#endif                                       /* LWGSM_CFG_CALL */
//#if LWGSM_CFG_PHONEBOOK
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET_OPT) && !strncmp(rcv->data, "+CPBS", 5))
//        {
//            lwgsmi_parse_cpbs(rcv->data, 0); /* Parse +CPBS response */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPBS_GET) && !strncmp(rcv->data, "+CPBS", 5))
//        {
//            lwgsmi_parse_cpbs(rcv->data, 1); /* Parse +CPBS response */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPBS_SET) && !strncmp(rcv->data, "+CPBS", 5))
//        {
//            lwgsmi_parse_cpbs(rcv->data, 2); /* Parse +CPBS response */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPBR) && !strncmp(rcv->data, "+CPBR", 5))
//        {
//            lwgsmi_parse_cpbr(rcv->data); /* Parse +CPBR statement */
//        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CPBF) && !strncmp(rcv->data, "+CPBF", 5))
//        {
//            lwgsmi_parse_cpbf(rcv->data); /* Parse +CPBR statement */
//#endif                                    /* LWGSM_CFG_PHONEBOOK */
//        }
//
//        /* Messages not starting with '+' sign */
    }
    else if ((rcv->data[0] == 'A')&&(rcv->data[1] == 'T')&&(rcv->data[2] == '+'))
    {
        //======================== LORA =====================
        if (!strncmp(rcv->data, "AT+RECV=", 8))
        {
        	parse_lora_rx(rcv->data);
        }
//    	else if (!strncmp(rcv->data, "AT+DEVEUI=", 10))
//        {
//        	parse_lora_dev_eui(rcv->data); /* Parse AT+DEVEUI= response  */
//        }
//        else if (!strncmp(rcv->data, "AT+APPEUI=", 10))
//        {
//        	parse_lora_app_eui(rcv->data); /* Parse AT+APPEUI= response  */
//        }
//        else if (!strncmp(rcv->data, "AT+APPKEY=", 10))
//        {
//        	parse_lora_app_key(rcv->data); /* Parse AT+APPKEY= response  */
//        }
        else if (!strncmp(rcv->data, "AT+VER=", 7))
        {
        	lora_device_firmware_version_number(rcv->data); /* Parse AT+VER= response  */
        }
        else if (!strncmp(rcv->data, "AT+SN=", 6))
        {
        	parse_lora_device_serial_number(rcv->data); /* Parse AT+SN= response  */
        }
//        else if (!strncmp(rcv->data, "AT+CLASS=", 9))
//        {
//        	parse_lora_class(rcv->data); /* Parse AT+CLASS= response  */
//        }
//        else if (!strncmp(rcv->data, "AT+BAND=", 8))
//        {
//        	parse_lora_active_region_band(rcv->data); /* Parse AT+BAND= response  */
//        }
        else if (!strncmp(rcv->data, "AT+NWM=", 7))
		{
			parse_lora_network_mode(rcv->data); /* Parse AT+NJM= response  */
		}
        else if (!strncmp(rcv->data, "AT+P2P=",7))
        {
        	parse_lora_network_param(rcv->data); /* Parse AT+P2P= response  */
        }
        //======================== LORA =====================
    }
    else
    {
        if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SHUT OK" CRLF, 7 + CRLF_LEN))
        {
            is_ok = 1;
#if LWGSM_CFG_CONN
        }
        else if (LWGSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',' && rcv->data[2] == ' '
                   && (!strncmp(&rcv->data[3], "CLOSE OK" CRLF, 8 + CRLF_LEN)
                       || !strncmp(&rcv->data[3], "CLOSED" CRLF, 6 + CRLF_LEN)))
        {
            uint8_t forced = 0, num;

            num = LWGSM_CHARTONUM(rcv->data[0]); /* Get connection number */
            if (CMD_IS_CUR(LWGSM_CMD_CIPCLOSE) && lwgsm.msg->msg.conn_close.conn->num == num)
            {
                forced = 1;
                is_ok = 1; /* If forced and connection is closed, command is OK */
            }

            /* Manually stop send command? */
            if (CMD_IS_CUR(LWGSM_CMD_CIPSEND) && lwgsm.msg->msg.conn_send.conn->num == num)
            {
                /*
                 * If active command is CIPSEND and CLOSED event received,
                 * manually set error and process usual "ERROR" event on senddata
                 */
                is_error = 1; /* This is an error in response */
                lwgsmi_process_cipsend_response(rcv, &is_ok, &is_error);
            }
            lwgsmi_conn_closed_process(num, forced); /* Connection closed, process */
#endif                                               /* LWGSM_CFG_CONN */
#if LWGSM_CFG_CALL
        }
        else if (rcv->data[0] == 'C' && !strncmp(rcv->data, "Call Ready" CRLF, 10 + CRLF_LEN))
        {
            lwgsm.m.call.ready = 1;
            lwgsmi_send_cb(LWGSM_EVT_CALL_READY); /* Send CALL ready event */
        }
        else if (rcv->data[0] == 'R' && !strncmp(rcv->data, "RING" CRLF, 4 + CRLF_LEN))
        {
            lwgsmi_send_cb(LWGSM_EVT_CALL_RING); /* Send call ring */
        }
        else if (rcv->data[0] == 'N' && !strncmp(rcv->data, "NO CARRIER" CRLF, 10 + CRLF_LEN))
        {
            lwgsmi_send_cb(LWGSM_EVT_CALL_NO_CARRIER); /* Send call no carrier event */
        }
        else if (rcv->data[0] == 'B' && !strncmp(rcv->data, "BUSY" CRLF, 4 + CRLF_LEN))
        {
            lwgsmi_send_cb(LWGSM_EVT_CALL_BUSY); /* Send call busy message */
#endif                                           /* LWGSM_CFG_CALL */
#if LWGSM_CFG_SMS
        }
        else if (rcv->data[0] == 'S' && !strncmp(rcv->data, "SMS Ready" CRLF, 9 + CRLF_LEN))
        {
            lwgsm.m.sms.ready = 1;               /* SMS ready flag */
            lwgsmi_send_cb(LWGSM_EVT_SMS_READY); /* Send SMS ready event */
#endif                                           /* LWGSM_CFG_SMS */
        }


        else if ((CMD_IS_CUR(LWGSM_CMD_CGMI_GET) || CMD_IS_CUR(LWGSM_CMD_CGMM_GET) || CMD_IS_CUR(LWGSM_CMD_CGSN_GET)  || CMD_IS_CUR(LWGSM_CMD_CGMR_GET))
                   && !is_ok && !is_error && strncmp(rcv->data, "AT+", 3))
        {
            const char* tmp = rcv->data;
            //size_t tocopy;
            if (CMD_IS_CUR(LWGSM_CMD_CGMI_GET))
            {
                lwgsmi_parse_string(&tmp, (char *)ModemInfo.model_manufacturer, sizeof(ModemInfo.model_manufacturer), 1);
            }
            else if (CMD_IS_CUR(LWGSM_CMD_CGMM_GET))
            {
                lwgsmi_parse_string(&tmp, (char *)ModemInfo.model_number, sizeof(ModemInfo.model_number), 1);
            }
            else if (CMD_IS_CUR(LWGSM_CMD_CGSN_GET))
            {
                lwgsmi_parse_string(&tmp, (char *)ModemInfo.model_serial_number, sizeof(ModemInfo.model_serial_number), 1);
            }
            else if (CMD_IS_CUR(LWGSM_CMD_CGMR_GET))
            {
                lwgsmi_parse_string(&tmp, (char *)ModemInfo.model_revision, sizeof(ModemInfo.model_revision), 1);
            }
        }
        else if((CMD_IS_CUR(LWGSM_CMD_ATI))&& !is_ok && !is_error)
        {
        	const char* tmp = rcv->data;
            lwgsmi_parse_string(&tmp, (char *)ModemInfo.model_revision, sizeof(ModemInfo.model_revision), 1);
        }
//        else if (CMD_IS_CUR(LWGSM_CMD_CIFSR) && LWGSM_CHARISNUM(rcv->data[0]))
//        {
//            const char* tmp = rcv->data;
//            lwgsmi_parse_ip(&tmp, &lwgsm.m.network.ip_addr); /* Parse IP address */
//
//            is_ok = 1; /* Manually set OK flag as we don't expect OK in CIFSR command */
//        }
    }

    /* Check general responses for active commands */
    //if (lwgsm.msg != NULL)
    {
    //    if (CMD_IS_CUR(LWGSM_CMD_CPIN_GET))
    	if (Modem_AT_Command == LWGSM_CMD_CPIN_GET)
        {
            /*
             * CME ERROR 10 indicates no SIM pin inserted.
             *
             * This may be different response on various modules,
             * some replying with CME error, some with "+CPIN: SIM NOT INSERTED" message
             */
            if (is_error == 10)
            {
            	Modem_gsm_sim_state_t = LWGSM_SIM_STATE_NOT_INSERTED;
                //lwgsmi_send_cb(LWGSM_EVT_SIM_STATE_CHANGED);
            }
            else
            {
            	is_ok = 1;
            }
        }
        if ((Modem_AT_Command == LWGSM_CMD_RESET_DEVICE_FIRST_CMD)||(Modem_AT_Command == LWGSM_CMD_ATE0))
        {
        	if(is_ok)
        	{
        		Modem_PHY_Status_t = 0;
        	}
        	else
        	{
        		Modem_PHY_Status_t = lwgsmERRNODEVICE;
        	}

#if LWGSM_CFG_SMS
        }
        else if (CMD_IS_CUR(LWGSM_CMD_CMGS) && is_ok)
        {
            /* At this point we have to wait for "> " to send data */
#endif /* LWGSM_CFG_SMS */
#if LWGSM_CFG_CONN
        }
        else if (CMD_IS_CUR(LWGSM_CMD_CIPSTATUS))
        {
            /* For CIPSTATUS, OK is returned before important data */
            if (is_ok) {
                is_ok = 0;
            }
            /* Check if connection data received */
            if (rcv->len > 3)
            {
                uint8_t continueScan = 0, processed = 0;
                if (rcv->data[0] == 'C' && rcv->data[1] == ':' && rcv->data[2] == ' ')
                {
                    processed = 1;
                    lwgsmi_parse_cipstatus_conn(rcv->data, 1, &continueScan);

                    if (lwgsm.m.active_conns_cur_parse_num == (LWGSM_CFG_MAX_CONNS - 1))
                    {
                        is_ok = 1;
                    }
                }
                else if (!strncmp(rcv->data, "STATE:", 6))
                {
                    processed = 1;
                    lwgsmi_parse_cipstatus_conn(rcv->data, 0, &continueScan);
                }

                /* Check if we shall stop processing at this stage */
                if (processed && !continueScan)
                {
                    is_ok = 1;
                }
            }
        }
        else if (CMD_IS_CUR(LWGSM_CMD_CIPSTART))
        {
            /* For CIPSTART, OK is returned before important data */
            if (is_ok)
            {
                is_ok = 0;
            }

            /* Wait here for CONNECT status before we cancel connection */
            if (LWGSM_CHARISNUM(rcv->data[0]) && rcv->data[1] == ',' && rcv->data[2] == ' ')
            {
                uint8_t num = LWGSM_CHARTONUM(rcv->data[0]);
                if (num < LWGSM_CFG_MAX_CONNS)
                {
                    uint8_t id;
                    lwgsm_conn_t* conn = &lwgsm.m.conns[num]; /* Get connection handle */

                    if (!strncmp(&rcv->data[3], "CONNECT OK" CRLF, 10 + CRLF_LEN))
                    {
                        id = conn->val_id;
                        LWGSM_MEMSET(conn, 0x00, sizeof(*conn)); /* Reset connection parameters */
                        conn->num = num;
                        conn->status.f.active = 1;
                        conn->val_id = ++id; /* Set new validation ID */

                        /* Set connection parameters */
                        conn->status.f.client = 1;
                        conn->evt_func = lwgsm.msg->msg.conn_start.evt_func;
                        conn->arg = lwgsm.msg->msg.conn_start.arg;

                        /* Set status */
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_OK;
                        is_ok = 1;
                    }
                    else if (!strncmp(&rcv->data[3], "CONNECT FAIL" CRLF, 12 + CRLF_LEN))
                    {
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_ERROR;
                        is_error = 1;
                    }
                    else if (!strncmp(&rcv->data[3], "ALREADY CONNECT" CRLF, 15 + CRLF_LEN))
                    {
                        lwgsm.msg->msg.conn_start.conn_res = LWGSM_CONN_CONNECT_ALREADY;
                        is_error = 1;
                    }
                }
            }
        }
        else if (CMD_IS_CUR(LWGSM_CMD_CIPSEND))
        {
            if (is_ok)
            {
                is_ok = 0;
            }
            lwgsmi_process_cipsend_response(rcv, &is_ok, &is_error);
#endif /* LWGSM_CFG_CONN */
        }
    }

    /*
     * In case of any of these events, simply release semaphore
     * and proceed with next command
     */
    if (is_ok || is_error)
    {
        //lwgsmr_t res = lwgsmOK;

    	if(CMD_IS_CUR(LWGSM_CMD_QPWRBACKOFF))
    	{
    		if(is_ok)
    		{
    			//memset((char *)ModemInfo.Qpower_Back_off_status,0,sizeof((char *)ModemInfo.Qpower_Back_off_status));
    			//memcpy((char *)ModemInfo.Qpower_Back_off_status,"OK\r\n",4);
    			Flag_QPWRBACKOFF = 1;
    		}
    		else if(is_error)
    		{
    			//memset((char *)ModemInfo.Qpower_Back_off_status,0,sizeof((char *)ModemInfo.Qpower_Back_off_status));
    			//memcpy((char *)ModemInfo.Qpower_Back_off_status,"ERROR\r\n",7);
    			Flag_QPWRBACKOFF = 1;;
    		}
    	}

    	if(CMD_IS_CUR(LWGSM_CMD_RESET))
    	{
    		if(is_ok)
    		{
    			Flag_Reset_CFUN = 1;
    		}
    		else if(is_error)
    		{
    			Flag_Reset_CFUN = 0;
    		}
    	}
    	if(CMD_IS_CUR(LWGSM_CMD_RESET_QRESET))
    	{
    		if(is_ok)
    		{
    			Flag_Reset_QRESET = 1;
    		}
    		else if(is_error)
    		{
    			Flag_Reset_QRESET = 0;
    		}
    	}

        xSemaphoreGive(modem_PortBlockSemaphore);
    }
}

/**
 * \brief           Parse number from string
 * \note            Input string pointer is changed and number is skipped
 * \param[in,out]   str: Pointer to pointer to string to parse
 * \return          Parsed number
 */

int32_t lwgsmi_parse_number(const char** str)
{
    int32_t val = 0;
    uint8_t minus = 0;
    const char* p = *str; /*  */

    if (*p == ' ') { /* Skip ' ' character */
        ++p;
    }
    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    if (*p == ',') { /* Skip leading comma */
        ++p;
    }
    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    if (*p == '/') { /* Skip '/' character, used in datetime */
        ++p;
    }
    if (*p == ':') { /* Skip ':' character, used in datetime */
        ++p;
    }
    if (*p == '+') { /* Skip '+' character, used in datetime */
        ++p;
    }
    if (*p == '-') { /* Check negative number */
        minus = 1;
        ++p;
    }
    //while (LWGSM_CHARISNUM(*p))
    while (((*p) >= '0' && (*p) <= '9'))
    {
    	/* Parse until character is valid number */
        val = val * 10 + LWGSM_CHARTONUM(*p);
        ++p;
    }
    if (*p == '"') { /* Skip trailling quotes */
        ++p;
    }
    *str = p; /* Save new pointer with new offset */

    return minus ? -val : val;
}

/**
 * \brief           Parse float number from string
 * \note            Input string pointer is changed and number is skipped
 * \param[in,out]   str: Pointer to pointer to string to parse
 * \return          Parsed number
 */

float lwgsmi_parse_floatNumber(const char** str)
{
	uint8_t digitsAfterDesiblePoint=0;
    int32_t val = 0;
    float float_val = 0.0;
    uint8_t minus = 0;
    const char* p = *str; /*  */

    if (*p == ' ') { /* Skip ' ' character */
        ++p;
    }
    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    if (*p == ',') { /* Skip leading comma */
        ++p;
    }
    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    if (*p == '/') { /* Skip '/' character, used in datetime */
        ++p;
    }
    if (*p == ':') { /* Skip ':' character, used in datetime */
        ++p;
    }
    if (*p == '+') { /* Skip '+' character, used in datetime */
        ++p;
    }
    if (*p == '-') { /* Check negative number */
        minus = 1;
        ++p;
    }
    //while (LWGSM_CHARISNUM(*p))
    while (((*p) >= '0' && (*p) <= '9'))
    {
    	/* Parse until character is valid number */
        val = val * 10 + LWGSM_CHARTONUM(*p);
        ++p;
    }


    if(minus)
    {
    	val = val*(-1);
    }

    float_val = (float)val;

    val = 0;

    ++p; // skip .

    while (((*p) >= '0' && (*p) <= '9'))
    {
    	/* Parse until character is valid number */
        val = val * 10 + LWGSM_CHARTONUM(*p);
        ++p;
        digitsAfterDesiblePoint++;
    }

    if(digitsAfterDesiblePoint==0)
    {
    	float_val = float_val;
    }
    else if(digitsAfterDesiblePoint==1)
    {
    	float_val = float_val + (float)val/10;
    }
    else if(digitsAfterDesiblePoint==2)
    {
    	float_val = float_val + (float)val/100;
    }
    else if(digitsAfterDesiblePoint==3)
    {
    	float_val = float_val + (float)val/1000;
    }
    else if(digitsAfterDesiblePoint==4)
    {
    	float_val = float_val + (float)val/10000;
    }
    else if(digitsAfterDesiblePoint==5)
    {
    	float_val = float_val + (float)val/100000;
    }
    else if(digitsAfterDesiblePoint==6)
    {
    	float_val = float_val + (float)val/1000000;
    }

    if (*p == '"') { /* Skip trailling quotes */
        ++p;
    }
    *str = p; /* Save new pointer with new offset */

    return float_val;
}

/**
 * \brief           Parse input string as string part of AT command
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer.
 *                      Set to `NULL` in case you want to skip string in source
 * \param[in]       dst_len: Length of distance buffer,
 *                      including memory for `NULL` termination
 * \param[in]       trim: Set to `1` to process entire string,
 *                      even if no memory anymore
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim) {
    const char* p = *src;
    size_t i;

    if (*p == ',') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    if (*p == ':') {
        ++p;
    }
    i = 0;
    if (dst_len > 0) {
        --dst_len;
    }
    while (*p) {
        if ((*p == '"' && (p[1] == ',' || p[1] == '\r' || p[1] == '\n')) || (*p == '\r' || *p == '\n')) {
            ++p;
            break;
        }
        if (dst != NULL) {
            if (i < dst_len) {
                *dst++ = *p;
                ++i;
            } else if (!trim) {
                break;
            }
        }
        ++p;
    }
    if (dst != NULL) {
        *dst = 0;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse input string as string part of AT command
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer.
 *                      Set to `NULL` in case you want to skip string in source
 * \param[in]       dst_len: Length of distance buffer,
 *                      including memory for `NULL` termination
 * \param[in]       trim: Set to `1` to process entire string,
 *                      even if no memory anymore
 * \return          `1` on success, `0` otherwise
 */
uint8_t lwgsmi_parse_string_new(const char** src, char* dst, size_t dst_len, uint8_t trim) {
    const char* p = *src;
    size_t i;

    if (*p == ',') {
        ++p;
    }
    if (*p == ':') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    i = 0;
    if (dst_len > 0) {
        --dst_len;
    }
    while (*p) {
        if ((*p == '"' && (p[1] == ',' || p[1] == '\r' || p[1] == '\n')) || (*p == '\r' || *p == '\n') || (*p == ':')) {
            ++p;
            break;
        }
        if (dst != NULL) {
            if (i < dst_len) {
                *dst++ = *p;
                ++i;
            } else if (!trim) {
                break;
            }
        }
        ++p;
    }
    if (dst != NULL) {
        *dst = 0;
    }
    *src = p;
    return 1;
}
/**
 * \brief           Parse number from string as hex
 * \note            Input string pointer is changed and number is skipped
 * \param[in,out]   str: Pointer to pointer to string to parse
 * \return          Parsed number
 */
uint32_t lwgsmi_parse_hexnumber(const char** str) {
    int32_t val = 0;
    const char* p = *str; /*  */

    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    if (*p == ',') { /* Skip leading comma */
        ++p;
    }
    if (*p == '"') { /* Skip leading quotes */
        ++p;
    }
    while (LWGSM_CHARISHEXNUM(*p)) { /* Parse until character is valid number */
        val = val * 16 + LWGSM_CHARHEXTONUM(*p);
        ++p;
    }
    if (*p == ',') { /* Go to next entry if possible */
        ++p;
    }
    *str = p; /* Save new pointer with new offset */
    return val;
}

/**
 * \brief           Parse string as MAC address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      mac: Pointer to MAC memory
 * \param[in]   	withWithout: Pointer to pointer to string to parse from
 * \return          1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_mac(const char** src, lwgsm_mac_t* mac,uint8_t withWithout)
{
	uint8_t i;

    const char* p = *src;

    if (*p == '"') {
        ++p;
    }

    if(withWithout)
    {
        mac->mac[0] = lwgsmi_parse_hexnumber(&p);
        ++p;
        mac->mac[1] = lwgsmi_parse_hexnumber(&p);
        ++p;
        mac->mac[2] = lwgsmi_parse_hexnumber(&p);
        ++p;
        mac->mac[3] = lwgsmi_parse_hexnumber(&p);
        ++p;
        mac->mac[4] = lwgsmi_parse_hexnumber(&p);
        ++p;
        mac->mac[5] = lwgsmi_parse_hexnumber(&p);
    }
    else
    {
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[0] = mac->mac[0] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[1] = mac->mac[1] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[2] = mac->mac[2] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[3] = mac->mac[3] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[4] = mac->mac[4] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
        for(i=0;i<2;i++)  /* Parse until character is valid number */
        {
        	mac->mac[5] = mac->mac[5] * 16 + LWGSM_CHARHEXTONUM(*p);
            ++p;
        }
    }
    if (*p == '"') {
        ++p;
    }
    if (*p == ',') {
        ++p;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse string as  mac
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      ip: Pointer to IP memory
 * \return          `1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_ip(const char** src, lwgsm_ip_t* ip) {
    const char* p = *src;

    if (*p == ',') {
        ++p;
    }
    if (*p == '"') {
        ++p;
    }
    if (LWGSM_CHARISNUM(*p)) {
        ip->ip[0] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[1] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[2] = lwgsmi_parse_number(&p);
        ++p;
        ip->ip[3] = lwgsmi_parse_number(&p);
    }
    if (*p == '"') {
        ++p;
    }

    *src = p; /* Set new pointer */
    return 1;
}

/**
 * \brief           Parse received +CREG message
 * \param[in]       str: Input string to parse from
 * \param[in]       skip_first: Set to `1` to skip first number
 * \return          1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_creg(const char* str, uint8_t skip_first)
{
    if (*str == '+')
    {
        str += 7;
    }

    if (skip_first)
    {
        lwgsmi_parse_number(&str);
    }

    Modem_gsm_network_status_t = (lwgsm_network_reg_status_t)lwgsmi_parse_number(&str);

//    /*
//     * In case we are connected to network,
//     * scan for current network info
//     */
//    if (lwgsm.m.network.status == LWGSM_NETWORK_REG_STATUS_CONNECTED || lwgsm.m.network.status == LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING)
//    {
//        /* Try to get operator */
//        /* Notify user in case we are not able to add new command to queue */
//        lwgsm_operator_get(&lwgsm.m.network.curr_operator, NULL, NULL, 0);
//#if LWGSM_CFG_NETWORK
//    }
//    else if (lwgsm_network_is_attached())
//    {
//        lwgsm_network_check_status(NULL, NULL, 0); /* Do the update */
//#endif                                             /* LWGSM_CFG_NETWORK */
//    }
//
//    /* Send callback event */
//    lwgsmi_send_cb(LWGSM_EVT_NETWORK_REG_CHANGED);

    return 1;
}

/**
 * \brief           Parse received +CSQ signal value
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_csq(const char* str)
{
    int16_t rssi;
    if (*str == '+')
    {
        str += 6;
    }

    rssi = lwgsmi_parse_number(&str);

//    if (rssi < 32)
//    {
//        rssi = -(113 - (rssi * 2));
//    }
//    else
//    {
//        rssi = 0;
//    }

    Modem_gsm_rssi = rssi; /* Save RSSI to global variable */

//    if (lwgsm.msg->cmd_def == LWGSM_CMD_CSQ_GET && lwgsm.msg->msg.csq.rssi != NULL)
//    {
//        *lwgsm.msg->msg.csq.rssi = rssi; /* Save to user variable */
//    }

//    /* Report CSQ status */
//    lwgsm.evt.evt.rssi.rssi = rssi;
//    lwgsmi_send_cb(LWGSM_EVT_SIGNAL_STRENGTH); /* RSSI event type */

    return 1;
}

/**
 * \brief           Parse received +CPIN status value
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */


uint8_t lwgsmi_parse_cpin(const char* str, uint8_t send_evt)
{
    lwgsm_sim_state_t state;
    if (*str == '+') {
        str += 7;
    }
    if (!strncmp(str, "READY", 5))
    {
        state = LWGSM_SIM_STATE_READY;
    }
    else if (!strncmp(str, "NOT READY", 9))
    {
        state = LWGSM_SIM_STATE_NOT_READY;
    }
    else if (!strncmp(str, "NOT INSERTED", 14))
    {
        state = LWGSM_SIM_STATE_NOT_INSERTED;
    }
    else if (!strncmp(str, "SIM PIN", 7))
    {
        state = LWGSM_SIM_STATE_PIN;
    }
    else if (!strncmp(str, "SIM PUK", 7))
    {
        state = LWGSM_SIM_STATE_PUK;
    } else
    {
        state = LWGSM_SIM_STATE_NOT_READY;
    }

    /* React only on change */
    if (state != Modem_gsm_sim_state_t)
    {
    	Modem_gsm_sim_state_t = state;
        /*
         * In case SIM is ready,
         * start with basic info about SIM
         */
//        if (Modem_gsm_sim_state == LWGSM_SIM_STATE_READY)
//        {
//            lwgsmi_get_sim_info(0);
//        }

//        if (send_evt)
//        {
//            lwgsm.evt.evt.cpin.state = lwgsm.m.sim.state;
//            lwgsmi_send_cb(LWGSM_EVT_SIM_STATE_CHANGED);
//        }
    }
    return 1;
}

/**
 * \brief           Parse received +CGATT message
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_cgatt(const char* str)
{
    if (*str == '+')
    {
        str += 8;
    }

    Modem_gsm_network_GATT_Status_t = (lwgsm_network_reg_status_t)lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse received +CGATT message
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 */
uint8_t lwgsmi_parse_cgdcont(const char* str)
{
//    if (*str == '+') //CGDCONT
//    {
//        str += 8;
//    }
//
//    Modem_gsm_network_GATT_Status_t = (lwgsm_network_reg_status_t)lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse received +CGACT message
 * \param[in]       str: Input string to parse from
 * \param[in]       skip_first: Set to `1` to skip first number
 * \return          1 on success, 0 otherwise
 */

uint8_t lwgsmi_parse_cgact(const char* str, uint8_t skip_first)
{
    if (*str == '+')
    {
        str += 7;
    }

    if (skip_first)
    {
        lwgsmi_parse_number(&str);
    }

    Modem_gsm_network_GACT_Status_t = (lwgsm_network_reg_status_t)lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse received +CGPADDR: 1, message
 * \param[in]       str: Input string to parse from
 * \param[in]       skip_first: Set to `1` to skip first number
 * \return          1 on success, 0 otherwise
 */

uint8_t lwgsmi_parse_cgpaddr(const char* str, uint8_t skip_first)
{
	lwgsm_ip_t modem_ip_t;
    if (*str == '+')
    {
        str += 10;
    }

    if (skip_first)
    {
        lwgsmi_parse_number(&str);
    }

    lwgsmi_parse_ip(&str, &modem_ip_t);

    modem_ip.ip[0] = modem_ip_t.ip[0];
    modem_ip.ip[1] = modem_ip_t.ip[1];
    modem_ip.ip[2] = modem_ip_t.ip[2];
    modem_ip.ip[3] = modem_ip_t.ip[3];

    return 1;
}
/**
 * \brief           Parse +QMTOPEN response
 * \param[in]       str: Input string to parse from
 * \param[in]       set_response : `1` for set command response
 * \return          1 on success, 0 otherwise
 */

uint8_t lwgsmi_parse_qmtopen(const char* str, uint8_t set_response)
{
	unsigned char mqtt_clientId;

    if (*str == '+')
    {
        str += 10;  //skip => [+QMTOPEN: ]
    }

    mqtt_clientId = lwgsmi_parse_number(&str);

    if (set_response)
    {
    	mqtt[mqtt_clientId].open.open_result_temp = lwgsmi_parse_number(&str);
    }
    else  // get response
    {
    	//IF required add code
    	//mqtt[0].open.open_result_temp = lwgsmi_parse_number(&str);
    }

    return 1;
}

/**
 * \brief           Parse +QMTCLOSE:  response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * set : +QMTCLOSE: <client_idx>,<result>
 */

uint8_t lwgsmi_parse_qmtclose(const char* str)
{
	unsigned char mqtt_clientId;
    if (*str == '+')
    {
        str += 11;  //skip => [+QMTCLOSE: ]
    }

    mqtt_clientId = lwgsmi_parse_number(&str);
    mqtt[mqtt_clientId].close.result_temp = lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse +QMTCONN response
 * \param[in]       str: Input string to parse from
 * \param[in]       set_response : `1` for set command response
 * \return          1 on success, 0 otherwise
 * set : [+QMTCONN: <client_idx>,<result>[,<ret_code>]]
 * get : [+QMTCONN: <client_idx>,<state>]
 */

uint8_t lwgsmi_parse_qmtconn(const char* str, uint8_t set_response)
{
	unsigned char _mqtt_clientId;
    if (*str == '+')
    {
        str += 10;  //skip => [+QMTCONN: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);

    if (set_response)
    {
    	mqtt[_mqtt_clientId].sent_result = lwgsmi_parse_number(&str);
    	mqtt[_mqtt_clientId].connect.ret_code_temp = lwgsmi_parse_number(&str);
    	if(mqtt[_mqtt_clientId].connect.ret_code_temp == MQTT_CONNECTION_RET_CODE_ACCEPTED)
    	{
    		mqtt[_mqtt_clientId].status = MQTT_STATE_CONNECTION_STATE_CONNECTED;
    	}
    	else
    	{
    		mqtt[_mqtt_clientId].status = mqtt[_mqtt_clientId].connect.ret_code_temp+0x20;
    	}
    }
    else  // get response
    {
    	mqtt[_mqtt_clientId].connect.state_temp = lwgsmi_parse_number(&str);
    	mqtt[_mqtt_clientId].status = mqtt[_mqtt_clientId].connect.state_temp;
    }

    return 1;
}

/**
 * \brief           Parse +QMTDISC response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * set : +QMTDISC: <client_idx>,<result>
 */

uint8_t lwgsmi_parse_qmtdisc(const char* str)
{
	unsigned char _mqtt_clientId;
    if (*str == '+')
    {
        str += 10;  //skip => [+QMTDISC: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);
    mqtt[_mqtt_clientId].disconnect.result_temp = lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse +QMTSUB response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * set : +QMTSUB: <client_idx>,<msgid>,<result>[,<value>]
 */

uint8_t lwgsmi_parse_qmtsub(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 9;  //skip => [+QMTSUB: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);

    if(mqtt[_mqtt_clientId].msgId == lwgsmi_parse_number(&str))
    {
    	mqtt[_mqtt_clientId].sent_result=lwgsmi_parse_number(&str);
    }

    return 1;
}

/**
 * \brief           Parse +QMTUNS response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * set : +QMTUNS: <client_idx>,<msgid>,<result>[,<value>]
 */

uint8_t lwgsmi_parse_qmtuns(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 9;  //skip => [+QMTUNS: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);

    if(mqtt[_mqtt_clientId].msgId == lwgsmi_parse_number(&str))
    {
    	mqtt[_mqtt_clientId].sent_result=lwgsmi_parse_number(&str);
    }

    return 1;
}
/**
 * \brief           Parse +QMTPUBEX response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * set : +QMTPUBEX: <client_idx>,<msgid>,<result>[,<value>]
 */

uint8_t lwgsmi_parse_qmtpubex(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 11;  //skip => [+QMTPUBEX: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);

    if(mqtt[_mqtt_clientId].msgId == lwgsmi_parse_number(&str))
    {
    	mqtt[_mqtt_clientId].sent_result=lwgsmi_parse_number(&str);

    	if(mqtt[_mqtt_clientId].sent_result == MQTT_PACKET_SENT_RESULT_SUCCESS_ACK_RECEIVED)
    	{
    		modem_MQTT_publish_success_count++;
    		modem_MQTT_publish_success_timer=0;
    	}
    }

    return 1;
}

/*
 * [1] +QMTSTAT: <client_idx>,<err_code>
 * When the state of MQTT link layer is changed, the client will close the MQTT connection and report the URC.
 * [2]+QMTRECV: <client_idx>,<msgid>,<topic>[,<payload_len>],<payload>
 * Reported when the client has received the packet data from MQTT server.
 * [3] +QMTRECV: <client_idx>,<recv_id>
 * Reported when the message received from MQTT server has been stored in buffer.
 * [4] +QMTPING: <client_idx>,<result>
 * When the state of MQTT link layer is changed, the client will close the MQTT connection and report the URC.
 */

/**
 * \brief           Parse +QMTSTAT response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \+QMTSTAT: <client_idx>,<err_code>
 * \When the state of MQTT link layer is changed, the client will close the MQTT connection and report the URC.
 */

uint8_t lwgsmi_parse_qmtstat(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 10;  //skip => [+QMTSTAT: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);
    mqtt[_mqtt_clientId].Error_code = lwgsmi_parse_number(&str);
    mqtt[_mqtt_clientId].status = mqtt[_mqtt_clientId].Error_code+0x10;

    return 1;
}

/**
 * \brief           Parse +QMTRECV response
 * \param[in]       str: Input string to parse from
 * \param[in]       recBuf : [1 : receive data store in modem buff enable] [0 : receive data store in modem buff disable]
 * \return          1 on success, 0 otherwise
 * [0]+QMTRECV: <client_idx>,<msgid>,<topic>[,<payload_len>],<payload>
 * Reported when the client has received the packet data from MQTT server.
 * [1] +QMTRECV: <client_idx>,<recv_id>
 * Reported when the message received from MQTT server has been stored in buffer.
 */

uint8_t lwgsmi_parse_qmtrecv(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 10;  //skip => [+QMTRECV: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);
    lwgsmi_parse_number(&str); // msgId

    lwgsmi_parse_string(&str,mqtt[_mqtt_clientId].mqtt_rx_Topic,sizeof(mqtt[_mqtt_clientId].mqtt_rx_Topic), 1);
    mqtt[_mqtt_clientId].mqtt_rx_data_len = lwgsmi_parse_number(&str);
    if(mqtt[_mqtt_clientId].mqtt_rx_data_len < sizeof(mqtt[_mqtt_clientId].mqtt_rx_data))
    {
    	mqtt[_mqtt_clientId].mqtt_rx_state = MQTT_RX_STATE_INPROGRESS;
        ReceivedDataOfMQTTClient = _mqtt_clientId;
        rx_dataCurrentPosition = 0;
    }

    return 1;
}

/**
 * \brief           Parse +QMTPING response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \+QMTPING: <client_idx>,<result>
 * \When the state of MQTT link layer is changed, the client will close the MQTT connection and report the URC.
 */

uint8_t lwgsmi_parse_qmtping(const char* str)
{
	unsigned char _mqtt_clientId;

    if (*str == '+')
    {
        str += 10;  //skip => [+QMTPING: ]
    }

    _mqtt_clientId = lwgsmi_parse_number(&str);
    mqtt[_mqtt_clientId].status = lwgsmi_parse_number(&str)+0x30;

    return 1;
}


uint8_t lwgsmi_parse_qflst(const char* str)
{

	 if (strstr(str, "\"SD:") != NULL)
	    {
	        flag_SDCARD_ok = 1;
	        return 1;
	    }
	 return 0;
}

/**
 * \brief           Parse +QMTPING response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \+QGPSLOC	: <UTC>,<latitude>,<longitude>,<HDOP>,<altitude>,<fix>,<COG>,<spkm>,<spkn>,<date>,<nsat>
 * UTC 			: hhmmss.sss
 * latitude 	: If <mode> is 2 : Format: (-)dd.ddddd (Quoted from GPGGA sentence) dd.ddddd Degree. Range: -89.9999–89.9999 - South latitude
 * longitude 	: If <mode> is 2:Format: (-)ddd.ddddd (Quoted from GPGGA sentence) ddd.ddddd Degree. Range: -179.99999-179.99999 - West longitude
 * HDOP 		: Horizontal dilution of precision. Range: 0.5–99.9 (Quoted from GPGGA sentence)
 * altitude 	: The altitude of the antenna away from the sea level, and is accurate to one decimal place. Unit: meter (Quoted from GPGGA sentence).
 * fix 			: Integer type. GNSS positioning mode (Quoted from GAGSA/GPGSA sentence). [2 2D positioning] [3 3D positioning]
 * COG 			: String type. Course Over Ground based on true north. Format: ddd.mm (Quoted from GPVTG sentence).
 * spkm 		: Speed over ground. Accurate to one decimal place. Unit: km/h (Quoted from GPVTG sentence).
 * spkn 		: Speed over ground. Accurate to one decimal place. Unit: knots (Quoted from GPVTG sentence)
 * date 		: UTC date. Format: ddmmyy (Quoted from GPRMC sentence).
 * nsat 		: Number of satellites. The value should be kept two digits, and add 0 If the leading digit is insufficient (Quoted from GPGGA sentence).
 *
 * \Acquire Positioning Information.
 */

uint8_t lwgsmi_parse_qgpsloc(const char* str)
{

    if (*str == '+')
    {
        str += 10;  //skip => [+QGPSLOC: ]
    }

    gps.utc.hour = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    gps.utc.min = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    gps.utc.sec = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    str ++; // skip .

    gps.utc.miliSec = (LWGSM_CHARTONUM(*str))*100 + LWGSM_CHARTONUM(*(str+1))*10 +LWGSM_CHARTONUM(*(str+2));
    str += 3;

    gps.latitude = lwgsmi_parse_floatNumber(&str);
    gps.longitude = lwgsmi_parse_floatNumber(&str);

    gps.HDOP = lwgsmi_parse_floatNumber(&str);

    gps.altitude = lwgsmi_parse_floatNumber(&str);

    gps.fix = lwgsmi_parse_number(&str);

    gps.COG = lwgsmi_parse_floatNumber(&str);

    gps.speedkm = lwgsmi_parse_floatNumber(&str);

    gps.speedkn = lwgsmi_parse_floatNumber(&str);

    str++; // skip ,

    gps.utc.date = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    gps.utc.month = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    gps.utc.year = (LWGSM_CHARTONUM(*str))*10 + LWGSM_CHARTONUM(*(str+1));
    str += 2;

    gps.noOfSatellites = lwgsmi_parse_number(&str);

    if(gps.fix == 2 || gps.fix == 3)
    {
    	gFinalAnaValF[GPS_LAT_gFinalAnaValF] = EPROM_General.Cust_Detail.Lattitude = gps.latitude;
    	gFinalAnaValF[GPS_Log_gFinalAnaValF] = EPROM_General.Cust_Detail.Longitude = gps.longitude;
    }
    return 1;
}

/**
 * \brief           Parse +QGPS response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \+QGPS: <GNSS_state>
 * \check GPS enable or Not
 */
uint8_t lwgsmi_parse_qgps(const char* str)
{

    if (*str == '+')
    {
        str += 7;  //skip => [+QGPS: ]
    }

    gps.GNSS_state = lwgsmi_parse_number(&str);

    return 1;
}

/**
 * \brief           Parse AT+BAND= response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \AT+NJS==<state>\r\n
 */

uint8_t parse_lora_active_region_band(const char* str)
{
	if (*str == 'A')
	{
		str += 8;  //skip => [AT+BAND=]
	}

	LoRa_Modem.lora_active_region = lwgsmi_parse_number(&str);

	return 1;
}

/**
 * \brief           Parse AT+NJM= response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \AT+NJS==<state>\r\n
 */

uint8_t parse_lora_network_mode(const char* str)
{
	if (*str == 'A')
	{
		str += 7;  //skip => [AT+NJM=]
	}

	LoRa_Modem.lora_network_Mode = lwgsmi_parse_number(&str);

	return 1;
}

/**
 * \brief           Remove extra byte
 * \param[in]       str: Input string to parse from
 * \return          nothing
 */

void remove_extra_byte(char *str)
{
	char *target = strstr(str, "AC1F09");
	if (target)
	{
		char *pos = target + strlen("AC1F09"); // move to just after "AC1F09"
		memmove(pos, pos + 2, strlen(pos + 2) + 1);
	}
}

/**
 * \brief           Parse AT+SN= response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \AT+SN=<state>\r\n
 */

uint8_t parse_lora_device_serial_number(const char* str)
{
	if (*str == 'A')
	{
		str += 6;  //skip => [AT+SN=]
	}

	memset(LoRa_Modem.lora_serial_no,0,sizeof(LoRa_Modem.lora_serial_no));
	lwgsmi_parse_string(&str,(char*)LoRa_Modem.lora_serial_no,sizeof(LoRa_Modem.lora_serial_no), 1);
	remove_extra_byte(LoRa_Modem.lora_serial_no);
	return 1;
}

/**
 * \brief           Parse AT+VER= response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \AT+VER=<state>\r\n
 */

uint8_t lora_device_firmware_version_number(const char* str)
{
	if (*str == 'A')
	{
		str += 7;  //skip => [AT+VER=]
	}

	memset(LoRa_Modem.lora_device_firmware_version_number,0,sizeof(LoRa_Modem.lora_device_firmware_version_number));
	lwgsmi_parse_string(&str,(char*)LoRa_Modem.lora_device_firmware_version_number,sizeof(LoRa_Modem.lora_device_firmware_version_number), 1);


	return 1;
}

/**
 * \brief           Parse AT+RECV= response
 * \param[in]       str: Input string to parse from
 * \return          1 on success, 0 otherwise
 * \AT+RECV=<state>\r\n
 */

uint8_t parse_lora_rx(const char* str)
{
	if (*str == 'A')
	{
		str += 8;  //skip => [AT+RECV=]
	}

	lora_rx_port = lwgsmi_parse_number(&str);

	if(lora_rx_port>0)
	{
		memset(lora_rx_buf,0,sizeof(lora_rx_buf));
		memset(lora_rx_buf_ascii,0,sizeof(lora_rx_buf_ascii));
		lwgsmi_parse_string(&str,(char*)lora_rx_buf,sizeof(lora_rx_buf), 1);
		convert_OTA_HextoAsciiString(lora_rx_buf,(char*) lora_rx_buf_ascii);
		memcpy(ModbusH[COM_LORA].u8RxBuffer,lora_rx_buf_ascii, strlen(lora_rx_buf)/2);
		ModbusH[COM_LORA].u8BufferSize = strlen((const char *)lora_rx_buf)/2;
	}

	return 1;
}

void parse_lora_network_param(const char* str)
{
		if (*str == 'A')
		{
			str += 7;  //skip => [AT+NJM=]
		}

		LoRa_Modem.p2pFreq = lwgsmi_parse_number(&str);
		LoRa_Modem.p2p_Spreading_Factor = lwgsmi_parse_number(&str);
		LoRa_Modem.p2p_Bandwidth = lwgsmi_parse_number(&str);
		LoRa_Modem.p2p_Code_Rate = lwgsmi_parse_number(&str);
		LoRa_Modem.p2p_Preamble_Length = lwgsmi_parse_number(&str);
		LoRa_Modem.p2p_TX_Power = lwgsmi_parse_number(&str);
}

void addNodeToTable(char *loraRx)
{
	// create mac table to show no of connected nodes
	char tempMacID[MAC_LEN+1];
	uint8_t found_idx, empty_idx;

	found_idx = empty_idx = 255;
	strncpy(tempMacID, loraRx, MAC_LEN);
	tempMacID[MAC_LEN] = '\0';
	for(uint8_t idx = 0; idx < MAX_NODES; idx++)
	{
		if(nodesCon[idx].macId[0] != '\0')
		{
			if(!strcmp(nodesCon[idx].macId, tempMacID))
			{
				nodesCon[idx].timeoutTick = HAL_GetTick();
				found_idx = idx;
				break;
			}
		}
		else if(empty_idx == 255)
			empty_idx = idx;
	}
	if(found_idx == 255)	// not found then add
	{
		strcpy(nodesCon[empty_idx].macId,tempMacID);
		nodesCon[empty_idx].timeoutTick = HAL_GetTick();
		NoConnNodes++;
		maxConnNode++;
		sprintf(print, "Total nodes connected: %d\r\n", NoConnNodes);
		WriteLog(1, print, 1);
	}
}
