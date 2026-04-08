/*
 * RTC_Time.h
 *
 *  Created on: Dec 27, 2022
 *      Author: SanketP
 */

#ifndef INC_RTC_TIME_H_
#define INC_RTC_TIME_H_

#include "main.h"
//======================================================================
//Constant
//======================================================================
extern RTC_HandleTypeDef hrtc;
//======================================================================
//RTC START
//======================================================================
#define RTC_WORKING										0 //MAU_EXCEPTION
#define RTC_FAULT_TIME_OUTOFF_RANGE 					1
#define RTC_FAULT_TIME_NOT_UPDATE_ON_BOOT 				2
#define RTC_FAULT_TIME_NOT_UPDATE_IN_TASK	 			3
#define RTC_FAULT_TIME_NOT_UPDATE_EVEN_AFTER_REINIT	 	4
#define _SEC_IN_MINUTE 									60L
#define _SEC_IN_HOUR 									3600L
#define _SEC_IN_DAY 									86400L

#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : DAYS_IN_MONTH[x])
#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

//======================================================================
//Temperature START
//======================================================================

#define TEMPERATURE_WORKING										0 //MAU_EXCEPTION
#define TEMPERATURE_FAULT_I2C_HANG 								1
#define TEMPERATURE_FAULT_VALUE_HANG 							2
#define TEMPERATURE_FAULT_I2C_HANG_EVEN_AFTER_REINIT	 		3
#define TEMPERATURE_FAULT_VALUE_HANG_EVEN_AFTER_REINIT	 		4

//======================================================================
//extern Variable
//======================================================================

//static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
//static unsigned char month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//static unsigned char week_days[7] = {4,5,6,0,1,2,3};
//static const int _DAYS_BEFORE_MONTH[12] ={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

extern unsigned char ntp_date, ntp_month, leap_days, leap_year_ind ;
extern unsigned short temp_days;
extern unsigned int epoch, ntp_year, days_since_epoch, day_of_year;
extern struct tm t1;
extern uint64_t UTC;
extern uint8_t rtctime[], rtcdate[];
extern uint8_t RTC_Fauly_reason;
extern unsigned char RTC_Time_hang_Counter,RTC_Reinit_Count; //MAU_EXCEPTION

extern float_t temperature;
extern uint8_t I2C_hang;
extern unsigned int I2C_Reboot_Count;
extern uint8_t TEMPERATURE_Fauly_reason; //MAU_EXCEPTION
extern unsigned int sameTempReceivedCount;
extern uint8_t Temperature_Reinit_Count;
extern RTC_DateTypeDef gDate;
extern RTC_TimeTypeDef gTime;
//======================================================================
//Function Proto type
//======================================================================
void MX_RTC_Init(void);
void get_time(struct tm t);
void set_time(struct tm t);
void print_time(void);
uint8_t decToBcd(uint8_t val);
uint8_t bcdToDec(uint8_t val);
uint64_t mktime_new(struct tm *tim_p);
struct tm localtime_new (uint64_t _timer);
void RTC_reset();
void RTC_Check_First_Time(void);
void RTC_Check_Working(uint64_t prvUTC); //MAU_EXCEPTION
//mbedtls_time_t RTC_time( mbedtls_time_t* timer );
uint32_t Calculate_SunsetSunrise(unsigned char direction);//, struct tm fine_time);
double DegreesToAngle(double degrees, double minutes, double seconds);
double Deg2Rad(double angle);
double Rad2Deg(double angle);
double FixValue(double value, double min, double max);
void Get_Astro_time(void);
int get_offset_UTC(void);


#endif /* INC_RTC_TIME_H_ */
