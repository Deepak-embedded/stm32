#include "pcbplc.h"
#include "pcbplcInterface.h"
#include "pcbplcService.h"
#include "pcbplccomm.h"
#include "main.h"

//static unsigned int tsIndex = 0;
//static unsigned int tIndex = 0;
//no_block  *gsfddi = NULL;
//no_block *gsfdai = NULL;
//no_block *gsfddo = NULL;
//no_block *gsfdao = NULL;

//static void json_parse_array( json_object *jobj, char *key);
//static void print_json_value(json_object *jobj);
//static void json_parse(json_object* jobj);
//static float ai_data_parse(const char *json_ptr);
//static int di_data_parse(const char *json_ptr);
//static float modbus_data_parse(const char *json_ptr, char *para_name);


/****************************************************************************
 * Function :   set_do_on
 * arg      :   iDoNum -Valid DO number
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void set_do_on(unsigned int iDoNum)
{

//    unsigned char tValue = 1;
//    char tBuffer[128] = {0, };
//    if(iDoNum >= gpcbplcCnfg.mMaxDoEnabled)
//    {
//        sprintf(tBuffer, "iDoNum(%d) is more than enabled DO(%d)", iDoNum, gpcbplcCnfg.mMaxDoEnabled);
//        WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        return;
//    }
//
//    ioctl(gsfddo, WR_VALUE+iDoNum, (int32_t*) &tValue);
//    gsfddo[iDoNum].bits = tValue;
//    writeDoByAddress(iDoNum+1, tValue);

    return;
}

/****************************************************************************
 * Function :   set_do_off
 * arg      :   iDoNum -Valid DO number
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void set_do_off(unsigned int iDoNum)
{
//    unsigned char tValue = 0;
//    char tBuffer[128] = {0, };
//    if(iDoNum >= gpcbplcCnfg.mMaxDoEnabled)
//    {
//        sprintf(tBuffer, "iDoNum(%d) is more than enabled DO(%d)", iDoNum, gpcbplcCnfg.mMaxDoEnabled);
//        WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        return;
//    }
//
//    ioctl(gsfddo, WR_VALUE+iDoNum, (int32_t*) &tValue);
//    gsfddo[iDoNum].bits=tValue;
//    writeDoByAddress(iDoNum+1, tValue);

    return;
}

/****************************************************************************
 * Function :   get_di_status
 * arg      :   none
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void get_di_status()
{
	extern float DI_Final_value[];
    //char tBuffer[256] = {0,};

    //gpcbplcCnfg.mMaxDiEnabled = 16;
    //for(int tDi = 0; tDi < EPROM_General.AI_DI_DO_Detail.Total_Di; ++tDi)
    for(int tDi = 0; tDi < MAX_PHYSICAL_DI; ++tDi)
    {
        int tValue;

        /* complementing DI status: For PCBPLC logic, LOW: DI ON,  High: DI OFF */
        tValue = DI_Final_value[tDi];
        dig_bit_array[tDi] = tValue^ 1;
        gFinalAnaValF[tDi+1] = dig_bit_array[tDi];
        //sprintf(tBuffer, "DI%02d : %d\r\n", tDi + 1, dig_bit_array[tDi]);
        //WriteLog(pcbplc_logger, tBuffer, LOG_INFO);

    }

    if(EPROM_Frequent.Pulse_DI_frequency_Method == 0)
    {
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_count = DI1_Pulse_Count;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF ] = DI1_Pulse_Count ;
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Freq = (DI1_Freq*1000)/(EPROM_Frequent.Pulse_DI_frequency_time);//DI1_Freq_fromWaveLength;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 1 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Freq ;//DI1_Freq_fromWaveLength;
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Freq/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Const;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 4 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated;

        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_count = DI2_Pulse_Count;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 5 ] = DI2_Pulse_Count ;
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Freq = (DI2_Freq*1000)/(EPROM_Frequent.Pulse_DI_frequency_time);//DI2_Freq_fromWaveLength;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 6 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Freq ;//DI2_Freq_fromWaveLength;
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Freq/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Const;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 9 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated;
    }
    else
    {
        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_count = DI1_Pulse_Count;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF ] = DI1_Pulse_Count ;
    	//if(DI1_waveLength > EPROM_Frequent.Pulse_DI_frequency_time)
    	if(DI1_waveLength > actualPulse_DI_frequency_time)
    	{
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Freq = 0;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 1 ] = 0;
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated = 0;//DI1_Freq_fromWaveLength/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Const;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 4 ] = 0;//EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated;
    	}
    	else
    	{
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Freq = DI1_Freq_fromWaveLength;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 1 ] = DI1_Freq_fromWaveLength;
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated = DI1_Freq_fromWaveLength/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Const;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 4 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated;
    	}


        EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_count = DI2_Pulse_Count;
    	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 5 ] = DI2_Pulse_Count ;

    	if(DI2_waveLength > actualPulse_DI_frequency_time)
    	//if(DI2_waveLength > EPROM_Frequent.Pulse_DI_frequency_time)
    	{
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Freq = 0;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 6 ] = 0;
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated = 0;//DI2_Freq_fromWaveLength/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Const;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 9 ] = 0;//EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated;
    	}
    	else
    	{
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Freq = DI2_Freq_fromWaveLength;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 6 ] = DI2_Freq_fromWaveLength;
            EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated = DI2_Freq_fromWaveLength/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Const;
        	gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 9 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated;
    	}

    }

	if((DI1_Pulse_Count - EPROM_Frequent.DI1_Pulse)>100)
	{
		EPROM_Frequent.DI1_Pulse = DI1_Pulse_Count;
		flag_flashUpdateEPROM_Frequent = 1;
		flag_flashUpdateEPROM_Frequent_WaitCounter = 5;
	}
	if((DI2_Pulse_Count - EPROM_Frequent.DI2_Pulse)>100)
	{
		EPROM_Frequent.DI2_Pulse = DI2_Pulse_Count;
		flag_flashUpdateEPROM_Frequent = 1;
		flag_flashUpdateEPROM_Frequent_WaitCounter = 5;
	}

    //EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[2].Pulse_DI_count = DI3_Pulse_Count;
	//gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 10 ] = DI3_Pulse_Count ;
    //EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[2].Pulse_DI_Freq = DI3_Freq;
	//gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 11] = DI3_Freq ;
    //EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[2].Pulse_DI_Flow_Calculated = DI3_Freq/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[2].Pulse_DI_Const;
	//gFinalAnaValF[CONFIG_PARA_PULSE_DI_gFinalAnaValF + 14] = DI3_Freq/EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[2].Pulse_DI_Const ;

    return;
}

/****************************************************************************
 * Function :   get_ai_status
 * arg      :   none
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void get_ai_status()
{
	//char tBuffer[256] = {0,};

    //for(int tAi = 0; tAi < EPROM_General.AI_DI_DO_Detail.Total_Ai; ++tAi)
	for(int tAi = 0; tAi < MAX_PHYSICAL_AI; ++tAi)
    {
    	float tValue;

    	//tValue = AO_VAL_float[tAi];
		tValue = AI_Final_value[tAi];
        gFinalAnaValF[PHYSICAL_AI_gFinalAnaValF + tAi] = tValue;

        //printf("\nAI-%02d : %f", tAi+1, gFinalAnaValF[45 + tAi]);
        //sprintf(tBuffer, "AI%02d : %f\r\n", tAi+1, gFinalAnaValF[45 + tAi]);
        //WriteLog(pcbplc_logger, tBuffer, LOG_INFO);
    }

    return;
}

/****************************************************************************
 * Function :   set_ao
 * arg      :   iAoNum - valid AO number, iPercentage - valid range (0 to 100%)
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void set_ao(unsigned char iAoNum, float iPercentage)
{
//    char tBuffer[128] = {0, };
//
//    if(iAoNum >= gpcbplcCnfg.mMaxAoEnabled)
//    {
//        sprintf(tBuffer, "AoNum(%d) is more than enabled AO(%d)", iAoNum, gpcbplcCnfg.mMaxAoEnabled);
//        WriteLog(pcbplc_logger, tBuffer, LOG_ERROR);
//        return;
//    }
//
//    //set AO
//    int value = (unsigned int)iPercentage*100;
//    //value = 2*1000; //TODO: Comment it
//    //ioctl(gsfdao, WR_VALUE+iAoNum, (int32_t*) &value); //TODO: Need to discuss here.
//    //gsfdao[iAoNum].bits=value;
//    writeAoByAddress(iAoNum+1, value);
//    sleep(1);
//
//    return;
}

/****************************************************************************
 * Function :   set_do
 * arg      :   none
 * return   :   none
 * Remark   :   none
 * **************************************************************************/

void set_pulse_do_polarity()
{

    for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do ; ++tDoIdx)
    {

    	if(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_type==1)
    	{
			HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?1:0);
    	}
    	else if(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_type==2)
    	{
    		HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?1:0);
    	}
    }
}


//void set_dual_Do()
//{
//
//    for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do/2 ; tDoIdx++)
//    {
//		if(Dual_DO_Pulse_Stage[tDoIdx] == 1)  // Dual_DO : oprate Dual DO command
//		{
//			if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_type==3)&&(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2+1].Pulse_DO_type==3))
//			{
//				Dual_DO_Pulse_Stage[tDoIdx] = 2; // Dual_DO : First DO On
//				gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 2;
//
//				EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count = 1;
//
//				if(DO_PulseIsSet_array[tDoIdx*2] == 0)
//				{
//					if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count) > 0)
//					{
//						EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count--;
//						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (tDoIdx*2*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count;
//						DO_actual_PulseWidth[tDoIdx*2] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Width*EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Width_scale;
//						DO_PulseIsSet_array[tDoIdx*2] = 1;
//						DO_actual_PulseWidth_Count[tDoIdx*2] = 0;
//						DO_pulseIsSet |= (1 << (tDoIdx*2));
//						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx*2], DO_Pin_Array[tDoIdx*2], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);
//						osDelay(1234);
//					}
//					else
//					{
//						DO_pulseIsSet &= ~(1 << (tDoIdx*2));
//					}
//				}
//			}
//			else
//			{
//				Dual_DO_Pulse_Stage[tDoIdx] = 0; // Dual_DO : if Do are Not set as pair than stop opration
//				gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 0;
//				Dual_DO_pulseIsSet &= ~(1 << tDoIdx);
//			}
//		}
//    }
//}

void set_dual_Do()
{

    for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do/2 ; tDoIdx++)
    {
		if(Dual_DO_Pulse_Stage[tDoIdx] == 1)  // Dual_DO : oprate Dual DO command
		{
			if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_type==3)&&(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2+1].Pulse_DO_type==3))
			{
				Dual_DO_Pulse_Stage[tDoIdx] = 2; // Dual_DO : First DO On
				gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 2;

				//EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count = 1;

				//if(DO_PulseIsSet_array[tDoIdx*2] == 0)
				{
					//if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count) > 0)
					{
						//EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count--;
						gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (tDoIdx*2*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Count;
						DO_actual_PulseWidth[tDoIdx*2] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Width*EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_Width_scale;

						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx*2], DO_Pin_Array[tDoIdx*2], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);

						osDelay(DO_actual_PulseWidth[tDoIdx*2]);

						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx*2], DO_Pin_Array[tDoIdx*2], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_SET:GPIO_PIN_RESET);
						Dual_DO_Pulse_Stage[tDoIdx]=3;
						gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 3;

						osDelay(Dual_DO_actual_PulseWidth[tDoIdx]);

						Dual_DO_Pulse_Stage[tDoIdx]=4;
						gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 4;
						HAL_GPIO_WritePin(DO_GPIO_Port_Array[(tDoIdx*2)+1], DO_Pin_Array[(tDoIdx*2)+1], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);

						osDelay(DO_actual_PulseWidth[tDoIdx*2]);

						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx*2+1], DO_Pin_Array[tDoIdx*2+1], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_SET:GPIO_PIN_RESET);
						Dual_DO_Pulse_Stage[tDoIdx]=0;
						gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 0;

//						DO_PulseIsSet_array[tDoIdx*2] = 1;
//						DO_actual_PulseWidth_Count[tDoIdx*2] = 0;
//						DO_pulseIsSet |= (1 << (tDoIdx*2));
//						HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx*2], DO_Pin_Array[tDoIdx*2], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx*2].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);
//						osDelay(1234);
					}
//					else
//					{
//						DO_pulseIsSet &= ~(1 << (tDoIdx*2));
//					}
				}
			}
			else
			{
				Dual_DO_Pulse_Stage[tDoIdx] = 0; // Dual_DO : if Do are Not set as pair than stop opration
				gFinalAnaValF[CONFIG_PARA_PULSE_DUAL_DO_gFinalAnaValF + ((tDoIdx)*2)] = 0;
				//Dual_DO_pulseIsSet &= ~(1 << tDoIdx);
			}
		}
    }
}
/****************************************************************************
 * Function :   set_do
 * arg      :   none
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void set_do()
{
    for(int tDoIdx = 0 ; tDoIdx < EPROM_General.Pulse_DO_DI_Detail.Total_Pulse_Do ; ++tDoIdx)
    {
    	if(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_type==1)
    	{
			//if(DO_PulseIsSet_array[tDoIdx] == 0)
			{
				if((EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_Count) > 0)
				{
					EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_Count=0;
					gFinalAnaValF[CONFIG_PARA_PULSE_DO_gFinalAnaValF + (tDoIdx*5) + 3 ] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_Count;

					DO_actual_PulseWidth[tDoIdx] = EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_Width*EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_Width_scale;

					HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);

					osDelay(DO_actual_PulseWidth[tDoIdx]);

					HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?GPIO_PIN_SET:GPIO_PIN_RESET);

//					DO_PulseIsSet_array[tDoIdx] = 1;
//					DO_actual_PulseWidth_Count[tDoIdx] = 0;
//					osDelay(200);
//					DO_pulseIsSet |= (1 << tDoIdx);
//					HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);
//					osDelay(1234);
				}
//				else
//				{
//					DO_pulseIsSet &= ~(1 << tDoIdx);
//				}
			}
    	}
    	else if(EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_type==2)
    	{
			HAL_GPIO_WritePin(DO_GPIO_Port_Array[tDoIdx], DO_Pin_Array[tDoIdx], EPROM_General.Pulse_DO_DI_Detail.Pulse_DO_Detail[tDoIdx].Pulse_DO_polarity?GPIO_PIN_RESET:GPIO_PIN_SET);
    	}
    }
}


/****************************************************************************
 * Function :   get_rs485_parameters
 * arg      :   none
 * return   :   none
 * Remark   :   none
 * **************************************************************************/
void get_rs485_parameters()
{
    char tDebug[128] = {0, };
    int tIndex = 0;

    /* Save rs485 parameters on 300 location of gFinalAnaValF.
     * and support up to 300 parameters.
     */
    tIndex = 0;
    for(int tIdx = 0 ; tIdx < gpcbplcCnfg.mNumOfModbusConfig ; ++tIdx)  // Not used mNumOfModbusConfig variable
  //  for(int tIdx = 0 ; tIdx < 2 ; ++tIdx) //TODO : Hardcoded
    {
    	sprintf(tDebug, "\nModbus Para[%d] : %f\r\n",
    			START_IDX_MODBUS_PARA_TAG + tIndex,
				gFinalAnaValF[START_IDX_MODBUS_PARA_TAG + tIndex]);
    	WriteLog(pcbplc_logger, tDebug, LOG_INFO);

    	++tIndex;

        if(tIndex >= MAX_RS485_PARA)
        {
            sprintf(tDebug, "modbus parameter index(%d) is greater than supported max(%d)\r\n",
            		tIndex, MAX_RS485_PARA);
            WriteLog(pcbplc_logger, tDebug, LOG_ERROR);
            break;
        }
    }
}
