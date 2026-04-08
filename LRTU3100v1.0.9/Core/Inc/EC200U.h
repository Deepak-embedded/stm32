/*
 * EC200U.h
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

#ifndef INC_EC200U_H_
#define INC_EC200U_H_

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
extern char flag_modem_reboot, flagSyncNodeTime;
extern int16_t Modem_AT_Command;
extern osThreadId EC200U_TaskHandle;
extern osSemaphoreId modem_PortBlockSemaphore;
extern osSemaphoreId modem_SequenceBlockSemaphore;
/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void MX_UART4_Init(void);
void StartEC200UTask(void const * argument);
void EC200U_start();
void delay_with_DevEUI();
uint16_t AT_Serial_Send_fn(const void* data, size_t len);


unsigned char EC200U_modem_reboot();
unsigned char EC200U_Modeminfo();
lwgsmr_t EC200U_networkCheck();
lwgsmr_t EC200U_make_internetConnection();

void lwgsmi_send_signed_number(int32_t num, uint8_t q, uint8_t c);
void lwgsmi_send_port(lwgsm_port_t port, uint8_t q, uint8_t c);
void lwgsmi_send_number(uint32_t num, uint8_t q, uint8_t c);
void lwgsmi_send_string(const char* str, uint8_t e, uint8_t q, uint8_t c);
void lwgsmi_send_ip_mac(const void* d, uint8_t is_ip,uint8_t withWithout, uint8_t q, uint8_t c);
lwgsmr_t lwgsmi_initiate_cmd(lwgsm_cmd_t cmd,void *argument);

#endif /* INC_EC200U_H_ */
