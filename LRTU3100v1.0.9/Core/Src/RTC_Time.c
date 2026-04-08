/*
 * RTC_Time.c
 *
 *  Created on: Dec 27, 2022
 *      Author: SanketP
 */

#include "RTC_Time.h"
#include "main.h"
#include <time.h>
//======================================================================
//Variable
//======================================================================

//======================================================================
//RTC START
//======================================================================

//static const int DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static unsigned char month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
static unsigned char week_days[7] = {4,5,6,0,1,2,3};
unsigned char ntp_date, ntp_month, leap_days, leap_year_ind ;
unsigned short temp_days;
unsigned int epoch, ntp_year, days_since_epoch, day_of_year;
static const int _DAYS_BEFORE_MONTH[12] ={0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
extern struct tm t1;
uint64_t UTC=1675425494+19800;//1672201570;//1654169962;
//uint8_t rtctime[20], rtcdate[20];
uint8_t RTC_Fauly_reason;
unsigned char RTC_Time_hang_Counter=0,RTC_Reinit_Count=0; //MAU_EXCEPTION
extern uint8_t rtctime[20], rtcdate[20];
//UBaseType_t uxHighWaterMark[12];
RTC_DateTypeDef gDate;
RTC_TimeTypeDef gTime;
//****RTC****//
struct tm t1,*time_sunset,*time_sunrise;
uint32_t Get_UTCsunset,Get_UTCsunrise;
bool flag_timezone_sign;
//======================================================================
//RTC END
//======================================================================
/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

 // RTC_TimeTypeDef sTime = {0};
 // RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;

  if(!(READ_BIT(RTC->ISR,RTC_ISR_INITS)))
  {
	  if (HAL_RTC_Init(&hrtc) != HAL_OK)
	  {
		Error_Handler();
	  }
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
//  sTime.Hours = 0x0;
//  sTime.Minutes = 0x0;
//  sTime.Seconds = 0x0;
//  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
//  sDate.Month = RTC_MONTH_JANUARY;
//  sDate.Date = 0x1;
//  sDate.Year = 0x0;
//  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
//  {
//    Error_Handler();
//  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.AlarmTime.SubSeconds = 0x0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 0x1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**************************************************************************//**
 * Function name 	: print_time
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
void print_time(void)  //////////
{
	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);

	sprintf((char*)rtcdate, "[%02d-%02d-%02d ",gDate.Date, gDate.Month, 2000 + gDate.Year);
	sprintf((char*)rtctime, "[%02d:%02d:%02d]\r\n",gTime.Hours, gTime.Minutes, gTime.Seconds);

	 // Send the formatted date and time over UART8
//	WriteLog(1, rtcdate, 1);
//	WriteLog(1, rtctime, 1);
}
/**************************************************************************//**
 * Function name 	: get_time
 * arguments		: 1)
 * return 		 	:
 * Note				: ms utc
 *****************************************************************************/
void get_time(struct tm t)
{
	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

	t.tm_hour = bcdToDec(gTime.Hours);
	t.tm_min = bcdToDec(gTime.Minutes);
	t.tm_sec = bcdToDec(gTime.Seconds);
	t.tm_wday = bcdToDec(gDate.WeekDay) - 1;
	t.tm_mon = bcdToDec(gDate.Month) - 1;
	t.tm_mday = bcdToDec(gDate.Date);
	t.tm_year = bcdToDec(gDate.Year) + 2000 - 1900;
	//strcpy(t1, t, sizeof(struct tm));
	t1 = t;
	UTC = (uint64_t)mktime_new(&t);
	//UTC = (uint64_t)mktime(&t);
	UTC *= 1000;

	//sprintf((char*)rtcdate, "\r\n%02d-%02d-%02d ",gDate.Date, gDate.Month, 2000 + gDate.Year);
	//sprintf((char*)rtctime, "%02d:%02d:%02d",gTime.Hours, gTime.Minutes, gTime.Seconds);
}
/**************************************************************************//**
 * Function name 	: set_time
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
void set_time(struct tm t)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;

	//time_t rawtime = UTC;

	//t = *localtime(&rawtime);

	t=localtime_new(UTC);

	sTime.Hours = decToBcd(t.tm_hour);

	sTime.Minutes = decToBcd(t.tm_min);

	sTime.Seconds = decToBcd(t.tm_sec);

	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;

	//sDate.WeekDay = decToBcd(t.tm_wday) + 1;
	sDate.WeekDay = decToBcd(t.tm_wday);


	//sDate.Month = decToBcd(t.tm_mon) + 1;
	sDate.Month = decToBcd(t.tm_mon);
	sDate.Date = decToBcd(t.tm_mday);

	sDate.Year = decToBcd(t.tm_year%100);

	t.tm_year -= 1900;
	t.tm_mon -= 1;
	t.tm_wday -= 1;
	t.tm_isdst = 0;

	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
	  Error_Handler();
	}

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
	  Error_Handler();
	}

//	UTC = (uint64_t)mktime_new(&t);
//	UTC *= 1000;

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}

/**************************************************************************//**
 * Function name 	: mktime_new
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
uint64_t mktime_new(struct tm *tim_p)
{
   time_t tim = 0;
   long days = 0;
   int year;//, isdst=0;

//   __tzinfo_type *tz = __gettzinfo ();

   /* validate structure */
//   validate_structure (tim_p);

   /* compute hours, minutes, seconds */
   tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) +
     (tim_p->tm_hour * _SEC_IN_HOUR);

   /* compute days in year */
   days += tim_p->tm_mday - 1;
   days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
   if (tim_p->tm_mon > 1 && _DAYS_IN_YEAR (tim_p->tm_year) == 366)
     days++;

   /* compute day of the year */
   tim_p->tm_yday = days;

   if (tim_p->tm_year > 10000 || tim_p->tm_year < -10000)
       return (time_t) -1;

   /* compute days in other years */
   if ((year = tim_p->tm_year) > 70)
     {
       for (year = 70; year < tim_p->tm_year; year++)
         days += _DAYS_IN_YEAR (year);
     }
   else if (year < 70)
     {
       for (year = 69; year > tim_p->tm_year; year--)
         days -= _DAYS_IN_YEAR (year);
       days -= _DAYS_IN_YEAR (year);
     }

   /* compute total seconds */
   tim += (time_t)days * _SEC_IN_DAY;

//   TZ_LOCK;

//   _tzset_unlocked ();

//   if (_daylight)
//     {
//       int tm_isdst;
//       int y = tim_p->tm_year + YEAR_BASE;
//       /* Convert user positive into 1 */
//       tm_isdst = tim_p->tm_isdst > 0  ?  1 : tim_p->tm_isdst;
//       isdst = tm_isdst;
//
//       if (y == tz->__tzyear || __tzcalc_limits (y))
//         {
//           /* calculate start of dst in dst local time and
//              start of std in both std local time and dst local time */
//           time_t startdst_dst = tz->__tzrule[0].change
//             - (time_t) tz->__tzrule[1].offset;
//           time_t startstd_dst = tz->__tzrule[1].change
//             - (time_t) tz->__tzrule[1].offset;
//           time_t startstd_std = tz->__tzrule[1].change
//             - (time_t) tz->__tzrule[0].offset;
//           /* if the time is in the overlap between dst and std local times */
//           if (tim >= startstd_std && tim < startstd_dst)
//             ; /* we let user decide or leave as -1 */
//           else
//             {
//               isdst = (tz->__tznorth
//                        ? (tim >= startdst_dst && tim < startstd_std)
//                        : (tim >= startdst_dst || tim < startstd_std));
//               /* if user committed and was wrong, perform correction, but not
//                * if the user has given a negative value (which
//                * asks mktime() to determine if DST is in effect or not) */
//               if (tm_isdst >= 0  &&  (isdst ^ tm_isdst) == 1)
//                 {
//                   /* we either subtract or add the difference between
//                      time zone offsets, depending on which way the user got it
//                      wrong. The diff is typically one hour, or 3600 seconds,
//                      and should fit in a 16-bit int, even though offset
//                      is a long to accomodate 12 hours. */
//                   int diff = (int) (tz->__tzrule[0].offset
//                                     - tz->__tzrule[1].offset);
//                   if (!isdst)
//                     diff = -diff;
//                   tim_p->tm_sec += diff;
//                   tim += diff;  /* we also need to correct our current time calculation */
//                   int mday = tim_p->tm_mday;
//                   validate_structure (tim_p);
//                   mday = tim_p->tm_mday - mday;
//                   /* roll over occurred */
//                   if (mday) {
//                     /* compensate for month roll overs */
//                     if (mday > 1)
//                           mday = -1;
//                     else if (mday < -1)
//                           mday = 1;
//                     /* update days for wday calculation */
//                     days += mday;
//                     /* handle yday */
//                     if ((tim_p->tm_yday += mday) < 0) {
//                           --year;
//                           tim_p->tm_yday = _DAYS_IN_YEAR(year) - 1;
//                     } else {
//                           mday = _DAYS_IN_YEAR(year);
//                           if (tim_p->tm_yday > (mday - 1))
//                                 tim_p->tm_yday -= mday;
//                     }
//                   }
//                 }
//             }
//         }
//     }
//
//   /* add appropriate offset to put time in gmt format */
//   if (isdst == 1)
//     tim += (time_t) tz->__tzrule[1].offset;
//   else /* otherwise assume std time */
//     tim += (time_t) tz->__tzrule[0].offset;
//
//   TZ_UNLOCK;
//
//   /* reset isdst flag to what we have calculated */
//   tim_p->tm_isdst = isdst;

   /* compute day of the week */
   if ((tim_p->tm_wday = (days + 4) % 7) < 0)
     tim_p->tm_wday += 7;

   return tim;
}


/**************************************************************************//**
 * Function name 	: decToBcd
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
uint8_t decToBcd(uint8_t val)
{
  return( (val/10*16) + (val%10) );
}
/**************************************************************************//**
 * Function name 	: bcdToDec
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
uint8_t bcdToDec(uint8_t val)
{
  return( (val/16*10) + (val%16) );
}
/*************************************************************************
Function Name: RTC_Check_First_Time
input: None.
Output: None.
Discription: this function is used to check RTC is working proprly or not on power ON.
*************************************************************************/
void RTC_Check_First_Time(void)
{

	RTC_DateTypeDef gDate;
	RTC_TimeTypeDef gTime;
	unsigned char dd=0,mo=0,yy=0,hh=0,mm=0,ss=0;

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

	dd = gDate.Date;
	mo = gDate.Month;
    yy = gDate.Year;
	hh = gTime.Hours;
	mm = gTime.Minutes;
	ss = gTime.Seconds;

	osDelay(3000);

	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

	if((dd == gDate.Date) && (mo == gDate.Month) && (yy == gDate.Year) && (hh == gTime.Hours) && (mm == gTime.Minutes) && (ss == gTime.Seconds))
	{
		UTC=1609459200; // 1/1/2021
		set_time(t1);
		RTC_Fauly_reason=RTC_FAULT_TIME_NOT_UPDATE_ON_BOOT;
		//HW_RTC_FAULT_BIT=FAULTY;
	}
	else
	{
		if((gTime.Hours > 23) || (gTime.Minutes > 59) || (gTime.Seconds > 59) || (gDate.Date > 31) || (gDate.Date < 1) || (gDate.Month > 12) || (gDate.Month < 1) || (gDate.Year < 1) || (gDate.Year > 99))																// Check RTC time
		{
			UTC=1609459200; // 1/1/2021
			set_time(t1);
			RTC_Fauly_reason=RTC_FAULT_TIME_OUTOFF_RANGE;
			//HW_RTC_FAULT_BIT=FAULTY;
		}
		else
		{
			RTC_Fauly_reason=RTC_WORKING;
			//HW_RTC_FAULT_BIT=WORKING;
		}
	}

}
/**************************************************************************//**
 * Function name 	: RTC_reset
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
void RTC_reset()
{
	HAL_RTC_DeInit(&hrtc);
	osDelay(20);
	MX_RTC_Init();
	osDelay(20);
	//set_time(t1);
}
/**************************************************************************//**
 * Function name 	: Pro_RTC_Check
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/
void Pro_RTC_Check(uint64_t UTC)
{
	osDelay(5000);
}
/*************************************************************************
Function Name: RTC_Check_Working
input: None.
Output: None.
Discription: this function is used to check RTC is working proprly or not.
*************************************************************************/
//void RTC_Check_Working(uint64_t prvUTC)
//{
//	if(prvUTC==UTC)
//	{
//		(RTC_Time_hang_Counter)++;
//	}
//	else
//	{
//		RTC_Time_hang_Counter=0;
//		RTC_Reinit_Count=0;
//		if(RTC_Fauly_reason!=RTC_WORKING)
//		{
//			//HW_RTC_FAULT_BIT=WORKING;
//			RTC_Fauly_reason=RTC_WORKING;
//		}
//
//	}
//
//	if(RTC_Time_hang_Counter>60)
//	{
//		RTC_Time_hang_Counter=0;
//		HW_RTC_FAULT_BIT=FAULTY;
//		if(RTC_Reinit_Count<=3)
//		{
//			(RTC_Reinit_Count)++;
//			RTC_Fauly_reason=RTC_FAULT_TIME_NOT_UPDATE_IN_TASK;
//			RTC_reset();
//		}
//		else
//		{
//			RTC_Fauly_reason=RTC_FAULT_TIME_NOT_UPDATE_EVEN_AFTER_REINIT;
//		}
//	}
//	if((reboot_cmd)&(1<<RTC_RESET_BIT))
//	{
//		reboot_cmd=reboot_cmd&(~(1<<RTC_RESET_BIT));
//		RTC_reset();
//		HW_RTC_FAULT_BIT=WORKING;
//		RTC_Fauly_reason=RTC_WORKING;
//		RTC_Time_hang_Counter=0;
//		RTC_Reinit_Count=0;
//	}
//}
/**************************************************************************//**
 * Function name 	: RTC_time
 * arguments		: 1)
 * return 		 	:
 * Note				: Retrn UTC time in S
 *****************************************************************************/
//mbedtls_time_t RTC_time( mbedtls_time_t* timer )
//{
//   // ((void) timer);
//    return( UTC/1000);
//    //return( UTC );
//}
/**************************************************************************//**
 * Function name 	: localtime_new
 * arguments		: 1)
 * return 		 	:
 * Note				:
 *****************************************************************************/

struct tm localtime_new(uint64_t _timer)
{
	struct tm tim;
		epoch=_timer;
		leap_days=0;
		leap_year_ind=0;

		// Add or substract time zone here.
		//epoch+=19800 ; //GMT +5:30 = +19800 seconds

		tim.tm_sec = epoch%60;
		epoch /= 60;
		tim.tm_min = epoch%60;
		epoch /= 60;
		tim.tm_hour  = epoch%24;

		epoch /= 24;

		days_since_epoch = epoch;      //number of days since epoch
		tim.tm_wday = week_days[days_since_epoch%7];  //Calculating WeekDay
		days_since_epoch -= 729;	// Decrease number of days for 1970 and 1971
		for(ntp_year = 0;days_since_epoch > 366;ntp_year++)
		{
			if(ntp_year % 4 == 0)
				days_since_epoch -= 366;
			else
				days_since_epoch -= 365;
		}
		if(days_since_epoch == 366 && ntp_year % 4 != 0)
		{
			days_since_epoch -= 365;
			ntp_year++;
		}
		ntp_year += 1972;

		//sprintf((char *)print,"%u %u\r\n ",ntp_year,days_since_epoch);
		//HAL_UART_Transmit(&huart8, (uint8_t *)print,strlen((const char *)print), 1000);

	//	ntp_year = 1970+(days_since_epoch/365); // ball parking year, may not be accurate!

	//	int i;
	//	for (i=1972; i<ntp_year; i+=4)      // Calculating number of leap days since epoch/1970
	//	 if(((i%4==0) && (i%100!=0)) || (i%400==0))
	//		leap_days++;
	//
	//	ntp_year = 1970+((days_since_epoch - leap_days)/365); // Calculating accurate current year by (days_since_epoch - extra leap days)
	//	day_of_year = ((days_since_epoch - leap_days)%365)+1;
		tim.tm_year=ntp_year;


		if(((ntp_year%4==0) && (ntp_year%100!=0)) || (ntp_year%400==0))
		{
		 month_days[1]=29;     //February = 29 days for leap years
		 leap_year_ind = 1;    //if current year is leap, set indicator to 1
		}
		else month_days[1]=28; //February = 28 days for non-leap years

		temp_days=0;
		day_of_year = days_since_epoch;
		for (ntp_month=0 ; ntp_month <= 11 ; ntp_month++) //calculating current Month
		{
		   if (day_of_year <= temp_days)
			   break;
		   temp_days = temp_days + month_days[ntp_month];
		}
		tim.tm_mon=ntp_month;

		temp_days = temp_days - month_days[ntp_month-1]; //calculating current Date
		ntp_date = day_of_year - temp_days;
		tim.tm_mday=ntp_date;

		//sprintf((char *)print,"%u %u %u\r\n ",ntp_date, temp_days, ntp_month);
		//HAL_UART_Transmit(&huart8, (uint8_t *)print,strlen((const char *)print), 1000);
		#ifdef print_epoch

			sprintf((char *)print,"Time:%u-%u-%u\r\n ",tim.tm_hour, tim.tm_min,tim.tm_sec);
			HAL_UART_Transmit(&huart1, (uint8_t *)print,strlen((const char *)print), 1000);
			sprintf((char *)print,"Date:%02u-%02u-%02u\r\n ",tim.tm_mday, tim.tm_mon,tim.tm_year);
			HAL_UART_Transmit(&huart1, (uint8_t *)print,strlen((const char *)print), 1000);

		#endif
		return tim;
}

void Get_Astro_time(void)
{
	time_t current_time = 0, timezone_UTC;
	// pointer
	struct tm* ptime;
	timezone_UTC = get_offset_UTC();
	if(flag_timezone_sign == 1)
	{
		current_time = Calculate_SunsetSunrise(SUNRISE)-timezone_UTC;
	}
	else
	{
		current_time = Calculate_SunsetSunrise(SUNRISE)+timezone_UTC;
	}
	ptime = gmtime(&current_time);
	gFinalAnaValF[SUNRISE_HOUR_gFinalAnaValF] = (ptime->tm_hour)%24;
	gFinalAnaValF[SUNRISE_MIN_gFinalAnaValF]  = ptime->tm_min;
	gFinalAnaValF[SUNRISE_SEC_gFinalAnaValF]  = ptime->tm_sec;

	if(flag_timezone_sign == 1)
	{
		current_time = Calculate_SunsetSunrise(SUNSET)-timezone_UTC;
	}
	else
	{
		current_time = Calculate_SunsetSunrise(SUNSET)+timezone_UTC;
	}
	ptime = gmtime(&current_time);
	gFinalAnaValF[SUNSET_HOUR_gFinalAnaValF] = (ptime->tm_hour)%24;
	gFinalAnaValF[SUNSET_MIN_gFinalAnaValF]  = ptime->tm_min;
	gFinalAnaValF[SUNSET_SEC_gFinalAnaValF]  = ptime->tm_sec;

}

uint32_t Calculate_SunsetSunrise(unsigned char direction)//, struct tm fine_time)
{
	int i=0;
	double lngHour =0;
    double t=0;
	double M=0 ;
	double L =0;
	double RA=0;
    double Lquadrant=0;
    double RAquadrant =0;
    double sinDec =0;
    double cosDec =0;
    double cosH =0;
    double H=0;
    double T =0;
    //double cosHFloat=0;
    double UT=0;
	int N = 0;
   // double CosCheck=0;
   // double CosCheckFloat=0;
   // double TempcosH=0;
   // double  acosTempcosH=0;
    double  RadacosTempcosH=0;
    float temp1,temp2;
   // return x;

	for(i=1;i<bcdToDec(gDate.Month);i++)
	{
		if(i==1 || i==3 ||i==5 ||i==7 ||i==9 ||i==11 )
		{
			N=N+31;
		}
		else if(i==4 || i==6 ||i==8 ||i==10||i==12 )
		{
			N=N+30;
		}
		if(i==2)
		{
			if((bcdToDec(gDate.Year) + 2000 - 1900)%4==0)
			N=N+29;
			else
			N=N+28;
		}
	}
	N=N+bcdToDec(gDate.Date);
    /* appr. time (t) */
	lngHour = (double)EPROM_General.Cust_Detail.Longitude / 15.0;
    if (direction == SUNRISE)
        t = N + ((6.0 - lngHour) / 24.0);
    else
        t = N + ((18.0 - lngHour) / 24.0);
    /* mean anomaly (M) */
    M = (0.9856 * t) - 3.289;
    temp1=Deg2Rad(M);
    temp1=sinf(temp1);
    temp2=Deg2Rad(2 * M);
    temp2=sinf(temp2);
	/* true gGpsLongitude (L) */
    L = M + (1.916 * temp1 ) + (0.020 * temp2) + 282.634;

    L = FixValue(L, 0, 360);
    /* right asc (RA) */
    RA = Rad2Deg(atan(0.91764 * tanf(Deg2Rad(L))));
    RA = FixValue(RA, 0, 360);
    /* adjust quadrant of RA */
    temp2 =(L / 90.0);
    temp2 =(int)(temp2);
    Lquadrant =  temp2* 90.0;
    temp2 =(RA / 90.0);
    temp2 =(int)(temp2);
    RAquadrant =  temp2* 90.0;
    RA = RA + (Lquadrant - RAquadrant);
    RA = RA / 15.0;
    /* sin cos DEC (sinDec / cosDec) */
     sinDec = 0.39782 * sinf(Deg2Rad(L));
     cosDec = cosf(asinf(sinDec));
    /* local hour angle (cosH) */
    cosH = (cos(Deg2Rad(zenith / 1000.0)) - (sinDec * sin(Deg2Rad(EPROM_General.Cust_Detail.Lattitude)))) / (cosDec * cos(Deg2Rad(EPROM_General.Cust_Detail.Lattitude)));
    RadacosTempcosH=Rad2Deg(acosf((float)cosH));
    if (direction == SUNRISE)
        H = 360.0 - RadacosTempcosH;
    else
        H = RadacosTempcosH;
    H = H / 15.0;
    /* time (T) */
     T = H + RA - (0.06571 * t) - 6.622;
    /* universal time (T) */
    UT = T - lngHour;
	//UT += utcOffset // local UTC offset - Not use
	// if(EPROM.DST.Time_change_dueto_DST == 1)           // add Day light saving time zone difference to min.
	// {
	// 	UT += (float)(DST.SLC_DST_Time_Zone_Diff/60);
    // }
    // if (daylightChanges != null)
    //     if ((RTUTime.Date > daylightChanges.Start) && (RTUTime.Date < daylightChanges.End))
    //         UT += daylightChanges.Delta.TotalHours;

    UT = FixValue(UT, 0, 24);
    UT = UT*100;
	return (((((int)UT * 3600) / 100)));// + utcOffset); // Convert to seconds
}

int get_offset_UTC(void)
{
	int utcOffset_hour=0, utcOffset_min=0;
	//char temp_hour[3]={0},temp_min[3]={0};
	if(EPROM_General.Cust_Detail.Tz_sign == 1)
	{
		flag_timezone_sign = 1;
	}
	else
	{
		flag_timezone_sign = 0;
	}

	utcOffset_hour = (int)(EPROM_General.Cust_Detail.Tz_hours);
	utcOffset_min = (int)(EPROM_General.Cust_Detail.Tz_min);
	return ((utcOffset_hour*3600)+(utcOffset_min*60));
}

double DegreesToAngle(double degrees, double minutes, double seconds)
{
	if (degrees < 0)
		return ((double)(degrees - (minutes / 60.0) - (seconds / 3600.0)));
	else
		return ((double)(degrees + (minutes / 60.0) + (seconds / 3600.0)));
}


double Deg2Rad(double angle)
{
    return (PI * angle / 180.0);
}

double Rad2Deg(double angle)
{
    return (180.0 * angle / PI);
}

double FixValue(double value, double min, double max)
{
    while (value < min)
    {
        value += (max - min);
    }

    while (value >= max)
    {
        value -= (max - min);
    }

    return value;
}

//======================================================================
// RTC END
//======================================================================
