/*
 * EC200U.h
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

#ifndef INC_RxRingProcess_H_
#define INC_RxRingProcess_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ATmodemTypes.h"
/**************************************************************************//**
 * Constant
 *****************************************************************************/

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/

/**************************************************************************//**
 * Macro
 *****************************************************************************/

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_EC200U_RxRingProcess;
extern uint8_t EC200U_RX_Buff[1200];        //buffer to feel from Rx intrupt
extern lwrb_t EC200U_RX_rb;  				//Ring buffer instance for RX data
extern uint8_t EC200U_RX_rb_data[1500]; 	//Ring buffer data array for RX DMA

extern char lora_AT_check;
extern char lora_AT_ok_check;
/**
 * \brief           Receive character structure to handle full line terminated with `\n` character
 */
typedef struct {
    char data[501]; /*!< Received characters */
    size_t len;     /*!< Length of valid characters */
} lwgsm_recv_t;

typedef struct {
	unsigned char model_manufacturer[100];
	unsigned char model_number[100];
	unsigned char model_serial_number[100];
	unsigned char model_revision[100];
} ModemInfo_t;

typedef struct {
	unsigned char lora_network_Mode; //the network mode (0 = ABP, 1 = OTAA)
	unsigned char lora_power; //set the power (0 high and 10 low)
	unsigned char lora_active_region;//AT+BAND: get or set the active region(0 = EU433, 1 = CN470, 2 = RU864, 3 = IN865, 4 = EU868,5 = US915, 6 = AU915, 7 = KR920, 8 = AS923-1, 9 = AS923-2, 10 = AS923-3, 11 = AS923-4, 12 = LA915)
	char lora_serial_no[24]; //AT+SN device serial number 1-18byte
	unsigned char lora_device_firmware_version_number[24]; //AT+VER=firmware version
	signed char lora_RSSI;
	int8_t lora_SNR;
	unsigned int lora_baudrate; // * AT+BAUD= set the baudrate
	unsigned char lora_rxState;
	unsigned char lora_restart_request;
	unsigned int p2pFreq;
	unsigned char p2p_Spreading_Factor;
	unsigned char p2p_Bandwidth;
	unsigned char p2p_Code_Rate;
	unsigned char p2p_Preamble_Length;
	unsigned char p2p_TX_Power;
} lora_Modem_t;

extern lora_Modem_t LoRa_Modem;
extern unsigned char lora_tx_port;
extern char lora_tx_buf[200];
//extern unsigned char lora_tx_buf_ascii[256];
extern unsigned char lora_rx_port;
extern unsigned char lora_rx_buf[200];
extern unsigned char lora_rx_buf_ascii[206];

extern ModemInfo_t ModemInfo;
extern lwgsmr_t Modem_PHY_Status;
extern lwgsmr_t Modem_PHY_Status_t;

extern unsigned char PDP_Context_ID;
extern char PDP_Context_APN[30];

extern int16_t Modem_gsm_rssi;

extern lwgsm_sim_state_t Modem_gsm_sim_state;
extern lwgsm_sim_state_t Modem_gsm_sim_state_t;

extern lwgsm_network_reg_status_t Modem_gsm_network_status;
extern lwgsm_network_reg_status_t Modem_gsm_network_status_t;

extern unsigned char Modem_gsm_network_GATT_Status;
extern unsigned char Modem_gsm_network_GATT_Status_t;
extern unsigned char Modem_gsm_network_GACT_Status;
extern unsigned char Modem_gsm_network_GACT_Status_t;

extern lwgsm_ip_t modem_ip;

extern int16_t Modem_AT_Command;
extern int16_t modemCommandSequence;

extern unsigned int rx_dataCurrentPosition;        // use for positioning for RX data of mqtt or UDU or TCP
extern unsigned char ReceivedDataOfMQTTClient;

extern unsigned int lora_rx_dataCurrentPosition;
extern unsigned char ReceivedDataOfLoRaClient;

extern osThreadId RxRingProcess_TaskHandle;
extern char Flag_Reset_CFUN, Flag_Reset_QRESET,Flag_QPWRBACKOFF;

extern unsigned int count_1,count_2,count_3,count_4;
extern uint8_t maxConnNode;
extern uint8_t flagExLog;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void MX_UART4_Init(void);
void Start_RxRingProcessTask(void const * argument);
void RxRingProcess_start();
void RxRingFiller(lwrb_t *rb, uint8_t*  data, size_t len);
void dataSendToRxRingFiller(lwrb_t *rb, uint8_t *buffer, size_t length, uint16_t RxXferCount, unsigned char callFrom);

lwgsmr_t RxRingProcess(const void* data, size_t data_len, uint8_t type);
void RxRingProcess_buffer(lwrb_t * rx_rb, uint8_t type);
void lwgsmi_parse_received(lwgsm_recv_t* rcv);
lwgsmr_t lwgsmi_unicode_decode(lwgsm_unicode_t* s, uint8_t c);
uint8_t lwgsmi_parse_ip(const char** src, lwgsm_ip_t* ip);
uint8_t lwgsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim);
//====== Network ==============================
int32_t lwgsmi_parse_number(const char** str);
float lwgsmi_parse_floatNumber(const char** str);
uint8_t lwgsmi_parse_creg(const char* str, uint8_t skip_first);
uint8_t lwgsmi_parse_csq(const char* str);
uint8_t lwgsmi_parse_cpin(const char* str, uint8_t send_evt);
uint8_t lwgsmi_parse_cgatt(const char* str);
uint8_t lwgsmi_parse_cgdcont(const char* str);
uint8_t lwgsmi_parse_cgact(const char* str, uint8_t skip_first);
uint8_t lwgsmi_parse_cgpaddr(const char* str, uint8_t skip_first);
//====== MQTT ==============================
uint8_t lwgsmi_parse_qmtopen(const char* str, uint8_t set_response);
uint8_t lwgsmi_parse_qmtclose(const char* str);
uint8_t lwgsmi_parse_qmtconn(const char* str, uint8_t set_response);
uint8_t lwgsmi_parse_qmtdisc(const char* str);
uint8_t lwgsmi_parse_qmtpubex(const char* str);
uint8_t lwgsmi_parse_qmtsub(const char* str);
uint8_t lwgsmi_parse_qmtuns(const char* str);
uint8_t lwgsmi_parse_qmtstat(const char* str);
uint8_t lwgsmi_parse_qmtrecv(const char* str);
uint8_t lwgsmi_parse_qmtping(const char* str);
//////////SD CARD//////////////============
uint8_t lwgsmi_parse_qflst(const char* str);
//====== GPS ==============================
uint8_t lwgsmi_parse_qgpsloc(const char* str);
uint8_t lwgsmi_parse_qgps(const char* str);
uint8_t lwgsmi_parse_qgpsrmc(const char* str);
//====== BLE ==============================
uint8_t lwgsmi_parse_qbtpwr(const char* str);
uint8_t lwgsmi_parse_qbtleaddr(const char* str);
uint8_t lwgsmi_parse_qbtgatscon(const char* str);
uint8_t lwgsmi_parse_qbtgatsdcon(const char* str);
uint8_t lwgsmi_parse_qbtlevaldata(const char* str);
uint8_t lwgsmi_parse_qbtgatrddataind(const char* str);
unsigned char parse_BLE_Data(unsigned short int _att_handle,unsigned short int _length,const char* _str);
//====== LORA ==============================
uint8_t parse_lora_linkCheck(const char* str);
uint8_t parse_lora_rx(const char* str);
uint8_t parse_lora_rx_event(const char* str);
uint8_t lora_device_firmware_version_number(const char* str);
uint8_t parse_lora_device_serial_number(const char* str);
uint8_t parse_lora_netork_join_state(const char* str);
uint8_t parse_lora_app_key(const char* str);
uint8_t parse_lora_app_eui(const char* str);
uint8_t parse_lora_dev_eui(const char* str);

uint8_t parse_lora_adaptive_data_rate(const char* str);
uint8_t parse_lora_class(const char* str);
uint8_t parse_lora_active_region_band(const char* str);
uint8_t parse_lora_network_mode(const char* str);
void parse_lora_network_param(const char* str);
void remove_extra_byte(char *str);
void addNodeToTable(char *loraRx);

#endif /* INC_RxRingProcess_H_ */
