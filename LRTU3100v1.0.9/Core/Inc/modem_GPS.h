/*
 * modem_GPS.h
 *
 *  Created on: Dec 12, 2022
 *      Author: maulin
 */

#ifndef INC_MODEM_GPS_H_
#define INC_MODEM_GPS_H_

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
typedef struct
{
	unsigned char hour;
	unsigned char min;
	unsigned char sec;
	unsigned char miliSec;
	unsigned char date;
	unsigned char month;
	unsigned int year;
}
modem_time_t;

typedef struct
{
	modem_time_t utc;
	float latitude;
	float longitude;
	float HDOP;
	float altitude;
	unsigned char fix;
	float COG;
	float speedkm;
	float speedkn;
	unsigned char noOfSatellites;
	unsigned char GNSS_state;
	uint32_t timeout;
}
modem_gps_loc_command_t;



/**************************************************************************//**
 * Macro
 *****************************************************************************/


/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern unsigned int count_Modem_GPS, Modem_GPS_check_sec;
extern modem_gps_loc_command_t gps;
extern osThreadId Modem_MQTT_TaskHandle;

/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/
void check_GPS_data(void);
void UTC_To_Local(modem_time_t *local);
void Check_And_Sync_RTC(modem_time_t *local_time);
uint8_t is_leap_year(uint16_t year);

#endif /* INC_MODEM_GPS_H_ */
