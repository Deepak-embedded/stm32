/*
 * ATmodemTypes.h
 *
 *  Created on: Dec 1, 2022
 *      Author: maulin
 */

#ifndef INC_ATMODEMTYPES_H_
#define INC_ATMODEMTYPES_H_

#include "EC200U.h"
/**************************************************************************//**
 * define
 *****************************************************************************/

//#define LWGSM_CMD_CPIN_GET  	1
//#define LWGSM_CMD_CREG_GET  	2
//#define LWGSM_CMD_CSQ_GET		3
//#define LWGSM_CMD_AT			4
//#define LWGSM_CMD_ATI			5

#define MODEM_CMD_SEQUEANCE_NONE			0

#define MODEM_CMD_SEQUEANCE_PHY_CHECK		1+MODEM_CMD_SEQUEANCE_NONE

#define MODEM_CMD_SEQUEANCE_REBOOT		    MODEM_CMD_SEQUEANCE_PHY_CHECK+1

#define MODEM_CMD_SEQUEANCE_NETWORK_INIT    MODEM_CMD_SEQUEANCE_REBOOT+1
#define MODEM_CMD_SEQUEANCE_NETWORK_CHECK	MODEM_CMD_SEQUEANCE_NETWORK_INIT+1

#define MODEM_CMD_SEQUEANCE_MQTT_INIT		MODEM_CMD_SEQUEANCE_NETWORK_CHECK+1
#define MODEM_CMD_SEQUEANCE_MQTT_CHECK    	MODEM_CMD_SEQUEANCE_MQTT_INIT+1
#define MODEM_CMD_SEQUEANCE_MQTT_PUBLISH    MODEM_CMD_SEQUEANCE_MQTT_CHECK+1
#define MODEM_CMD_SEQUEANCE_MQTT_SUB    	MODEM_CMD_SEQUEANCE_MQTT_PUBLISH+1
#define MODEM_CMD_SEQUEANCE_MQTT_UNSUB    	MODEM_CMD_SEQUEANCE_MQTT_SUB+1
#define MODEM_CMD_SEQUEANCE_MQTT_DISCONNECT MODEM_CMD_SEQUEANCE_MQTT_UNSUB+1

#define MODEM_CMD_SEQUEANCE_BLE_INT			MODEM_CMD_SEQUEANCE_MQTT_DISCONNECT+1
#define MODEM_CMD_SEQUEANCE_BLE_UPDATE		MODEM_CMD_SEQUEANCE_BLE_INT+1
#define MODEM_CMD_SEQUEANCE_BLE_CHECK    	MODEM_CMD_SEQUEANCE_BLE_UPDATE+1

#define MODEM_CMD_SEQUEANCE_GPS			    MODEM_CMD_SEQUEANCE_BLE_CHECK+1

#define MODEM_CMD_MODEM_INFO			    MODEM_CMD_SEQUEANCE_GPS+1
#define MODEM_CMD_MODEM_RESET			    MODEM_CMD_MODEM_INFO+1

#define MODEM_CMD_SEQUENCE_LORA_INIT		MODEM_CMD_MODEM_RESET+1
#define MODEM_CMD_SEQUENCE_LORA_CHECK		MODEM_CMD_SEQUENCE_LORA_INIT+1
#define MODEM_CMD_SEQUENCE_LORA_SEND		MODEM_CMD_SEQUENCE_LORA_CHECK+1
#define MODEM_CMD_SEQUENCE_LORA_RECEIVE		MODEM_CMD_SEQUENCE_LORA_SEND+1
#define MODEM_CMD_SEQUENCE_LORA_UPDATE		MODEM_CMD_SEQUENCE_LORA_RECEIVE+1
#define MODEM_CMD_SEQUENCE_LORA_CONNECT		MODEM_CMD_SEQUENCE_LORA_UPDATE+1
#define MODEM_CMD_SEQUEANCE_SDCARD		    MODEM_CMD_SEQUENCE_LORA_CONNECT+1

#define CRLF                       "\r\n"
#define CRLF_LEN                   2

/**************************************************************************//**
 * Macro
 *****************************************************************************/

#define LWGSM_CHARISNUM(x)    ((x) >= '0' && (x) <= '9')

#define LWGSM_CHARTONUM(x)    ((x) - '0')

#define LWGSM_CHARISHEXNUM(x) (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))

#define LWGSM_CHARHEXTONUM(x)                                                                                          \
    (((x) >= '0' && (x) <= '9')                                                                                        \
         ? ((x) - '0')                                                                                                 \
         : (((x) >= 'a' && (x) <= 'f') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'F') ? ((x) - 'A' + 10) : 0)))

#define LWGSM_ISVALIDASCII(x) (((x) >= 32 && (x) <= 126) || (x) == '\r' || (x) == '\n')

/* Send data over AT port */

/*#define AT_PORT_SEND_WITH_FLUSH(d, l)                                                                                  \
    do {                                                                                                               \
        AT_PORT_SEND((d), (l));                                                                                        \
        AT_PORT_SEND_FLUSH();                                                                                          \
    } while (0)*/

/* Beginning and end of every AT command */
/*#define AT_PORT_SEND_BEGIN_AT()                                                                                        \
    do {                                                                                                               \
        AT_PORT_SEND_CONST_STR("AT");                                                                                  \
    } while (0)
#define AT_PORT_SEND_END_AT()                                                                                          \
    do {                                                                                                               \
        AT_PORT_SEND(CRLF, CRLF_LEN);                                                                                  \
        AT_PORT_SEND(NULL, 0);                                                                                         \
    } while (0)*/

/* Send special characters over AT port with condition */
/*#define AT_PORT_SEND_QUOTE_COND(q)                                                                                     \
    do {                                                                                                               \
        if ((q)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR("\"");                                                                              \
        }                                                                                                              \
    } while (0)
#define AT_PORT_SEND_COMMA_COND(c)                                                                                     \
    do {                                                                                                               \
        if ((c)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR(",");                                                                               \
        }                                                                                                              \
    } while (0)
#define AT_PORT_SEND_EQUAL_COND(e)                                                                                     \
    do {                                                                                                               \
        if ((e)) {                                                                                                     \
            AT_PORT_SEND_CONST_STR("=");                                                                               \
        }                                                                                                              \
    } while (0)*/

/* Send special characters */
//#define AT_PORT_SEND_CTRL_Z() AT_PORT_SEND_STR("\x1A")
//#define AT_PORT_SEND_ESC()    AT_PORT_SEND_STR("\x1B")

#define CMD_IS_CUR(c)              (Modem_AT_Command == (c))
//#define CMD_IS_DEF(c)              (lwgsm.msg != NULL && lwgsm.msg->cmd_def == (c))
//#define CMD_GET_CUR()              ((lwgsm_cmd_t)(((lwgsm.msg != NULL) ? lwgsm.msg->cmd : LWGSM_CMD_IDLE)))
//#define CMD_GET_DEF()              ((lwgsm_cmd_t)(((lwgsm.msg != NULL) ? lwgsm.msg->cmd_def : LWGSM_CMD_IDLE)))
/**************************************************************************//**
 * typedef
 *****************************************************************************/
/**
 * \ingroup         LWGSM_TYPES
 * \brief           Result enumeration used across application functions
 */
typedef enum {
    lwgsmOK = 0,       /*!< Function returned OK */
    lwgsmOKIGNOREMORE, /*!< Function succedded, should continue as \ref lwgsmOK
                                                        but ignore sending more data.
                                                        This result is possible on connection data receive callback */
    lwgsmERR,          /*!< Generic error */
    lwgsmERRPAR,       /*!< Wrong parameters on function call */
    lwgsmERRMEM,       /*!< Memory error occurred */
    lwgsmTIMEOUT,      /*!< Timeout occurred on command */
    lwgsmCONT,         /*!< There is still some command to be processed in current command */
    lwgsmCLOSED,       /*!< Connection just closed */
    lwgsmINPROG,       /*!< Operation is in progress */

    lwgsmERRNOTENABLED,       /*!< Feature not enabled error */
    lwgsmERRNOIP,             /*!< Station does not have IP address */
    lwgsmERRNOFREECONN,       /*!< There is no free connection available to start */
    lwgsmERRCONNTIMEOUT,      /*!< Timeout received when connection to access point */
    lwgsmERRPASS,             /*!< Invalid password for access point */
    lwgsmERRNOAP,             /*!< No access point found with specific SSID and MAC address */
    lwgsmERRCONNFAIL,         /*!< Connection failed to access point */
    lwgsmERRWIFINOTCONNECTED, /*!< Wifi not connected to access point */
    lwgsmERRNODEVICE,         /*!< Device is not present */
    lwgsmERRBLOCKING,         /*!< Blocking mode command is not allowed */
} lwgsmr_t;

/**
 * \brief           List of possible messages
 */
typedef enum {
    LWGSM_CMD_IDLE = 0, /*!< IDLE mode */

    /* Basic AT commands */
    LWGSM_CMD_RESET,                  /*!< Reset device */
	LWGSM_CMD_RESET_QRESET,                  /*!< Reset device by Qreset */
    LWGSM_CMD_RESET_DEVICE_FIRST_CMD, /*!< Reset device first driver specific command */
    LWGSM_CMD_ATE0,                   /*!< Disable ECHO mode on AT commands */
    LWGSM_CMD_ATE1,                   /*!< Enable ECHO mode on AT commands */
    LWGSM_CMD_GSLP,                   /*!< Set GSM to sleep mode */
    LWGSM_CMD_RESTORE,                /*!< Restore GSM internal settings to default values */
    LWGSM_CMD_UART,

    LWGSM_CMD_CGACT_SET_0,
    LWGSM_CMD_CGACT_SET_1,
	LWGSM_CMD_CGACT_GET,
    LWGSM_CMD_CGATT_SET_0,
    LWGSM_CMD_CGATT_SET_1,
	LWGSM_CMD_CGATT_GET,
	LWGSM_CMD_CGDCONT,
	LWGSM_CMD_CGPADDR_GET,
	LWGSM_CMD_QPWRBACKOFF, // Add command as per Quectel suggetion for M2M card || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-19
	LWGSM_CMD_NETWORK_ATTACH, /*!< Attach to a network */
    LWGSM_CMD_NETWORK_DETACH, /*!< Detach from network */

    LWGSM_CMD_CIPMUX_SET,
    LWGSM_CMD_CIPRXGET_SET,
    LWGSM_CMD_CSTT_SET,

    /* AT commands according to the V.25TER */
    LWGSM_CMD_CALL_ENABLE,
    LWGSM_CMD_A,       /*!< Re-issues the Last Command Given */
    LWGSM_CMD_ATA,     /*!< Answer an Incoming Call */
    LWGSM_CMD_ATD,     /*!< Mobile Originated Call to Dial A Number */
    LWGSM_CMD_ATD_N,   /*!< Originate Call to Phone Number in Current Memory: ATD<n> */
    LWGSM_CMD_ATD_STR, /*!< Originate Call to Phone Number in Memory Which Corresponds to Field "str": ATD>str */
    LWGSM_CMD_ATDL,    /*!< Redial Last Telephone Number Used */
    LWGSM_CMD_ATE,     /*!< Set Command Echo Mode */
    LWGSM_CMD_ATH,     /*!< Disconnect Existing */
    LWGSM_CMD_ATI,     /*!< Display Product Identification Information */
    LWGSM_CMD_ATL,     /*!< Set Monitor speaker */
    LWGSM_CMD_ATM,     /*!< Set Monitor Speaker Mode */
    LWGSM_CMD_PPP,     /*!< Switch from Data Mode or PPP Online Mode to Command Mode, "+++" originally */
    LWGSM_CMD_ATO,     /*!< Switch from Command Mode to Data Mode */
    LWGSM_CMD_ATP,     /*!< Select Pulse Dialing */
    LWGSM_CMD_ATQ,     /*!< Set Result Code Presentation Mode */
    LWGSM_CMD_ATS0,    /*!< Set Number of Rings before Automatically Answering the Call */
    LWGSM_CMD_ATS3,    /*!< Set Command Line Termination Character */
    LWGSM_CMD_ATS4,    /*!< Set Response Formatting Character */
    LWGSM_CMD_ATS5,    /*!< Set Command Line Editing Character */
    LWGSM_CMD_ATS6,    /*!< Pause Before Blind */
    LWGSM_CMD_ATS7,    /*!< Set Number of Seconds to Wait for Connection Completion */
    LWGSM_CMD_ATS8, /*!< Set Number of Seconds to Wait for Comma Dial Modifier Encountered in Dial String of D Command */
    LWGSM_CMD_ATS10, /*!< Set Disconnect Delay after Indicating the Absence of Data Carrier */
    LWGSM_CMD_ATT,   /*!< Select Tone Dialing */
    LWGSM_CMD_ATV,   /*!< TA Response Format */
    LWGSM_CMD_ATX,   /*!< Set CONNECT Result Code Format and Monitor Call Progress */
    LWGSM_CMD_ATZ,   /*!< Reset Default Configuration */
    LWGSM_CMD_AT_C,  /*!< Set DCD Function Mode, AT&C */
    LWGSM_CMD_AT_D,  /*!< Set DTR Function, AT&D */
    LWGSM_CMD_AT_F,  /*!< Factory Defined Configuration, AT&F */
    LWGSM_CMD_AT_V,  /*!< Display Current Configuration, AT&V */
    LWGSM_CMD_AT_W,  /*!< Store Active Profile, AT&W */
    LWGSM_CMD_GCAP,  /*!< Request Complete TA Capabilities List */
    LWGSM_CMD_GMI,   /*!< Request Manufacturer Identification */
    LWGSM_CMD_GMM,   /*!< Request TA Model Identification */
    LWGSM_CMD_GMR,   /*!< Request TA Revision Identification of Software Release */
    LWGSM_CMD_GOI,   /*!< Request Global Object Identification */
    LWGSM_CMD_GSN,   /*!< Request TA Serial Number Identification (IMEI) */
    LWGSM_CMD_ICF,   /*!< Set TE-TA Control Character Framing */
    LWGSM_CMD_IFC,   /*!< Set TE-TA Local Data Flow Control */
    LWGSM_CMD_IPR,   /*!< Set TE-TA Fixed Local Rate */
    LWGSM_CMD_HVOIC, /*!< Disconnect Voice Call Only */

    /* AT commands according to 3GPP TS 27.007 */
    LWGSM_CMD_COPS_SET,     /*!< Set operator */
    LWGSM_CMD_COPS_GET,     /*!< Get current operator */
    LWGSM_CMD_COPS_GET_OPT, /*!< Get a list of available operators */
    LWGSM_CMD_CPAS,         /*!< Phone Activity Status */
    LWGSM_CMD_CGMI_GET,     /*!< Request Manufacturer Identification */
    LWGSM_CMD_CGMM_GET,     /*!< Request Model Identification */
    LWGSM_CMD_CGMR_GET,     /*!< Request TA Revision Identification of Software Release */
    LWGSM_CMD_CGSN_GET,     /*!< Request Product Serial Number Identification (Identical with +GSN) */

    LWGSM_CMD_CLCC_SET, /*!< List Current Calls of ME */
    LWGSM_CMD_CLCK,     /*!< Facility Lock */

    LWGSM_CMD_CACM,     /*!< Accumulated Call Meter (ACM) Reset or Query */
    LWGSM_CMD_CAMM,     /*!< Accumulated Call Meter Maximum (ACM max) Set or Query */
    LWGSM_CMD_CAOC,     /*!< Advice of Charge */
    LWGSM_CMD_CBST,     /*!< Select Bearer Service Type */
    LWGSM_CMD_CCFC,     /*!< Call Forwarding Number and Conditions Control */
    LWGSM_CMD_CCWA,     /*!< Call Waiting Control */
    LWGSM_CMD_CEER,     /*!< Extended Error Report  */
    LWGSM_CMD_CSCS,     /*!< Select TE Character Set */
    LWGSM_CMD_CSTA,     /*!< Select Type of Address */
    LWGSM_CMD_CHLD,     /*!< Call Hold and Multiparty */
    LWGSM_CMD_CIMI,     /*!< Request International Mobile Subscriber Identity */
    LWGSM_CMD_CLIP,     /*!< Calling Line Identification Presentation */
    LWGSM_CMD_CLIR,     /*!< Calling Line Identification Restriction */
    LWGSM_CMD_CMEE_SET, /*!< Report Mobile Equipment Error */
    LWGSM_CMD_COLP,     /*!< Connected Line Identification Presentation */

    LWGSM_CMD_PHONEBOOK_ENABLE,
    LWGSM_CMD_CPBF,         /*!< Find Phonebook Entries */
    LWGSM_CMD_CPBR,         /*!< Read Current Phonebook Entries  */
    LWGSM_CMD_CPBS_SET,     /*!< Select Phonebook Memory Storage */
    LWGSM_CMD_CPBS_GET,     /*!< Get current Phonebook Memory Storage */
    LWGSM_CMD_CPBS_GET_OPT, /*!< Get available Phonebook Memory Storages */
    LWGSM_CMD_CPBW_SET,     /*!< Write Phonebook Entry */
    LWGSM_CMD_CPBW_GET_OPT, /*!< Get options for write Phonebook Entry */

    LWGSM_CMD_SIM_PROCESS_BASIC_CMDS, /*!< Command setup, executed when SIM is in READY state */
    LWGSM_CMD_CPIN_SET,               /*!< Enter PIN */
    LWGSM_CMD_CPIN_GET,               /*!< Read current SIM status */
    LWGSM_CMD_CPIN_ADD,               /*!< Add new PIN to SIM if pin is not set */
    LWGSM_CMD_CPIN_CHANGE,            /*!< Change already active SIM */
    LWGSM_CMD_CPIN_REMOVE,            /*!< Remove current PIN */
    LWGSM_CMD_CPUK_SET,               /*!< Enter PUK and set new PIN */

    LWGSM_CMD_CSQ_GET,  /*!< Signal Quality Report */
    LWGSM_CMD_CFUN_SET, /*!< Set Phone Functionality */
    LWGSM_CMD_CFUN_GET, /*!< Get Phone Functionality */
    LWGSM_CMD_CREG_SET, /*!< Network Registration set output */
    LWGSM_CMD_CREG_GET, /*!< Get current network registration status */
	LWGSM_CMD_CGREG_GET,/*!< Get current data network registration status */
    LWGSM_CMD_CBC,      /*!< Battery Charge */
    LWGSM_CMD_CNUM,     /*!< Subscriber Number */

    LWGSM_CMD_CPWD,     /*!< Change Password */
    LWGSM_CMD_CR,       /*!< Service Reporting Control */
    LWGSM_CMD_CRC,      /*!< Set Cellular Result Codes for Incoming Call Indication */
    LWGSM_CMD_CRLP,     /*!< Select Radio Link Protocol Parameters  */
    LWGSM_CMD_CRSM,     /*!< Restricted SIM Access */
    LWGSM_CMD_VTD,      /*!< Tone Duration */
    LWGSM_CMD_VTS,      /*!< DTMF and Tone Generation */
    LWGSM_CMD_CMUX,     /*!< Multiplexer Control */
    LWGSM_CMD_CPOL,     /*!< Preferred Operator List */
    LWGSM_CMD_COPN,     /*!< Read Operator Names */
    LWGSM_CMD_CCLK,     /*!< Clock */
    LWGSM_CMD_CSIM,     /*!< Generic SIM Access */
    LWGSM_CMD_CALM,     /*!< Alert Sound Mode */
    LWGSM_CMD_CALS,     /*!< Alert Sound Select */
    LWGSM_CMD_CRSL,     /*!< Ringer Sound Level */
    LWGSM_CMD_CLVL,     /*!< Loud Speaker Volume Level */
    LWGSM_CMD_CMUT,     /*!< Mute Control */
    LWGSM_CMD_CPUC,     /*!< Price Per Unit and Currency Table */
    LWGSM_CMD_CCWE,     /*!< Call Meter Maximum Event */
    LWGSM_CMD_CUSD_SET, /*!< Unstructured Supplementary Service Data, Set command */
    LWGSM_CMD_CUSD_GET, /*!< Unstructured Supplementary Service Data, Get command */
    LWGSM_CMD_CUSD,     /*!< Unstructured Supplementary Service Data, Execute command */
    LWGSM_CMD_CSSN,     /*!< Supplementary Services Notification */

    LWGSM_CMD_CIPMUX,     /*!< Start Up Multi-IP Connection */
    LWGSM_CMD_CIPSTART,   /*!< Start Up TCP or UDP Connection */
    LWGSM_CMD_CIPSEND,    /*!< Send Data Through TCP or UDP Connection */
    LWGSM_CMD_CIPQSEND,   /*!< Select Data Transmitting Mode */
    LWGSM_CMD_CIPACK,     /*!< Query Previous Connection Data Transmitting State */
    LWGSM_CMD_CIPCLOSE,   /*!< Close TCP or UDP Connection */
    LWGSM_CMD_CIPSHUT,    /*!< Deactivate GPRS PDP Context */
    LWGSM_CMD_CLPORT,     /*!< Set Local Port */
    LWGSM_CMD_CSTT,       /*!< Start Task and Set APN, username, password */
    LWGSM_CMD_CIICR,      /*!< Bring Up Wireless Connection with GPRS or CSD */
    LWGSM_CMD_CIFSR,      /*!< Get Local IP Address */
    LWGSM_CMD_CIPSTATUS,  /*!< Query Current Connection Status */
    LWGSM_CMD_CDNSCFG,    /*!< Configure Domain Name Server */
    LWGSM_CMD_CDNSGIP,    /*!< Query the IP Address of Given Domain Name */
    LWGSM_CMD_CIPHEAD,    /*!< Add an IP Head at the Beginning of a Package Received */
    LWGSM_CMD_CIPATS,     /*!< Set Auto Sending Timer */
    LWGSM_CMD_CIPSPRT,    /*!< Set Prompt of greater than sign When Module Sends Data */
    LWGSM_CMD_CIPSERVER,  /*!< Configure Module as Server */
    LWGSM_CMD_CIPCSGP,    /*!< Set CSD or GPRS for Connection Mode */
    LWGSM_CMD_CIPSRIP,    /*!< Show Remote IP Address and Port When Received Data */
    LWGSM_CMD_CIPDPDP,    /*!< Set Whether to Check State of GPRS Network Timing */
    LWGSM_CMD_CIPMODE,    /*!< Select TCPIP Application Mode */
    LWGSM_CMD_CIPCCFG,    /*!< Configure Transparent Transfer Mode */
    LWGSM_CMD_CIPSHOWTP,  /*!< Display Transfer Protocol in IP Head When Received Data */
    LWGSM_CMD_CIPUDPMODE, /*!< UDP Extended Mode */
    LWGSM_CMD_CIPRXGET,   /*!< Get Data from Network Manually */
    LWGSM_CMD_CIPSCONT,   /*!< Save TCPIP Application Context */
    LWGSM_CMD_CIPRDTIMER, /*!< Set Remote Delay Timer */
    LWGSM_CMD_CIPSGTXT,   /*!< Select GPRS PDP context */
    LWGSM_CMD_CIPTKA,     /*!< Set TCP Keepalive Parameters */
    LWGSM_CMD_CIPSSL,     /*!< Connection SSL function */

    LWGSM_CMD_SMS_ENABLE,
    LWGSM_CMD_CMGD,         /*!< Delete SMS Message */
    LWGSM_CMD_CMGF,         /*!< Select SMS Message Format */
    LWGSM_CMD_CMGL,         /*!< List SMS Messages from Preferred Store */
    LWGSM_CMD_CMGR,         /*!< Read SMS Message */
    LWGSM_CMD_CMGS,         /*!< Send SMS Message */
    LWGSM_CMD_CMGW,         /*!< Write SMS Message to Memory */
    LWGSM_CMD_CMSS,         /*!< Send SMS Message from Storage */
    LWGSM_CMD_CMGDA,        /*!< MASS SMS delete */
    LWGSM_CMD_CNMI,         /*!< New SMS Message Indications */
    LWGSM_CMD_CPMS_SET,     /*!< Set preferred SMS Message Storage */
    LWGSM_CMD_CPMS_GET,     /*!< Get preferred SMS Message Storage */
    LWGSM_CMD_CPMS_GET_OPT, /*!< Get optional SMS message storages */
    LWGSM_CMD_CRES,         /*!< Restore SMS Settings */
    LWGSM_CMD_CSAS,         /*!< Save SMS Settings */
    LWGSM_CMD_CSCA,         /*!< SMS Service Center Address */
    LWGSM_CMD_CSCB,         /*!< Select Cell Broadcast SMS Messages */
    LWGSM_CMD_CSDH,         /*!< Show SMS Text Mode Parameters */
    LWGSM_CMD_CSMP,         /*!< Set SMS Text Mode Parameters */
    LWGSM_CMD_CSMS,         /*!< Select Message Service */

	LWGSM_CMD_MQTT_QMTCFG_RECV_MODE_SET, 	/*!< set MQTT Receive mode */
	LWGSM_CMD_MQTT_QMTOPEN_SET, 				/*!< set MQTT OPEN Link */
	LWGSM_CMD_MQTT_QMTCLOSE, 				/*!< Close a link for MQTT Client*/
	LWGSM_CMD_MQTT_QMTOPEN_GET, 			/*!< get MQTT OPEN Link status*/
	LWGSM_CMD_MQTT_QMTCONN_SET, 			/*!< set MQTT make Connection with broker */
	LWGSM_CMD_MQTT_QMTCONN_GET, 			/*!< get MQTT make Connection with broker */
	LWGSM_CMD_MQTT_QMTDISC, 				/*!< Disconnect a Client from MQTT Server*/
	LWGSM_CMD_MQTT_QMTSUB_SET, 				/*!< set MQTT subscribe */
	LWGSM_CMD_MQTT_QMTPUBEX, 				/*!< MQTT Publish */
	LWGSM_CMD_MQTT_PUB_MSG,
	LWGSM_CMD_MQTT_QMTUNS, 					/*!< set MQTT unsubscribe */

	LWGSM_CMD_GPS_QGPS_GET,					/*!< Get GPS Power On Or Not */
	LWGSM_CMD_GPS_QGPS_SET,					/*!< GPS Power On */
	LWGSM_CMD_GPS_QGPSEND,					/*!< GPS Power Down */
	LWGSM_CMD_GPS_QGPSLOC,					/*!< get gps location */
	LWGSM_CMD_GPS_QGPSRMC,

	LWGSM_CMD_QSCLK,						/*!< set modem sleep mode*/
	LWGSM_CMD_SDCARD_QFLST,					/*!< List SD Card Files */
    LWGSM_CMD_END, /*!< Last CMD entry */
} lwgsm_cmd_t;

/**
 * \ingroup         LWGSM_UNICODE
 * \brief           Unicode support structure
 */
typedef struct {
    uint8_t ch[4]; /*!< UTF-8 max characters */
    uint8_t t;     /*!< Total expected length in UTF-8 sequence */
    uint8_t r;     /*!< Remaining bytes in UTF-8 sequence */
    lwgsmr_t res;  /*!< Current result of processing */
} lwgsm_unicode_t;

/**
 * \ingroup         LWGSM_NETWORK
 * \brief           Network Registration status
 */
typedef enum {
    LWGSM_NETWORK_REG_STATUS_SIM_ERR = 0x00,            /*!< SIM card error */
    LWGSM_NETWORK_REG_STATUS_CONNECTED = 0x01,          /*!< Device is connected to network */
    LWGSM_NETWORK_REG_STATUS_SEARCHING = 0x02,          /*!< Network search is in progress */
    LWGSM_NETWORK_REG_STATUS_DENIED = 0x03,             /*!< Registration denied */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING = 0x05,  /*!< Device is connected and is roaming */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_SMS_ONLY = 0x06, /*!< Device is connected to home network in SMS-only mode */
    LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING_SMS_ONLY = 0x07 /*!< Device is roaming in SMS-only mode */
} lwgsm_network_reg_status_t;

/**
 * \ingroup         LWGSM_TYPES
 * \brief           MAC address
 */
typedef struct {
    uint8_t mac[6]; /*!< MAC address */
} lwgsm_mac_t;

/**
 * \ingroup         LWGSM_TYPES
 * \brief           IP structure
 */
typedef struct {
    uint8_t ip[4]; /*!< IPv4 address */
} lwgsm_ip_t;

/**
 * \ingroup         LWGSM_TYPES
 * \brief           Port variable
 */
typedef uint16_t lwgsm_port_t;

/**
 * \ingroup         LWGSM_SIM
 * \brief           SIM state
 */
typedef enum {
    LWGSM_SIM_STATE_NOT_INSERTED, /*!< SIM is not inserted in socket */
    LWGSM_SIM_STATE_READY,        /*!< SIM is ready for operations */
    LWGSM_SIM_STATE_NOT_READY,    /*!< SIM is not ready for any operation */
    LWGSM_SIM_STATE_PIN,          /*!< SIM is waiting for SIM to be given */
    LWGSM_SIM_STATE_PUK,          /*!< SIM is waiting for PUT to be given */
    LWGSM_SIM_STATE_PH_PIN,
    LWGSM_SIM_STATE_PH_PUK,
    LWGSM_SIM_STATE_END,
} lwgsm_sim_state_t;



#endif /* INC_ATMODEMTYPES_H_ */
