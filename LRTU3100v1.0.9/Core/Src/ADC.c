/*
 * ADC.c
 *
 *  Created on: Sep 28, 2022
 *      Author: maulin
 */


/**************************************************************************//**
 * Includes
 *****************************************************************************/

#include "main.h"
//#include"Configuration.h"
/**************************************************************************//**
 * Variable
 *****************************************************************************/

osThreadId ADC_TaskHandle;

char print[300];
uint32_t ADC_VAL_RAW[MAX_AI_CHANNEL],count_ADC=0;
float ADC_VAL_float[MAX_AI_CHANNEL];
float AO_VAL_float[MAX_AI_CHANNEL];
float AI_mA_VAL_float[MAX_AI_CHANNEL];
float AI_Final_value[MAX_AI_CHANNEL];
float ADC_Temp[MAX_AI_CHANNEL];
float Pin_voltage=0.0f;
//struct Save_Para EPROM;
int status,status1,status2,status3,status4,status5,status6,status7,status8,status9,status10,status11,status12,status13,status14,status15,status16;

/**************************************************************************//**
 * Function name 	: MX_ADC1_Init
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: # ADC1 Initialization Function
 *****************************************************************************/

void MX_ADC1_Init(void)
{
	ADC_MultiModeTypeDef multimode = {0};

	/** Common config */

	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_16B;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc1.Init.LowPowerAutoWait = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DiscontinuousConvMode = ENABLE;
	hadc1.Init.NbrOfDiscConversion = 1;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
	hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
	hadc1.Init.OversamplingMode = ENABLE;
	hadc1.Init.Oversampling.Ratio = 1024;
	hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_10;
	hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
	hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
	{
		Error_Handler();
	}
}

/**************************************************************************//**
 * Function name 	: ADC_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void ADC_start()
{
	osThreadDef(ADCTask, StartADCTask, osPriorityNormal, 0, 512); //512
	ADC_TaskHandle = osThreadCreate(osThread(ADCTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartADCTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void StartADCTask(void const * argument)
{
	float ADC_VAL_accumalate[MAX_AI_CHANNEL];
	unsigned char sample_count = 0;
	for(int i=0;i<MAX_AI_CHANNEL;i++)
	{
		ADC_VAL_accumalate[i] = 0;
	}

	for(;;)
	{
		#ifdef WATCH_DOG_ENABLE //MAU_EXCEPTION
			HAL_IWDG_Refresh(&hiwdg1);
		#endif
		//count_ADC++;
		count_ADC = 0;
		sample_count++;
		print_time();

		for(int i=0;i<MAX_AI_CHANNEL;i++)
		{
			//ADC1_calibration_CH(SCALE0TO10,i);
			ADC1_calibration_CH(EPROM_General.AI_DI_DO_Detail.AI_Detail[i].AI_ch_Type,i);
			ADC_calculation(AO_VAL_float[i],i);
			ADC_VAL_accumalate[i] = (float)ADC_Temp[i]+(float)ADC_VAL_accumalate[i];
		}

		//if(sample_count >= (EPROM_General.AI_DI_DO_Detail.Sample_time_to_collect_AI)*4) // TODO: make 5 as configurable from modbus
		if(sample_count >= 1)
		{
			for(int i=0;i<MAX_AI_CHANNEL;i++)
			{
				AI_Final_value[i] = ADC_VAL_accumalate[i]/sample_count;
				ADC_VAL_accumalate[i] = 0;
			}
			sample_count = 0;
		}

		// Remove this
//        get_di_status();
//        get_ai_status();
//        read_rtu_datetime();
		osDelay(1000);
	}
}

/**************************************************************************//**
 * Function name 	: ADC1_Select_CH3
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH3 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: ADC1_Select_CH5
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH5 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: ADC1_Select_CH7
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH7 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_7;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: ADC1_Select_CH4
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH4 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: ADC1_Select_CH18
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH18 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_18;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: ADC1_Select_CH19
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_Select_CH19 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	*/
	sConfig.Channel = ADC_CHANNEL_19;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_32CYCLES_5;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	sConfig.OffsetSignedSaturation = DISABLE;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
	  Error_Handler();
	}
}
/**************************************************************************//**
 * Function name 	: Get_ADC_Value
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void Get_ADC_Value(int mChannelType,int Channel)
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	ADC_VAL_RAW[Channel] = HAL_ADC_GetValue(&hadc1);

	Adc_ConvertBin2Engg(Channel,ADC_VAL_RAW[Channel], &AO_VAL_float[Channel]);


	if(AO_VAL_float[3]<2.5)  // Due to leakage voltage set PV cell voltage accordingly
	{
		AO_VAL_float[3]=0;
	}

	HAL_ADC_Stop(&hadc1);

}
/**************************************************************************//**
 * Function name 	: ADC1_calibration
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
void ADC1_calibration (void)
{
	HAL_ADC_Stop(&hadc1);
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) /* Run the ADC calibration in single-ended mode */
	{
	  Error_Handler(); /* Calibration Error */
	}

	HAL_Delay(10);

//	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK) /* Run the ADC calibration in single-ended mode */
//	{
//	  Error_Handler(); /* Calibration Error */
//	}
//	HAL_Delay(10);
}
/**************************************************************************//**
 * Function name 	: ADC1_calculation
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

//void ADC_calculation(float Ana_value, unsigned int Channel)
//{
//	float Hval,Diff,rval;
//
//	if((EPROM.scaleType[Channel]) == SCALE4TO20)
//	{
//		Hval = 	(Ana_value - 0.999);
//
//		if(Hval < 0)
//		{
//			Hval = 0;
//		}
//		Diff = ((EPROM.scaleHi[Channel]) - (EPROM.scaleLo[Channel]));
//		rval =	((Hval * Diff)/4.0);
//		Ana_value = ((EPROM.scaleLo[Channel]) + rval);
//
//		if((EPROM.calS[Channel]) > 0)
//		{
//			Ana_value = ((Ana_value * (Diff/(EPROM.calS[Channel]))) + (EPROM.calZ[Channel]));
//		}
//		AI_Final_value[Channel] = Ana_value;
//	}
//	else
//	{
//		Diff = (EPROM.scaleHi[Channel])-(EPROM.scaleLo[Channel]);
//		Ana_value = Ana_value * (Diff/5);
//
//		if((EPROM.calS[Channel]) > 0)
//		{
//			Ana_value = ((Ana_value * (Diff/(EPROM.calS[Channel]))) + (EPROM.calZ[Channel]));
//		}
//		AI_Final_value[Channel] = Ana_value;
//	}
//}
void ADC1_calibration_CH(int mChannelType,int Channel)
{
	switch(Channel)
	{
		case 0:
			ADC1_Select_CH18();//ok
			break;
		case 1:
			ADC1_Select_CH19();//ok
			break;
		case 2:
			ADC1_Select_CH3();//ok
			break;
		case 3:
			ADC1_Select_CH7();//ok
			break;
		case 4:
			ADC1_Select_CH4();//ok
			break;
		case 5:
			ADC1_Select_CH5();//0k
			break;
	}
	Get_ADC_Value(mChannelType,Channel);


}
/**************************************************************************//**
 * Function name 	: ADC1_calculation
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/

void ADC_calculation(float Ana_value, unsigned int Channel)
{
	float rval,ADCrawvalue;

	if((EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].AI_ch_Type) == SCALE4TO20)
	{
		float tChannelLow=4, tChannelHigh = 20;
		float engscal_fac = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		float Range_diff = tChannelHigh - tChannelLow;
		float engv;

		if(Ana_value > tChannelLow)
		{
			engv = Ana_value - tChannelLow;
		}
		else
		{
			engv = 0;
		}

		ADCrawvalue = ((engscal_fac / Range_diff) * engv) + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;

		ADCrawvalue = (ADCrawvalue * (engscal_fac / EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS)) + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ;

		if(ADCrawvalue > EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi) // Add limit max and min of scaling https://cimcondigital.atlassian.net/browse/IRTU6000PP-49
		{
			ADCrawvalue = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi;
		}
		if(ADCrawvalue < EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo)
		{
			ADCrawvalue = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		}

		//AI_Final_value[Channel] = ADCrawvalue;
		ADC_Temp[Channel] = ADCrawvalue;
		#if 0
		if(Ana_value >= EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ )
		{
			float engscal_fac = (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo) / (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ);
			float engv = Ana_value - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ;
			ADCrawvalue = engv * engscal_fac;
			ADCrawvalue = ADCrawvalue + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		}
		else
		{
			ADCrawvalue = 0;
		}

		AI_Final_value[Channel] = ADCrawvalue;
		#endif
		#if 0
		//Hval = 	(Ana_value - 0.999);
		Hval = 	(Ana_value);

		if(Hval < 0)
		{
			Hval = 0;
		}

		Diff = ((EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi) - (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo));
		//rval =	((Hval * Diff)/4.0);
		rval =	((Hval * Diff)/(20-0));
		//Ana_value = ((EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo) + rval);
		Ana_value = rval;

		if((EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS) > 0)
		{
			Ana_value = ((Ana_value * (Diff/(EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS))) + (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ));
		}
		AI_Final_value[Channel] = Ana_value;
		#endif
	}
	else
	{
		float tChannelLow=0, tChannelHigh = 30;
		float engscal_fac = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi -
				EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		float Range_diff = tChannelHigh - tChannelLow;
		float engv;
		if(Ana_value > tChannelLow)
		{
			engv = Ana_value - tChannelLow;
		}
		else
		{
			engv = 0;
		}

		ADCrawvalue = ((engscal_fac / Range_diff) * engv) + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;

		ADCrawvalue = (ADCrawvalue * (engscal_fac / EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS)) + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ;

		if(ADCrawvalue > EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi) // Add limit max and min of scaling https://cimcondigital.atlassian.net/browse/IRTU6000PP-49
		{
			ADCrawvalue = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi;
		}
		if(ADCrawvalue < EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo)
		{
			ADCrawvalue = EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		}

		//AI_Final_value[Channel] = ADCrawvalue;
		ADC_Temp[Channel] = ADCrawvalue;

		#if 0
		if((Ana_value >= EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ) && (Ana_value <= EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS))
		{
			float engscal_fac = (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo) / (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ);
			float engv = Ana_value - EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ;
			ADCrawvalue = engv * engscal_fac;
			ADCrawvalue = ADCrawvalue + EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo;
		}
		else
		{
			ADCrawvalue = 0;
		}

		AI_Final_value[Channel] = ADCrawvalue;
		#endif
		#if 0
		Diff = (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleHi)-(EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].scaleLo);
		Ana_value = Ana_value * (Diff/10);

		if((EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS) > 0)
		{
			Ana_value = ((Ana_value * (Diff/(EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calS))) + (EPROM_General.AI_DI_DO_Detail.AI_Detail[Channel].calZ));
		}
		AI_Final_value[Channel] = Ana_value;
		#endif
	}

//	if((EPROM.scaleType[Channel]) == SCALE4TO20)
//	{
//		Hval = 	(Ana_value - 0.999);
//
//		if(Hval < 0)
//		{
//			Hval = 0;
//		}
//		Diff = ((EPROM.scaleHi[Channel]) - (EPROM.scaleLo[Channel]));
//		rval =	((Hval * Diff)/4.0);
//		Ana_value = ((EPROM.scaleLo[Channel]) + rval);
//
//		if((EPROM.calS[Channel]) > 0)
//		{
//			Ana_value = ((Ana_value * (Diff/(EPROM.calS[Channel]))) + (EPROM.calZ[Channel]));
//		}
//		AI_Final_value[Channel] = Ana_value;
//	}
//	else
//	{
//		Diff = (EPROM.scaleHi[Channel])-(EPROM.scaleLo[Channel]);
//		Ana_value = Ana_value * (Diff/10);
//
//		if((EPROM.calS[Channel]) > 0)
//		{
//			Ana_value = ((Ana_value * (Diff/(EPROM.calS[Channel]))) + (EPROM.calZ[Channel]));
//		}
//		AI_Final_value[Channel] = Ana_value;
//	}
}

//void Adc_ConvertBin2Engg(int mChannelType, unsigned short int  mAiHighCal,  unsigned short int  mAiMidCal,unsigned short int  mAiLowCal, unsigned short int mAiLowMidCal, unsigned short int tAnalogDataBin, float *pAnalogDataEngg)
//{
//    switch(mChannelType)
//    {
//        case SCALE4TO20:
//        {
//            if(tAnalogDataBin > mAiMidCal)
//            {
//                (*pAnalogDataEngg) = 10 + ((float)((tAnalogDataBin - mAiMidCal) * ((20-10)) ) /
//                                           (mAiHighCal - mAiMidCal));
//            }
//            else if((tAnalogDataBin > mAiLowMidCal) && (tAnalogDataBin <= mAiMidCal))
//            {
//                (*pAnalogDataEngg) = 4 + ((float)((tAnalogDataBin - mAiLowMidCal) * (10-4)) /
//                                          (mAiMidCal - mAiLowMidCal));
//            }
//            else if((tAnalogDataBin > mAiLowCal) && (tAnalogDataBin <= mAiLowMidCal))
//            {
//                (*pAnalogDataEngg) = 0 + ((float)((tAnalogDataBin - mAiLowCal) * ((4-0)) ) /
//                                          (mAiLowMidCal - mAiLowCal));
//            }
//            else if(tAnalogDataBin <= mAiLowCal)
//			{
//				(*pAnalogDataEngg) = 0;
//			}
//            break;
//        }
//        case SCALE0TO10:
//        {
//        	if(tAnalogDataBin < mAiLowMidCal) // 0v to 0.1v calibration
//			{
//				(*pAnalogDataEngg) = 0 + ((float) ((tAnalogDataBin - mAiLowCal) * (0.5 - 0))) /
//										 (mAiLowMidCal - mAiLowCal);
//			}
//			else if(tAnalogDataBin > mAiLowMidCal && tAnalogDataBin < mAiMidCal)// 0.1v to 1v calibration
//			{
//				(*pAnalogDataEngg) = 0.5 + ((float) ((tAnalogDataBin - mAiLowMidCal) * (5 - 0.5))) /
//															  (mAiMidCal - mAiLowMidCal);
//			}
//			else if(tAnalogDataBin > mAiLowMidCal)// 5v to 10v calibration
//			{
//				(*pAnalogDataEngg) = 5 + ((float) ((tAnalogDataBin - mAiMidCal) * (10 - 5))) /
//											  (mAiHighCal - mAiMidCal);
//			}
//			else
//			{
//				//(*pAnalogDataEngg) = 5;
//				if(tAnalogDataBin == mAiLowMidCal) // 0v to 0.1v calibration
//				{
//					(*pAnalogDataEngg) = 0.5;
//				}
//				else if(tAnalogDataBin == mAiMidCal)// 0.1v to 5v calibration
//				{
//					(*pAnalogDataEngg) = 5;
//				}
//				else if(tAnalogDataBin == mAiHighCal)// 0.1v to 5v calibration
//				{
//					(*pAnalogDataEngg) = 10;
//				}
//			}
//
//        	///////////////////////////////
////			if(tAnalogDataBin < mAiLowMidCal) // 0v to 0.1v calibration
////			{
////				(*pAnalogDataEngg) = 0 + ((float) ((tAnalogDataBin - mAiLowCal) * (0.1 - 0))) /
////										 (mAiLowMidCal - mAiLowCal);
////			}
////			else if(tAnalogDataBin > mAiLowMidCal && tAnalogDataBin < mAiMidCal)// 0.1v to 5v calibration
////			{
////				(*pAnalogDataEngg) = 0.1 + ((float) ((tAnalogDataBin - mAiLowMidCal) * (5 - 0.1))) /
////															  (mAiMidCal - mAiLowMidCal);
////			}
////			else if(tAnalogDataBin > mAiMidCal)// 5v to 10v calibration
////			{
////				(*pAnalogDataEngg) = 5 + ((float) ((tAnalogDataBin - mAiMidCal) * (10 - 5))) /
////											  (mAiHighCal - mAiMidCal);
////			}
////			else
////			{
////				//(*pAnalogDataEngg) = 5;
////				if(tAnalogDataBin == mAiLowMidCal) // 0v to 0.1v calibration
////				{
////					(*pAnalogDataEngg) = 0.1;
////				}
////				else if(tAnalogDataBin == mAiMidCal)// 0.1v to 5v calibration
////				{
////					(*pAnalogDataEngg) = 5;
////				}
////				else if(tAnalogDataBin == mAiHighCal)// 0.1v to 5v calibration
////				{
////					(*pAnalogDataEngg) = 10;
////				}
////			}
//
//            break;
//        }
//        default:
//        {
//            //printf("Invalid channel:%d type:%c at %s\n", iChannelNum, mChannelType, __func__ );
//        }
//    }
//
//}

void Adc_ConvertBin2Engg(int mChannelno,unsigned short int tAnalogDataBin, float *pAnalogDataEngg)
{
	float mAiHighCal,mAiMidCal,mAiLowMidCal,mAiLowCal,mAiHighCal_Point,mAiMidCal_Point,mAiLowMidCal_Point,mAiLowCal_Point;

    switch(EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].AI_ch_Type)
    {
        case SCALE4TO20:
        {
        	mAiHighCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiHighCal_mA;
        	mAiMidCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiMidCal_mA;
        	mAiLowMidCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowMidCal_mA;
        	mAiLowCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowCal_mA;

        	mAiHighCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiHighCal_mA_Point;
        	mAiMidCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiMidCal_mA_Point;
        	mAiLowMidCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowMidCal_mA_Point;
        	mAiLowCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowCal_mA_Point;

            if(tAnalogDataBin > mAiMidCal)
            {
                (*pAnalogDataEngg) = mAiMidCal_Point + ((float)((tAnalogDataBin - mAiMidCal) * ((mAiHighCal_Point-mAiMidCal_Point)) ) /
                                           (mAiHighCal - mAiMidCal));
            }
            else if((tAnalogDataBin > mAiLowMidCal) && (tAnalogDataBin <= mAiMidCal))
            {
                (*pAnalogDataEngg) = mAiLowMidCal_Point + ((float)((tAnalogDataBin - mAiLowMidCal) * (mAiMidCal_Point-mAiLowMidCal_Point)) /
                                          (mAiMidCal - mAiLowMidCal));
            }
            else if((tAnalogDataBin > mAiLowCal) && (tAnalogDataBin <= mAiLowMidCal))
            {
                (*pAnalogDataEngg) = mAiLowCal_Point + ((float)((tAnalogDataBin - mAiLowCal) * ((mAiLowMidCal_Point-mAiLowCal_Point)) ) /
                                          (mAiLowMidCal - mAiLowCal));
            }
            else if(tAnalogDataBin <= mAiLowCal)
			{
				(*pAnalogDataEngg) = 0;
			}
            break;
        }
//        if(mChannelno!=3&&mChannelno!=4)
//        {
        case SCALE0TO20:
        {
        	mAiHighCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiHighCal_V;
        	mAiMidCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiMidCal_V;
        	mAiLowMidCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowMidCal_V;
        	mAiLowCal = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowCal_V;

        	mAiHighCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiHighCal_V_Point;
        	mAiMidCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiMidCal_V_Point;
        	mAiLowMidCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowMidCal_V_Point;
        	mAiLowCal_Point = EPROM_General.AI_DI_DO_Detail.AI_Detail[mChannelno].mAiLowCal_V_Point;

        	if(tAnalogDataBin < mAiLowMidCal) // 0v to 0.1v calibration
			{
				(*pAnalogDataEngg) = mAiLowCal_Point + ((float) ((tAnalogDataBin - mAiLowCal) * (mAiLowMidCal_Point - mAiLowCal_Point))) /
										 (mAiLowMidCal - mAiLowCal);
			}
			else if(tAnalogDataBin > mAiLowMidCal && tAnalogDataBin < mAiMidCal)// 0.1v to 1v calibration
			{
				(*pAnalogDataEngg) = mAiLowMidCal_Point + ((float) ((tAnalogDataBin - mAiLowMidCal) * (mAiMidCal_Point - mAiLowMidCal_Point))) /
															  (mAiMidCal - mAiLowMidCal);
			}
			else if(tAnalogDataBin > mAiLowMidCal)// 5v to 10v calibration
			{
				(*pAnalogDataEngg) = mAiMidCal_Point + ((float) ((tAnalogDataBin - mAiMidCal) * (mAiHighCal_Point - mAiMidCal_Point))) /
											  (mAiHighCal - mAiMidCal);
			}
			else
			{
				//(*pAnalogDataEngg) = 5;
				if(tAnalogDataBin == mAiLowMidCal) // 0v to 0.1v calibration
				{
					(*pAnalogDataEngg) = mAiLowMidCal_Point;
				}
				else if(tAnalogDataBin == mAiMidCal)// 0.1v to 5v calibration
				{
					(*pAnalogDataEngg) = mAiMidCal_Point;
				}
				else if(tAnalogDataBin == mAiHighCal)// 0.1v to 5v calibration
				{
					(*pAnalogDataEngg) = mAiHighCal_Point;
				}
			}
            break;
        }
//        }
        default:
        {
            //printf("Invalid channel:%d type:%c at %s\n", iChannelNum, mChannelType, __func__ );
        }
    }

}
