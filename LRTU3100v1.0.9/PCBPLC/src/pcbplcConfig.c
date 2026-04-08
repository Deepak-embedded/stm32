#include "pcbplcService.h"
#include "pcbplcConfig.h"
#include "pcbplccomm.h"
#include <errno.h>

//static float timezoneindex[83] = { -12, -11, -10, -9, -8, -8, -7, -7, -7, -6, -6, -6, -6, -5, -5, -5, -4, -4, -4, -4, -3.3,
//                               -3, -3, -3, -3, -2, -1, -1, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
//                               3.3, 4, 4, 4, 4.3, 5, 5, 5.3, 5.3, 5.45, 6, 6, 6.3, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9.3,
//                               9.3, 10, 10, 10, 10, 10, 11, 12, 12, 13 };

void pcbplc_config_log(pcbplcCnfg_t *ptr)
{
//    char tBuf[256] = {0,};
//
//    if(NULL == ptr)
//    {
//        WriteLog(pcbplc_logger, "ptr is null\r\n", LOG_ERROR);
//        return;
//    }
//
//    WriteLog(pcbplc_logger, "/******************* service configuration*******************\r\n", LOG_INFO);
//
//    sprintf(tBuf, "pcbplc service : %s\r\n", ptr->mIsEnable ? "ENABLED" : "DISABLED");
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "client address : %d\r\n", ptr->mClientAddr);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "group address : %d\r\n", ptr->mGroupAddr);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "rtu address : %d\r\n", ptr->mRtuAddr);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "rtu do mode : %d\r\n", ptr->mRtuDoMode);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "max do enabled : %d\r\n", ptr->mMaxDoEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "max ao enabled : %d\r\n", ptr->mMaxAoEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "max di enabled : %d\r\n", ptr->mMaxDiEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "max ai enabled : %d\r\n", ptr->mMaxAiEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "max schedule enabled : %d\r\n", ptr->mMaxSchEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    for(int i = 0; i < ptr->mMaxSchEnabled ; ++i)
//    {
//        sprintf(tBuf, "enabled : %s, start : hour(%d) minute(%d), stop : hour(%d) minute(%d)\r\n",
//                                ptr->sSch[i].mIsSchEnabled ? "ENABLED" : "DISABLED",
//                                ptr->sSch[i].mStartHour, ptr->sSch[i].mStartMin,
//                                ptr->sSch[i].mStopHour, ptr->sSch[i].mStopMin);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    for(int tIdx = 0 ; tIdx < ptr->mMaxDoEnabled ; ++tIdx)
//    {
//        sprintf(tBuf, "do key status(%s)\r\n", ptr->mDoStatus[tIdx] ? "ON" : "OFF");
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    sprintf(tBuf, "max alarm enabled : %d\r\n", ptr->mMaxAlarmEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "site name : %s\r\n", ptr->mSiteName);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    for(int i = 0; i < ptr->mMaxAlarmEnabled ; ++i)
//    {
//        sprintf(tBuf, "type : %d, mobile number : %s\r\n", ptr->mAlarm[i].mType, ptr->mAlarm[i].mobileNo);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    sprintf(tBuf, "timezone : %lf, timeoffset:%d\r\n", ptr->mtz, ptr->mtimeoffset);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "Max Query Enabled : %d\r\n", ptr->mMaxQuery);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "Max Data tag Enabled : %d\r\n", ptr->mMaxDataTagEnabled);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    for(int tIdx = 0; tIdx < ptr->mMaxDataTagEnabled ; ++tIdx)
//    {
//        sprintf(tBuf, "Data Tag(%d) : %d\r\n", tIdx+1, ptr->mDataTag[tIdx]);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    //sprintf(tBuf, "Scan Rate : %d", ptr->mScanRate);
//    //WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    sprintf(tBuf, "Number Of Destination : %d\r\n", ptr->mNumOfDest);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    for(int tIdx = 0; tIdx < ptr->mNumOfDest ; ++tIdx)
//    {
//        sprintf(tBuf, "Destination-%02d : %s\r\n", tIdx, destination_list[tIdx]);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    WriteLog(pcbplc_logger, "Modbus configuration: ", LOG_INFO);
//
//    sprintf(tBuf, "Number Of modbus configurations : %d\r\n", ptr->mNumOfModbusConfig);
//    WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//
//    for(int tIdx = 0; tIdx < ptr->mNumOfModbusConfig ; ++tIdx)
//    {
//        sprintf(tBuf, "\tDevice name: %s, Parameter name: %s\r\n",
//                ptr->modbusConfig[tIdx].mDeviceName, ptr->modbusConfig[tIdx].mParaName);
//        WriteLog(pcbplc_logger, tBuf, LOG_INFO);
//    }
//
//    WriteLog(pcbplc_logger, "/************************************************************\r\n", LOG_INFO);
}

bool pcbplc_default_configuration(pcbplcCnfg_t* ptr)
{
    if(ptr)
    {
        ptr->mIsEnable = 1;
        ptr->mClientAddr = 1;
        ptr->mGroupAddr = 1;
        ptr->mRtuAddr = 1;
        ptr->mMaxDoEnabled = 8;//12;  // replace 8 with 12
        ptr->mMaxDiEnabled = 16;
        ptr->mMaxAiEnabled = 4;
        ptr->mMaxAoEnabled = 2;
        ptr->mMaxSchEnabled = 0;
        ptr->mMaxAlarmEnabled = 0;

        for(int tIdx = 0 ; tIdx < MAXSCH ; ++tIdx)
        {
            ptr->sSch[tIdx].mIsSchEnabled = 0;
            ptr->sSch[tIdx].mStartHour = 0;
            ptr->sSch[tIdx].mStartMin = 0;
            ptr->sSch[tIdx].mStopHour = 0;
            ptr->sSch[tIdx].mStopMin = 0;
        }

        for(int tIdx = 0 ; tIdx < ptr->mMaxDoEnabled ; ++tIdx)
        {
            ptr->mDoStatus[tIdx] = 0;
            ptr->mOldDoStatus[tIdx] = 0;
        }

        ptr->mLogRate = 60; //60 minutes

        strcpy((char *)ptr->mSiteName, "default");

        memset(ptr->mSiteName, 0 , sizeof(ptr->mSiteName));

        for(int tIdx = 0 ; tIdx < MAXALARM ; ++tIdx)
        {
            ptr->mAlarm[tIdx].mType = 0;
            memset(ptr->mAlarm[tIdx].mobileNo, 0, sizeof (ptr->mAlarm[tIdx].mobileNo));
        }

        ptr->mtz = 5.3;
        ptr->mtimeoffset = 19800;

        ptr->mMaxQuery = 0;

        ptr->mMaxDataTagEnabled = 18;
        for(int tIdx = 0; tIdx < ptr->mMaxDataTagEnabled ; ++tIdx)
        {
            ptr->mDataTag[tIdx] = 1+(tIdx*2);
        }

        //ptr->mScanRate = 10;
        ptr->mNumOfDest = 0;
    }
    return 0;
}

//int pcbplc_config_json_parsing(pcbplcCnfg_t* ptr)
//{
//    struct json_object	*tMainObj = NULL;
//    struct json_object	*tSubObj  = NULL;
//    struct json_object	*tMainSmsInfo	= NULL;
//    struct json_object	*tSubSmsInfo	= NULL;
//    struct json_object	*tMainSchedule	= NULL;
//    struct json_object	*tSubSchedule	= NULL;
//    struct json_object	*tMainDoStatus  = NULL;
//    int tDoStatusArrLen = 0;
//    //int tScheduleArrLen = 0;
//    //int tSmsInfoArrLen = 0;
//    char tBuffer[128] = {0,};
//    int tIdx, tRetVal = -1;
//
//    char* filename = (char*)configuration_get_filepath();
//    if(!filename)
//    {
//        WriteLog(pcbplc_logger, "could not found configuration file", LOG_CRITICAL);
//        return -1;
//    }
//
//    tMainObj = json_object_from_file(filename);
//    if(NULL == tMainObj)
//    {
//        WriteLog(pcbplc_logger, "could not parse configuration file", LOG_CRITICAL);
//        return -1;
//    }
//
//    do
//    {
//        /* publisher  */
//        tSubObj = json_object_object_get(tMainObj, "publisher");
//        if(NULL != tSubObj)
//        {
//            if (destination_list)
//            {
//                for (tIdx = 0; tIdx < gpcbplcCnfg.mNumOfDest; ++tIdx)
//                {
//                    if(destination_list[tIdx])
//                    {
//                        free(destination_list[tIdx]);
//                        destination_list[tIdx] = NULL;
//                    }
//                }
//
//                free(destination_list);
//                destination_list = NULL;
//            }
//
//            json_object *tObj = json_object_object_get(tSubObj, "destination");
//            if (NULL != tObj)
//            {
//                int tPublisherArrLen = json_object_array_length(tObj);
//                destination_list = (char **) calloc(1, (int) (tPublisherArrLen) * sizeof(char *));
//                gpcbplcCnfg.mNumOfDest = tPublisherArrLen;
//
//                for (tIdx = 0; tIdx < tPublisherArrLen; ++tIdx)
//                {
//                    json_object *tPublisherObj = json_object_array_get_idx(tObj, tIdx);
//                    if (NULL != tPublisherObj)
//                    {
//                        int tLen = json_object_get_string_len(tPublisherObj);
//                        destination_list[tIdx] = (char *) calloc(1, tLen + 10);
//                        strcpy(destination_list[tIdx], json_object_get_string(tPublisherObj));
//                    }
//                }
//            }
//            else
//            {
//                sprintf(tBuffer, "Error at \"destination\" json_object_object_get");
//                WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at \"publisher\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* RTU Mode */
//        tSubObj = json_object_object_get(tMainObj, "mode");
//        if(NULL != tSubObj)
//        {
//            ptr->mRtuDoMode = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"RTU mode\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Client address */
//        tSubObj = json_object_object_get(tMainObj, "client_id");
//        if(NULL != tSubObj)
//        {
//            ptr->mClientAddr = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"client_id\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Group address */
//        tSubObj = json_object_object_get(tMainObj, "group_id");
//        if(NULL != tSubObj)
//        {
//            ptr->mGroupAddr = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"group_id\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* RTU address */
//        tSubObj = json_object_object_get(tMainObj, "rtu_id");
//        if(NULL != tSubObj)
//        {
//            ptr->mRtuAddr = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"RTU address\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number Of Di */
//        tSubObj = json_object_object_get(tMainObj, "no_of_di");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxDiEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_di\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number of AI */
//        tSubObj = json_object_object_get(tMainObj, "no_of_ai");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxAiEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_ai\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number of DO */
//        tSubObj = json_object_object_get(tMainObj, "no_of_do");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxDoEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_do\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number of AO */
//        tSubObj = json_object_object_get(tMainObj, "no_of_ao");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxAoEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_ao\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Do key status  */
//        tSubObj = json_object_object_get(tMainObj, "do_status");
//        if(NULL != tSubObj)
//        {
//            tDoStatusArrLen = json_object_array_length(tSubObj);
//
//            for(tIdx = 0; tIdx < tDoStatusArrLen ; ++tIdx)
//            {
//                tMainDoStatus = json_object_array_get_idx(tSubObj, tIdx);
//                if(NULL != tMainDoStatus)
//                {
//                    ptr->mDoStatus[tIdx] = json_object_get_int(tMainDoStatus);
//                    ptr->mOldDoStatus[tIdx] = json_object_get_int(tMainDoStatus);
//                }
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at \"do_status\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number Of Schedule */
//        /* This key is not used so mark as commented
//
//        tSubObj = json_object_object_get(tMainObj, "no_of_schedule");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxSchEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_schedule\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//        */
//
//#if 0
//        /* Number Of query */
//        tSubObj = json_object_object_get(tMainObj, "no_of_query");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxQuery = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_query\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Number Of Alarm */
//        tSubObj = json_object_object_get(tMainObj, "no_of_alarm");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxAlarmEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"no_of_alarm\"json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//#endif
//        /* Site Name */
//        tSubObj = json_object_object_get(tMainObj, "site_name");
//        if(NULL != tSubObj)
//        {
//            if(NULL != json_object_get_string(tSubObj))
//            {
//                strncpy(ptr->mSiteName, json_object_get_string(tSubObj), sizeof(ptr->mSiteName));
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"site_name\"json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Log rate (in minutes) */
//        tSubObj = json_object_object_get(tMainObj, "log_rate");
//        if(NULL != tSubObj)
//        {
//            ptr->mLogRate = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"log_rate\"json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* SMS Info */
//        tSubObj = json_object_object_get(tMainObj, "smsinfo");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxAlarmEnabled = json_object_array_length(tSubObj);
//
//            for(tIdx = 0; tIdx < ptr->mMaxAlarmEnabled ; ++tIdx)
//            {
//                tMainSmsInfo = json_object_array_get_idx(tSubObj, tIdx);
//                if(NULL != tMainSmsInfo)
//                {
//                    tSubSmsInfo = json_object_object_get(tMainSmsInfo, "type");
//                    if(NULL != tSubSmsInfo)
//                    {
//                        ptr->mAlarm[tIdx].mType = json_object_get_int(tSubSmsInfo);
//                    }
//
//                    tSubSmsInfo = json_object_object_get(tMainSmsInfo, "mobileno");
//                    if(NULL != tSubSmsInfo)
//                    {
//                        if(NULL != json_object_get_string(tSubSmsInfo))
//                        {
//                            strncpy(ptr->mAlarm[tIdx].mobileNo, json_object_get_string(tSubSmsInfo),
//                                    sizeof(ptr->mAlarm[tIdx].mobileNo));
//                        }
//                    }
//                }
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at smsinfo json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* Schedule */
//        tSubObj = json_object_object_get(tMainObj, "schedule");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxSchEnabled = json_object_array_length(tSubObj);
//
//            for(tIdx = 0; tIdx < ptr->mMaxSchEnabled ; ++tIdx)
//            {
//                tMainSchedule = json_object_array_get_idx(tSubObj, tIdx);
//                if(NULL != tMainSchedule)
//                {
//                    tSubSchedule = json_object_object_get(tMainSchedule, "enable");
//                    if(NULL != tSubSchedule)
//                    {
//                        ptr->sSch[tIdx].mIsSchEnabled = json_object_get_int(tSubSchedule);
//                    }
//
//                    tSubSchedule = json_object_object_get(tMainSchedule, "start_hour");
//                    if(NULL != tSubSchedule)
//                    {
//                        ptr->sSch[tIdx].mStartHour = json_object_get_int(tSubSchedule);
//                    }
//
//                    tSubSchedule = json_object_object_get(tMainSchedule, "start_minute");
//                    if(NULL != tSubSchedule)
//                    {
//                        ptr->sSch[tIdx].mStartMin = json_object_get_int(tSubSchedule);
//                    }
//
//                    tSubSchedule = json_object_object_get(tMainSchedule, "stop_hour");
//                    if(NULL != tSubSchedule)
//                    {
//                        ptr->sSch[tIdx].mStopHour = json_object_get_int(tSubSchedule);
//                    }
//
//                    tSubSchedule = json_object_object_get(tMainSchedule, "stop_minute");
//                    if(NULL != tSubSchedule)
//                    {
//                        ptr->sSch[tIdx].mStopMin = json_object_get_int(tSubSchedule);
//                    }
//                }
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at schedule json_object_object_get\n");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* timezone */
//        tSubObj = json_object_object_get(tMainObj, "timezone");
//        if(NULL != tSubObj)
//        {
//            char tFlag = 0;
//
//            //printf("timezone:%f\n", json_object_get_double(tSubObj));
//            int tConvertIndex = 0;
//            tConvertIndex = json_object_get_int(tSubObj);
//            ptr->mtz = timezoneindex[tConvertIndex - 1];
//
//            if(ptr->mtz < 0)
//            {
//                tFlag = 1;
//                ptr->mtz *= -1;
//            }
//            unsigned int temp = ((int)gpcbplcCnfg.mtz*3600);
//            temp += (float)(gpcbplcCnfg.mtz - (int)gpcbplcCnfg.mtz)   * 100 * 60;
//            ptr->mtimeoffset = temp;
//            if(tFlag == 1)
//            {
//                tFlag = 0;
//                ptr->mtz *= -1;
//                ptr->mtimeoffset *= -1;
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"timezone\"json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        #if 0
//        /* Number of Data tag */
//        tSubObj = json_object_object_get(tMainObj, "max_data_tag");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxDataTagEnabled = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"max_data_tag\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//        #endif
//
//        /* Data Tag */
//        tSubObj = json_object_object_get(tMainObj, "data_tag");
//        if(NULL != tSubObj)
//        {
//            ptr->mMaxDataTagEnabled = json_object_array_length(tSubObj);
//
//            for(tIdx = 0; tIdx < ptr->mMaxDataTagEnabled ; ++tIdx)
//            {
//                struct json_object	*tMainDataTag = json_object_array_get_idx(tSubObj, tIdx);
//                if(NULL != tMainDataTag)
//                {
//                    ptr->mDataTag[tIdx] = json_object_get_int(tMainDataTag);
//                }
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at \"data_tag\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* debug */
//        tSubObj = json_object_object_get(tMainObj, "debug");
//        if(NULL != tSubObj)
//        {
//            ptr->debug = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"debug\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        /* service enable / disable */
//        tSubObj = json_object_object_get(tMainObj, "service_enable");
//        if(NULL != tSubObj)
//        {
//            ptr->mIsEnable = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"service_enable\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        #if 0
//        /* service scan rate */
//        tSubObj = json_object_object_get(tMainObj, "scan_rate");
//        if(NULL != tSubObj)
//        {
//            ptr->mScanRate = json_object_get_int(tSubObj);
//        }
//        else
//        {
//            sprintf(tBuffer, "Error to parse \"scan_rate\" json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//        #endif
//
//        if(gpcbplcCnfg.modbusConfig)
//        {
//            free(gpcbplcCnfg.modbusConfig);
//            gpcbplcCnfg.modbusConfig = NULL;
//        }
//
//        /* Modbus addresses */
//        tSubObj = json_object_object_get(tMainObj, "modbus");
//        if(NULL != tSubObj)
//        {
//            gpcbplcCnfg.mNumOfModbusConfig = json_object_array_length(tSubObj);
//
//            if(gpcbplcCnfg.mNumOfModbusConfig)
//            {
//                gpcbplcCnfg.modbusConfig = calloc(gpcbplcCnfg.mNumOfModbusConfig, sizeof(sdkConfig_t));
//                if(gpcbplcCnfg.modbusConfig)
//                {
//                    for(tIdx = 0; tIdx < gpcbplcCnfg.mNumOfModbusConfig ; ++tIdx)
//                    {
//                        json_object *tModbusAddress = json_object_array_get_idx(tSubObj, tIdx);
//                        if(NULL != tModbusAddress)
//                        {
//                            json_object *tDev_name = json_object_object_get(tModbusAddress, "device_name");
//                            if(NULL != tDev_name)
//                            {
//                                const char *ptr = json_object_get_string(tDev_name);
//                                if(ptr)
//                                {
//                                    strcpy(gpcbplcCnfg.modbusConfig[tIdx].mDeviceName, ptr);
//                                }
//                            }
//
//                            json_object *tPara_name = json_object_object_get(tModbusAddress, "para_name");
//                            if(NULL != tPara_name)
//                            {
//                                const char *ptr = json_object_get_string(tPara_name);
//                                if(ptr)
//                                {
//                                    strcpy(gpcbplcCnfg.modbusConfig[tIdx].mParaName, ptr);
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//        else
//        {
//            sprintf(tBuffer, "Error at modbus json_object_object_get");
//            WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        }
//
//        tRetVal = 0;
//    }while(0);
//
//    json_object_put(tMainObj);
//
//    free(filename);
//    filename = NULL;
//
//    return (tRetVal);
//}
