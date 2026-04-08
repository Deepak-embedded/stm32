/*
 * Lora_AT_Types.h
 *
 *  Created on: Sep 16, 2025
 *      Author: Admin
 */

#ifndef INC_LORA_AT_TYPES_H_
#define INC_LORA_AT_TYPES_H_

/**************************************************************************//**
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

typedef enum {
	///LORA AT Commands
	RAK_LORA_DEVICE_AT = 0, 				/*!< Send AT command without any specific parameters. */
	RAK_LORA_DEVICE_ATE, 			/*!< Send AT command "ATE" without any additional parameters. */
	RAK_LORA_DEVICE_ATR, 			/*!< Send AT command "ATR" without any additional parameters. */
	RAK_LORA_DEVICE_ATZ, 			/*!< Send AT command "ATZ" without any additional parameters. */
	RAK_LORA_DEVICE_SN_GET, 			/*!< Send AT command "+SN=?" to retrieve the serial number. */
	RAK_LORA_DEVICE_FIRMVARE_GET, 	/*!< Send AT command "+VER=?" to retrieve firmware version. */
	RAK_LORA_DEVICE_HWMODEL_GET, 	/*!< Send AT command "+HWMODEL=?" to retrieve hardware model. */
	RAK_LORA_DEVICE_HWID_GET, 		/*!< Send AT command "+HWID=?" to retrieve hardware ID. */
	RAK_LORA_DEVICE_BAUD_GET, 		/*!< Send AT command "+BAUD=?" to retrieve baud rate. */
	RAK_LORA_DEVICE_BAUD_SET, 		/*!< Send AT command "+BAUD=?" to retrieve baud rate. */
	RAK_LORA_DEVICE_ATM_MODE, 		/*!< Send AT command "+ATM?" to get the mode. */
	RAK_LORA_DEVICE_NETID_GET, 		/*!< Send AT command "+NETID=?" to retrieve the network ID. */
	RAK_LORA_DEVICE_CLASS_GET, 		/*!< Send AT command "+CLASS=?" to retrieve device class. */
	RAK_LORA_DEVICE_CLASS_SET, 		/*!< Send AT command "+CLASS=" without additional parameters. */
	RAK_LORA_DEVICE_BGW_GET, 		/*!< Send AT command "+BGW=?" to retrieve the device band. */
	RAK_LORA_DEVICE_BAND_GET, 			/*!< Send AT command "+BAND=?" to check gateway response. */
	RAK_LORA_DEVICE_BAND_SET, 			/*!< Send AT command "+BAND=?" to Set gateway response. */
	RAK_LORA_DEVICE_P2P_MODE_SET,	/*!< Send AT command "+NWM=0" to Set P2P mode. */
	RAK_LORA_DEVICE_P2P_MODE_GET,	/*!< Send AT command "+NWM?" to Get P2P mode. */
	RAK_LORA_DEVICE_PRECV_SET,		/*!< Send AT command "+PRECV?" to put in P2P RX mode for a period of time (ms). */
	RAK_LORA_DEVICE_P2P_PARAM_SET,	/*!< Send AT command "+P2P=" to Set P2P mode parameters. */
	RAK_LORA_DEVICE_P2P_PARAM_GET,	/*!< Send AT command "+P2P?" to Get P2P mode parameters. */
	RAK_LORA_DEVICE_PSEND,			/*!< Send AT command "+PSEND=" to send data in P2P mode. */
	RAK_LORA_DEVICE_PENCRYPT_EN,	/*!< Send AT command "+ENCRY=" to enable encryption in P2P mode. */
	RAK_LORA_DEVICE_PENCRYPT_KEY,	/*!< Send AT command "+ENCKEY=" to set encryption key in P2P mode. */
	RAK_LORA_DEVICE_CAD_EN			/*!< Send AT command "+CAD=1" to set P2P Channel Activity Detection. */
} lora_cmd_t;

typedef struct {
    size_t offset;
    size_t size;
} exFlash_stream_t;

#define JANPATCH_STREAM exFlash_stream_t
#define SECTOR_SIZE 4096

typedef struct {
    unsigned char*   buffer;
    size_t           size;
    uint32_t         current_page;
    size_t           current_page_size;
    JANPATCH_STREAM* stream;
    long int         position;
} janpatch_buffer;

typedef struct {
    // fread/fwrite buffers
    janpatch_buffer source_buffer;
    janpatch_buffer patch_buffer;
    janpatch_buffer target_buffer;

    // function signatures
    size_t (*fread)(void*, size_t, size_t, JANPATCH_STREAM*);
    size_t (*fwrite)(const void*, size_t, size_t, JANPATCH_STREAM*);
    int    (*fseek)(JANPATCH_STREAM*, long int, int);
    long   (*ftell)(JANPATCH_STREAM*);

    // progress callback
    void   (*progress)(uint8_t);

    // the combination of the size of both the source + patch files (that's the max. the target file can be)
    long   max_file_size;
} janpatch_ctx;

uint16_t lora_serial_send_fn(const void* data, size_t len);
unsigned char lora_modem_init();
unsigned char lora_modem_network_check();
unsigned char lora_modem_network_reconnect();
unsigned char lora_modem_send_msg(const char *msg);
void lora_send_with_retry(void);
unsigned char lora_modem_update_parameter();
unsigned char lora_modem_receive_msg();
void Lora_start();
void lora_set_P2P_mode(void);
uint8_t Get_Lora_Channel(void);
void BuildStartPayload(void);
void BuildPushChunk(void);
void BuildNackChunk(void);
void BuildVerifyChunk(void);
void CheckTimeouts(void);
void BuildSendDlPayload(char *mac, uint8_t msg_id, char *payload);
void sendTimeSyncCmd(void);
void scanKey(void);
unsigned char lora_modem_send_cont(const char *msg);
char isTimeForDeltaOTA();
char copyTargetToSource(uint32_t FileSize);

#endif /* INC_LORA_AT_TYPES_H_ */
