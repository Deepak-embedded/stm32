#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <sys/time.h>

#include "pcbplcInterface.h"
#include "pcbplcService.h"
#include "pcbplccomm.h"
#include "pcbplcConfig.h"
#include "main.h"
#define BUTTON_DEBOUNCE_MS 50           // 50ms debounce time

pcbplcCnfg_t gpcbplcCnfg;
extern uint8_t pcbplcfile[12*1024];
extern uint8_t RecFile[1*1024];

bool continue_loop = true;
void *timer_thread_routine(void *arg);

static long sampling_rate = 10; //In milli second

bool service_destroy()
{
    return true;
}

bool service_start()
{
	char logBuff[256]= {0, };
	unsigned int new_time = 0, prev_time = 0;

	if ((checkFileAvibility(gPlcFile)==0) && (checkFileAvibility(gRecFile)==0))
	{
        PLC_RPOG_Flag = 0;
        Extract_Alloc();

        if(xSemaphoreTake(sendExternalFlashSemaphore, 5000) == pdTRUE )
		{
			ReadRecipeFile();
			xSemaphoreGive(sendExternalFlashSemaphore);
		}
        //TODO : If don't get semaphoshe than retry
        plcTimer_start();
	}
    else
    {
    	WriteLog(pcbplc_logger, "Could not found PLC Logic", LOG_ERROR);
    	//return false;
    }

    while (continue_loop)
    {
    	if((1 == PLC_RPOG_Flag)  && (1 == REC_RPOG_Flag))
    	{
    		PLC_RPOG_Flag = 0;
    		REC_RPOG_Flag = 0;

            Extract_Alloc();
			if(xSemaphoreTake(sendExternalFlashSemaphore, 5000) == pdTRUE )
			{
				ReadRecipeFile();
				xSemaphoreGive(sendExternalFlashSemaphore);
			}
            plcTimer_start();
        }

    	osDelay(500); // 500ms
        read_rtu_datetime();

        if (gpcbplcCnfg.debug)
        {
            memset(&logBuff, 0, sizeof(logBuff));
            sprintf(logBuff, "service state %d\r\n", gpcbplcCnfg.mIsEnable);
            WriteLog(pcbplc_logger, logBuff, LOG_INFO);
        }

       // if(gpcbplcCnfg.mIsEnable == 1)
        {
            get_di_status();
            get_ai_status();
			set_do();
			set_dual_Do();

            if (IsLogRateMatched())
            {
            	Build_Data_for_server();
            	flagLORAPubLogData = 1;
            	flagLORAPubLogData_fail = 255; // set 255 to check if it failed
            }
            pcbplc_BaseLoop();

            new_time = ((gTimeInfo.mHour * 3600) + (gTimeInfo.mMinute * 60) + gTimeInfo.mSecond);
            if (new_time - prev_time > 10)
            {
                prev_time = new_time;

                if (gpcbplcCnfg.debug)
                {
                    memset(&logBuff, 0, sizeof(logBuff));
                    sprintf(logBuff, "Store general purpose data\r\n");
                    WriteLog(pcbplc_logger, logBuff, LOG_INFO);
                }
            }
        }

//        if(true == gPcbplcInfo.mReloadCnfg)
//        {
//            gPcbplcInfo.mReloadCnfg = false;
//            if(-1 == pcbplc_config_json_parsing(&gpcbplcCnfg))
//            {
//                WriteLog(pcbplc_logger, "Could not found configuration file", LOG_ERROR);
//                break;
//            }
//            pcbplc_config_log(&gpcbplcCnfg);
//
//            if (gpcbplcCnfg.debug)
//            {
//                memset(&logBuff, 0, sizeof(logBuff));
//                sprintf(logBuff, "Found reload_config event");
//                WriteLog(pcbplc_logger, logBuff, LOG_INFO);
//            }
//        }
//
//        if(true == gPcbplcInfo.mReloadReceipe)
//        {
//            gPcbplcInfo.mReloadReceipe = false;
//            WriteModifiedRecipeFile(MODIFIED_RECIPE_FILE_PATH);
//            ReadRecipeJsonFile();
//        }

        osDelay(sampling_rate);  // ms
    }

    return true;
}

bool service_restart()
{
    return false;
}

bool service_stop()
{
    /* Free memory which is allocated for pcbplc extraction */
    pcbplc_memory_free();

    return false;
}

void read_rtu_datetime() {

	gTimeInfo.mHour=gTime.Hours;
	gTimeInfo.mMinute=gTime.Minutes;
	gTimeInfo.mSecond=gTime.Seconds;
	gTimeInfo.mDate=gDate.Date;
	gTimeInfo.mMonth=gDate.Month;
	gTimeInfo.mYear=gDate.Year;
	gTimeInfo.mDayofWeek = gDate.WeekDay;

	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+0] = gTimeInfo.mDate;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+1] = gTimeInfo.mMonth;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+2] = gTimeInfo.mYear + 2000;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+3] = gTimeInfo.mHour;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+4] = gTimeInfo.mMinute;
	gFinalAnaValF[RTC_TIME_DATE_gFinalAnaValF+5] = gTimeInfo.mSecond;
}

int IsLogRateMatched()
{
    int tIsMatched = 0;
    static unsigned char tsOld_minute=255;
    static unsigned char tsIsFirstTime = 1;
    static unsigned char tsOld_hour=255;

    if((EPROM_General.LogRate == 60) || (EPROM_General.LogRate == 120) || (EPROM_General.LogRate == 180) || (EPROM_General.LogRate == 240) || (EPROM_General.LogRate == 360) || (EPROM_General.LogRate == 720))
    {
    	if( (((gTimeInfo.mHour%(unsigned char)(EPROM_General.LogRate/60)) == 0) && (gTimeInfo.mHour != tsOld_hour))||((flagLORAPubLogData_fail == 1)))
    	{
			tsOld_hour = gTimeInfo.mHour;
			tIsMatched = 1;
    	}
    }
    else
    {
        if ((((gTimeInfo.mHour * 3600) + (gTimeInfo.mMinute * 60) + gTimeInfo.mSecond) % (EPROM_General.LogRate * 60)) < 2)  // for log rate data structure match to use in IsLogRateMatched() func || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-22
        {
            if((1 == tsIsFirstTime) || (tsOld_minute != gTimeInfo.mMinute))
            {
                tsIsFirstTime = 0;
                tsOld_minute = gTimeInfo.mMinute;
                tsOld_hour = gTimeInfo.mHour;
                tIsMatched = 1;
            }
            else if ( ((EPROM_General.LogRate % 60) == 0) && (tsOld_hour != gTimeInfo.mHour))  // for log rate data structure match to use in IsLogRateMatched() func || ticket : https://cimcondigital.atlassian.net/browse/IRTU6000PP-22
            {
                tsOld_hour = gTimeInfo.mHour;
                tIsMatched = 1;
            }
        }
    }

    return (tIsMatched);
}

int update_do_status_key(int pin_no, int pin_value)
{
//    if( (gpcbplcCnfg.mMaxDoEnabled >= pin_no) && (pin_no != 0))
//    {
//        configuration_create_backup(NULL);
//
//        char *filename = (char *) configuration_get_filepath();
//        if (filename)
//        {
//            json_object *parse_result = json_object_from_file(filename);
//            if (parse_result)
//            {
//                json_object *do_Status = json_object_object_get(parse_result, "do_status");
//                for (int i = 0; i < pin_no; i++)
//                {
//                    if ((pin_no - 1) == i)
//                    {
//                        json_object_array_put_idx(do_Status, i, json_object_new_int(pin_value));
//                        break;
//                    }
//                }
//
//                int d = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
//                if (d > -1)
//                {
//                    json_object_to_fd(d, parse_result,JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
//                    close(d);
//                }
//
//                free(filename);
//                filename = NULL;
//                json_object_put(parse_result);
//                parse_result = NULL;
//                return 0;
//            }
//        }
//        else
//        {
//            WriteLog(pcbplc_logger, "configuration file not found", LOG_CRITICAL);
//            return -1;
//        }
//    }
//    else
//    {
//        WriteLog(pcbplc_logger, "Invalid pin number", LOG_ERROR);
        return -1;
//    }
}

unsigned char checkFileAvibility(unsigned char file)
{
	unsigned char result=0;
	//Todo : Add PLC and REC File Validation logic Maulin
	//return 0 on validate 1 on fail

	if(xSemaphoreTake(sendExternalFlashSemaphore, 500) == pdTRUE )
	{
		if(file == 1)
		{
			if(gPlcRecFlash.mPlcFileLength)
			{
				result = verify_OTA_CRC(gPlcRecFlash.mPlcFileCRC,gPlcRecFlash.mPlcFileLength,PUB_FILE_START_ADDRESS);
			}
			else
			{
				result = 1;
			}

		}
		else if(file == 2)
		{
			if(gPlcRecFlash.mRecFileLength)
			{
				result = verify_OTA_CRC(gPlcRecFlash.mRecFileCRC,gPlcRecFlash.mRecFileLength,REC_FILE_START_ADDRESS);
			}
			else
			{
				result = 1;
			}
		}
		xSemaphoreGive(sendExternalFlashSemaphore);
	}
	else
	{
		result =1;
	}

	return result;
}
