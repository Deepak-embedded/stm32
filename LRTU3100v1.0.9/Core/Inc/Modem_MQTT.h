/*
 * EC200U.h
 *
 *  Created on: Nov 23, 2022
 *      Author: maulin
 */

#ifndef INC_Modem_MQTT_H_
#define INC_Modem_MQTT_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "ATmodemTypes.h"

/**************************************************************************//**
 * Constant
 *****************************************************************************/

#define CIM_MAX_SIZE_OF_MQTT_PAYLOAD 	1000

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/

/**
 * @ingroup mqtt
 * Client information and connection parameters */

typedef enum {
	MQTT_OPEN_RESULT_FAIL_TO_OPEN = -1,            			/*!<Failed to open network */
    MQTT_OPEN_RESULT_OPEN_SUCCESS = 0x00,            		/*!<Network opened successfully */
	MQTT_OPEN_RESULT_WRONG_PARA = 0x01,        				/*!< Wrong parameter */
	MQTT_OPEN_RESULT_IDENTIFIER_OCCUPIED = 0x02,        	/*!< MQTT identifier is occupied */
	MQTT_OPEN_RESULT_FAIL_TO_ACTIVE_PDP = 0x03,             /*!< Failed to activate PDP */
	MQTT_OPEN_RESULT_FAIL_TO_DOMAIN = 0x04,  				/*!< Failed to parse domain name */
	MQTT_OPEN_RESULT_NET_CONNECTION_ERR = 0x05, 			/*!< Network connection error */
	MQTT_OPEN_RESULT_DEFAULT = 0x06 							/*!< default Result  */
} mqtt_open_result_t;

typedef enum {
	MQTT_CLOSE_RESULT_FAILED = -1, 							/*!< -1 Failed to close network */
	MQTT_CLOSE_RESULT_SUCCESS = 0x00, 						/*!< 0 Network closed successfully */
	MQTT_CLOSE_RESULT_DEFAULT = 0x01						/*!< default Result  */
} mqtt_close_result_t;

typedef enum {
	MQTT_CONNECTION_RET_CODE_ACCEPTED = 0x00,            			/*!<Connection Accepted */
	MQTT_CONNECTION_RET_CODE_PRTOCOL_VER_ERR = 0x01,        		/*!< Connection Refused: Unacceptable Protocol Version */
	MQTT_CONNECTION_RET_CODE_IDENTIFIER_REJECTED = 0x02,        	/*!< Connection Refused: Identifier Rejected */
	MQTT_CONNECTION_RET_CODE_SERVER_UNAVAILABLE = 0x03,             /*!< Connection Refused: Server Unavailable */
	MQTT_CONNECTION_RET_CODE_BAD_USER_PASS = 0x04,  				/*!< Connection Refused: Bad User Name or Password */
	MQTT_CONNECTION_RET_CODE_NET_NOT_AUTORIZED = 0x05, 				/*!< Connection Refused: Not Authorized */
	MQTT_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT = 0x06			/*!< default Result  */
} mqtt_connection_ret_code_t;


typedef enum {
	MQTT_CONNECTION_STATE_INITIALIZING = 0x01,          /*!< MQTT is initializing */
	MQTT_CONNECTION_STATE_CONNECTING = 0x02,        	/*!< MQTT is connecting */
	MQTT_CONNECTION_STATE_CONNECTED = 0x03,        		/*!< MQTT is connected */
	MQTT_CONNECTION_STATE_DISCONNECTED = 0x04,         	/*!< MQTT is disconnecting */
	MQTT_CONNECTION_STATE_DEFAULT = 0x05				/*!< default Result  */
} mqtt_connection_state_t;

typedef enum {
	MQTT_DISCONNECTION_RESULT_FAIL = -1,          		/*!< -1 Failed to close connection */
	MQTT_DISCONNECTION_RESULT_SUCCESS = 0x00,        	/*!< 0 Connection closed successfully */
	MQTT_DISCONNECTION_RESULT_DEFAULT = 0x01			/*!< default Result  */
} mqtt_disconnection_result_t;

typedef enum {
	MQTT_PACKET_SENT_RESULT_SUCCESS_ACK_RECEIVED = 0x00,    /*!< Packet sent successfully and ACK received from server */
	MQTT_PACKET_SENT_RESULT_RETRANSMISSION = 0x01,        	/*!< Packet retransmission */
	MQTT_PACKET_SENT_RESULT_FAIL = 0x02,        			/*!< Failed to send packet */
	MQTT_PACKET_SENT_RESULT_DEFAULT = 0x03					/*!< default Result  */
} mqtt_sent_result_t;

/*
 <err_code> Description How to do
1	[Connection is closed or reset by a peer end.]	[Execute AT+QMTOPEN command and reopen MQTT connection.]

2	[Sending PINGREQ packet timed out or failed.] 	[Deactivate PDP first, and then active PDP and reopen MQTT connection.]

3	[Sending CONNECT packet timed out or failed.]	[1. Check whether the inputted user name and password are correct.]
													[2. Make sure the client ID is not used.]
													[3. Reopen MQTT connection and try to send CONNECT packet to server again.]

4	[Receiving CONNACK packet timed out or failed.]	[1. Check whether the inputted user name and password are correct.]
													[2. Make sure the client ID is not used. ]
													[3. Reopen MQTT connection and try to send CONNECT packet to server again.]

5	[The client sends DISCONNECT packet to sever
		and the server closes MQTT connection.]		[This is a normal process.]

6	[The client closes MQTT connection due to 		[1. Make sure the data is correct.]
		packet sending failure all the time.]		[2. Try to reopen MQTT connection since there
															  may be network congestion or an error.]

7	[The link is not alive or the server is
		unavailable.]								[Make sure the link is alive or the server is available currently.]

8	[The client closes the MQTT connection]			[Try to reconnect]
*/

typedef enum {
	MQTT_DISCONNECT_REASON_DEFAULT = 0x00,						/*!< Default State. */
	MQTT_DISCONNECT_REASON_CONN_CLOSE_BY_PEER = 0x01,          	/*!< Connection is closed or reset by a peer end. */
	MQTT_DISCONNECT_REASON_PINGREG_TIMEOUT = 0x02,        		/*!< Sending PINGREQ packet timed out or failed. */
	MQTT_DISCONNECT_REASON_CONNECT_TIMEOUT = 0x03,        		/*!< Sending CONNECT packet timed out or failed. */
	MQTT_DISCONNECT_REASON_CONNACK_TIMEOUT = 0x04,          	/*!< Receiving CONNACK packet timed out or failed. */
	MQTT_DISCONNECT_REASON_DISCONNECT_SELF = 0x05,          	/*!< The client sends DISCONNECT packet to sever and the server closes MQTT connection*/
	MQTT_DISCONNECT_REASON_PACKET_SENT_FAIL_MANYTIME = 0x06,    /*!< The client closes MQTT connection due to packet sending failure all the time.*/
	MQTT_DISCONNECT_REASON_SERVER_UNAVAILABLE = 0x07,        	/*!< The link is not alive or the server is unavailable. */
	MQTT_DISCONNECT_REASON_CLIENT_CLOSE = 0x08,          		/*!< The client closes the MQTT connection */
} mqtt_disconnect_urc_reason_t;

typedef enum {
	//MQTT_STATE_CONNECTED = 0x00,
	MQTT_STATE_CONNECTION_STATE_INITIALIZING = 0x01,          			/*!< MQTT is initializing */
	MQTT_STATE_CONNECTION_STATE_CONNECTING = 0x02,        				/*!< MQTT is connecting */
	MQTT_STATE_CONNECTION_STATE_CONNECTED = 0x03,        				/*!< MQTT is connected */
	MQTT_STATE_CONNECTION_STATE_DISCONNECTED = 0x04,         			/*!< MQTT is disconnecting */
	MQTT_STATE_DISCONNECT_REASON_CONN_CLOSE_BY_PEER = 0x11,          	/*!< Connection is closed or reset by a peer end. */
	MQTT_STATE_DISCONNECT_REASON_PINGREG_TIMEOUT = 0x12,        		/*!< Sending PINGREQ packet timed out or failed. */
	MQTT_STATE_DISCONNECT_REASON_CONNECT_TIMEOUT = 0x13,        		/*!< Sending CONNECT packet timed out or failed. */
	MQTT_STATE_DISCONNECT_REASON_CONNACK_TIMEOUT = 0x14,          		/*!< Receiving CONNACK packet timed out or failed. */
	MQTT_STATE_DISCONNECT_REASON_DISCONNECT_SELF = 0x15,          		/*!< The client sends DISCONNECT packet to sever and the server closes MQTT connection*/
	MQTT_STATE_DISCONNECT_REASON_PACKET_SENT_FAIL_MANYTIME = 0x16,    	/*!< The client closes MQTT connection due to packet sending failure all the time.*/
	MQTT_STATE_DISCONNECT_REASON_SERVER_UNAVAILABLE = 0x17,        		/*!< The link is not alive or the server is unavailable. */
	MQTT_STATE_DISCONNECT_REASON_CLIENT_CLOSE = 0x18,          			/*!< The client closes the MQTT connection */

	//MQTT_CONNECTION_RET_CODE_ACCEPTED = 0x20,            				/*!<Connection Accepted */
	MQTT_STATE_CONNECTION_RET_CODE_PRTOCOL_VER_ERR = 0x21,        			/*!< Connection Refused: Unacceptable Protocol Version */
	MQTT_STATE_CONNECTION_RET_CODE_IDENTIFIER_REJECTED = 0x22,        		/*!< Connection Refused: Identifier Rejected */
	MQTT_STATE_CONNECTION_RET_CODE_SERVER_UNAVAILABLE = 0x23,             	/*!< Connection Refused: Server Unavailable */
	MQTT_STATE_CONNECTION_RET_CODE_BAD_USER_PASS = 0x24,  					/*!< Connection Refused: Bad User Name or Password */
	MQTT_STATE_CONNECTION_RET_CODE_NET_NOT_AUTORIZED = 0x25, 					/*!< Connection Refused: Not Authorized */
	MQTT_STATE_CONNECTION_RET_CODE_CLOSE_RESULT_DEFAULT = 0x26,				/*!< default Result  */

	MQTT_STATE_PING_FAILED = 0x31,				/*!< PING URC says fail */

	MQTT_STATE_DISCONNECTED_DUE_TO_NETWORK_DOWN = 0x41		/*!< GSM Networking task indicate network is down than make mqtt status disconnected */
} mqtt_status_t;

typedef enum {
	MQTT_RX_STATE_FREE = 0x00,
	MQTT_RX_STATE_INPROGRESS = 0x01,
	MQTT_RX_STATE_DATA_READY = 0x02,
}
mqtt_rx_state_t;

typedef struct
{
	uint8_t msg_recv_mode;
	uint8_t msg_len_enable;
}
modem_mqtt_cfg_commands_recv_mode_t;

typedef struct
{
	uint16_t keep_alive_time; /** keep alive time in seconds, 0 to disable keep alive functionality*/
}
modem_mqtt_cfg_commands_keepalive_t;

typedef struct
{
	modem_mqtt_cfg_commands_keepalive_t keepalive;
	modem_mqtt_cfg_commands_recv_mode_t recv_mode;
}
modem_mqtt_cfg_commands_t;

typedef struct
{
	const char* Broker;  // IP or Domain in Srting formate
	uint16_t Broker_Port;
	mqtt_open_result_t open_result;
	mqtt_open_result_t open_result_temp;
}
modem_mqtt_open_commands_t;

typedef struct
{
	mqtt_close_result_t result;
	mqtt_close_result_t result_temp;
}
modem_mqtt_close_commands_t;

typedef struct
{
	const char* client_id; /** Client identifier, must be set by caller */ // client id accroding to Broker
	const char* client_user; /** User name, set to NULL if not used */
	const char* client_pass; /** Password, set to NULL if not used */
	mqtt_connection_ret_code_t ret_code;
	mqtt_connection_ret_code_t ret_code_temp;
	mqtt_connection_state_t state;
	mqtt_connection_state_t state_temp;
}
modem_mqtt_connect_commands_t;

typedef struct
{
	mqtt_disconnection_result_t result;
	mqtt_disconnection_result_t result_temp;
}
modem_mqtt_disconnect_commands_t;

typedef struct
{
	unsigned char QOS;
	const char* topic;
}
modem_mqtt_sub_commands_t;

typedef struct
{
	unsigned char QOS;
	unsigned char retain;
	const char* topic;
	const char* msg;
	unsigned int msg_len;
}
modem_mqtt_pub_commands_t;



typedef struct
{

	uint8_t client_idx; // module Client ID

	modem_mqtt_cfg_commands_t cfg;

	modem_mqtt_open_commands_t open;
	modem_mqtt_close_commands_t close;

	modem_mqtt_connect_commands_t connect;
	modem_mqtt_disconnect_commands_t disconnect;

	modem_mqtt_sub_commands_t sub;

	modem_mqtt_pub_commands_t pub;

	uint16_t msgId;

	mqtt_sent_result_t sent_result;

	mqtt_disconnect_urc_reason_t Error_code;

	mqtt_status_t status;

	char mqtt_rx_Topic[100];
	unsigned int mqtt_rx_data_len;
	unsigned char mqtt_rx_data[5000];
	mqtt_rx_state_t mqtt_rx_state;
//  /** will topic, set to NULL if will is not to be used,
//      will_msg, will_qos and will retain are then ignored */
//  const char* will_topic;
//  /** will_msg, see will_topic */
//  const char* will_msg;
//  /** will_qos, see will_topic */
//  u8_t will_qos;
//  /** will_retain, see will_topic */
//  u8_t will_retain;
//#if LWIP_ALTCP && LWIP_ALTCP_TLS
//  /** TLS configuration for secure connections */
//  struct altcp_tls_config *tls_config;
//#endif
}
modem_mqtt_client_t;


typedef struct {
	char	 mac[MAC_LEN+1];
    uint8_t  msgId;
    uint8_t  retries;
    uint32_t timeoutTick;
    bool     ackReceived;
    bool	 used;          // mark if entry is used
    char     payload[150];
} dlEntry_t;

/**************************************************************************//**
 * Macro
 *****************************************************************************/
#define OTA_RESET_TIME			120 //(second)
#define DL_TBL_SIZE				7
#define DL_TIMEOUT_MS			2000	// 2 sec
/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_Modem_MQTT, Modem_MQTT_check_sec, Modem_MQTT_reconnectCount;
extern unsigned int modem_MQTT_publish_sec,modem_MQTT_publish_count,modem_MQTT_publish_success_count,modem_MQTT_publish_success_timer;

extern uint8_t MqttBorker[100];
extern uint16_t MqttBrokerPort;
extern char MqttClientId[100];
extern uint8_t MqttClientUser[100];
extern uint8_t MqttClientPass[100];
extern uint8_t MqttSubTopic[100];
extern uint8_t MqttPubTopic[100] ;
extern uint8_t MqttPubBuf[CIM_MAX_SIZE_OF_MQTT_PAYLOAD] ;

extern modem_mqtt_client_t mqtt[1];

extern osThreadId Modem_MQTT_TaskHandle;
extern unsigned char flag_modem_MQTT_Reconnect;
/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/


void StartModem_MQTTTask(void const * argument);
void Modem_MQTT_start();

void fillMQttClient();
unsigned char modem_make_mqttConnection(void *argument);
unsigned char modem_check_mqtt_status(void *argument);
unsigned char modem_MQTT_disconnect(void *argument);
unsigned char modem_MQTT_publish(void *argument,const char* _topic,const char* _msg,unsigned int _msg_len,unsigned char _QOS,unsigned char retain);
unsigned char modem_MQTT_Check(void *argument);
unsigned char modem_MQTTsubscribe(void *argument,const char* _subTopic,unsigned char _QOS);
void MQTT_PUB_Routine(void);
void modem_sleep_mode(void);
void parse_downlink(char* json_data);
void init_read_cmd_timer(void);
void init_send_timer(void);
#endif /* INC_Modem_MQTT_H_ */
