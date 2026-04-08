/*
 * modem_GPS.c
 *
 *  Created on: Dec 12, 2022
 *      Author: maulin
 */

/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
#include "modem_GPS.h"

#define RTC_SYNC_THRESHOLD_SEC 60  // 1 minute

/**************************************************************************//**
 * Variable
 *****************************************************************************/
modem_gps_loc_command_t gps;
static const uint8_t days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
/**************************************************************************//**
 * Function name 	: check_GPS_data
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void check_GPS_data(void)
{
	if(xSemaphoreTake(modem_SequenceBlockSemaphore, 1200000))
	{
		modemCommandSequence = MODEM_CMD_SEQUEANCE_MQTT_PUBLISH;
		if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
		{
			Modem_AT_Command = LWGSM_CMD_GPS_QGPS_GET;
			lwgsmi_initiate_cmd(Modem_AT_Command,0);
			if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
			{
				if(gps.GNSS_state == 0)
				{
					Modem_AT_Command = LWGSM_CMD_GPS_QGPS_SET;
					lwgsmi_initiate_cmd(Modem_AT_Command,0);
					if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
					{
						xSemaphoreGive(modem_PortBlockSemaphore);
					}
					osDelay(100);
					WriteLog(1, "Set GPS State\r\n", 1);
				}

				Modem_AT_Command = LWGSM_CMD_GPS_QGPSLOC;
				lwgsmi_initiate_cmd(Modem_AT_Command,0);
				if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
				{
					xSemaphoreGive(modem_PortBlockSemaphore);
				}

				if(gps.fix || (HAL_GetTick() - gps.timeout) > 5*MIN_MS)
				{
					Modem_AT_Command = LWGSM_CMD_GPS_QGPSEND;
					lwgsmi_initiate_cmd(Modem_AT_Command,0);
					if(xSemaphoreTake(modem_PortBlockSemaphore, 1000))
					{
						xSemaphoreGive(modem_PortBlockSemaphore);
					}

					if((HAL_GetTick() - gps.timeout) > 5*MIN_MS)	// 5 min timeout
						gps.fix = 4;	// Not fixed
					WriteLog(1, "Turning GPS OFF\r\n", 1);
				}
			}
		}
		Modem_AT_Command = LWGSM_CMD_IDLE;
		modemCommandSequence = MODEM_CMD_SEQUEANCE_NONE;
		xSemaphoreGive(modem_SequenceBlockSemaphore);
	}
}

uint8_t is_leap_year(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

void UTC_To_Local(modem_time_t *local)
{
    int32_t total_minutes;

    // Copy UTC as base
    *local = gps.utc;

    total_minutes = gps.utc.hour * 60 + gps.utc.min;

    int32_t tz_offset = EPROM_General.Cust_Detail.Tz_hours * 60 + EPROM_General.Cust_Detail.Tz_min;
    if (EPROM_General.Cust_Detail.Tz_sign == 1)	// negative offset
        tz_offset = -tz_offset;

    total_minutes += tz_offset;

    // Handle day rollover
    while (total_minutes < 0)
    {
        total_minutes += 1440;
        local->date--;
    }
    while (total_minutes >= 1440)
    {
        total_minutes -= 1440;
        local->date++;
    }

    local->hour = total_minutes / 60;
    local->min  = total_minutes % 60;

    // Handle month/year rollover
    uint8_t dim = days_in_month[local->month - 1];
    if (local->month == 2 && is_leap_year(local->year))
        dim = 29;

    if (local->date == 0)
    {
        local->month--;
        if (local->month == 0)
        {
            local->month = 12;
            local->year--;
        }

        dim = days_in_month[local->month - 1];
        if (local->month == 2 && is_leap_year(local->year))
            dim = 29;

        local->date = dim;
    }
    else if (local->date > dim)
    {
        local->date = 1;
        local->month++;
        if (local->month > 12)
        {
            local->month = 1;
            local->year++;
        }
    }
}


void Check_And_Sync_RTC(modem_time_t *local_time)
{
    int32_t rtc_sec   = gTime.Hours*3600 + gTime.Minutes*60 + gTime.Seconds;
    int32_t local_sec = local_time->hour*3600 + local_time->min*60 + local_time->sec;

    int32_t diff = abs(rtc_sec - local_sec);

    if (diff > 43200)          // > 12 hours
        diff = 86400 - diff;   // wrap-around correction

    if (diff >= RTC_SYNC_THRESHOLD_SEC)
    {
        RTC_TimeTypeDef sTime;
        RTC_DateTypeDef sDate;

        sTime.Hours   = local_time->hour;
        sTime.Minutes = local_time->min;
        sTime.Seconds = local_time->sec;

        sDate.Date  = (uint8_t) local_time->date;
        sDate.Month = (uint8_t) local_time->month;
        sDate.Year  = (uint8_t) local_time->year;
        sDate.WeekDay = 1;	// Not used anywhere, set to a valid value, so that RTC set properly

        HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
        HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    }
}
