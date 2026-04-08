/*
 * DIDO.c
 *
 *  Created on: Dec 28, 2022
 *      Author: SanketP
 */

#include "main.h"
#include "DIDO.h"
osThreadId DIDO_TaskHandle;
unsigned char bit_status[16]={0};
float DO_Final_value[MAX_DO_CHANNEL];
float DI_Final_value[MAX_DI_CHANNEL];
unsigned char test_DI_Final_value[MAX_DI_CHANNEL];
unsigned char  DO_KEY_Array[32];
unsigned char Set_to_switch,BLE_led_status,BLE_led_status1,GPS_led_status,GPS_led_status1;
int count_DO;
volatile unsigned int DI1_Pulse_Count=0,DI2_Pulse_Count=0,DI3_Pulse_Count=0,DI4_Pulse_Count=0,DI1_Pulse_Count_for_Frequency=0,DI2_Pulse_Count_for_Frequency=0;
float DI1_Freq=0.0,DI2_Freq=0.0;
float DI1_Freq_fromWaveLength=0.0000,DI2_Freq_fromWaveLength=0.0000;
uint64_t actualPulse_DI_frequency_time=0;
/* DO4 repeated to fill the space of spare and keep PLC same as of 3000*/
GPIO_TypeDef* DO_GPIO_Port_Array[] = {DO1_GPIO_Port, DO2_GPIO_Port, DO3_GPIO_Port, DO4_GPIO_Port, DO4_GPIO_Port,
		DO4_GPIO_Port, DO5_GPIO_Port,DO6_GPIO_Port, DO7_GPIO_Port, DO8_GPIO_Port, DO9_GPIO_Port, DO10_GPIO_Port,
		DO11_GPIO_Port, DO12_GPIO_Port, DO13_GPIO_Port, DO14_GPIO_Port, DO15_GPIO_Port,DO16_GPIO_Port, DO17_GPIO_Port,
		DO18_GPIO_Port, DO19_GPIO_Port, DO20_GPIO_Port,
		/*DO21_GPIO_Port, DO22_GPIO_Port, DO23_GPIO_Port, DO24_GPIO_Port, DO25_GPIO_Port, DO26_GPIO_Port*/};
uint16_t DO_Pin_Array[] = {DO1_Pin, DO2_Pin, DO3_Pin, DO4_Pin, DO4_Pin, DO4_Pin, DO5_Pin, DO6_Pin, DO7_Pin,
		DO8_Pin, DO9_Pin, DO10_Pin, DO11_Pin, DO12_Pin, DO13_Pin, DO14_Pin, DO15_Pin, DO16_Pin, DO17_Pin,
		DO18_Pin, DO19_Pin, DO20_Pin,
		/*DO21_Pin, DO22_Pin, DO23_Pin, DO24_Pin, DO25_Pin, DO26_Pin*/};

char Dual_DO_Pulse_Stage[15];
char DO_PulseIsSet_array[30];
unsigned short int DO_actual_PulseWidth[30];
unsigned short int DO_actual_PulseWidth_Count[30];
unsigned short int Dual_DO_actual_PulseWidth[30];
unsigned short int Dual_DO_actual_PulseWidth_Count[30];

extern char dispState;

/**************************************************************************//**
 * Function name 	: DO_DI_start
 * arguments		: 1)
 * return 		 	: no return type
 * Note				: #
 *****************************************************************************/

void DIDO_start()
{
	osThreadDef(DIDOTask, StartDIDOTask, osPriorityNormal, 0, 512*8); //512
	DIDO_TaskHandle = osThreadCreate(osThread(DIDOTask), NULL);
}

/**************************************************************************//**
 * Function name 	: StartDIDOTask
 * arguments		: 1)
 * return 		 	:
 * Note				:
 * 					:
 * 					:
 *****************************************************************************/
xTimerHandle PTHandle;
void StartDIDOTask(void const * argument)
{
	osDelay(1000);
	for(;;)
	{
		if(Pro_Application_flag==0)
		{
			osDelay(1000);
			lcd_clear();
			switch(dispState)
			{
			case 0:
				lcd_set_cursor(1,0);
				lcd_display_string("LoRA dBm:");
				lcd_set_cursor(2,0);
				lcd_display_string("LoRA Com:");
				lcd_set_cursor(1,9);
				lcd_float_print((float)LoRa_Modem.lora_RSSI,0);
				lcd_set_cursor(2,9);
				if(LoraOTA.ota_state)
					lcd_display_string("In OTA");
				else if(NoConnNodes)
					lcd_display_string("OK");
				else
					lcd_display_string("ERROR");
				break;
			case 1:
				lcd_set_cursor(1,0);
				lcd_display_string("Bat %:");
				lcd_set_cursor(2,0);
				lcd_display_string("PV Vo:");
				lcd_set_cursor(1,6);
				float percent;
				if(!DI_Final_value[4])
					percent = 0;
				else
				{
					percent = 100-((14.1-AO_VAL_float[4])*80/2.7);	// battery percentage
					if(percent > 100)
						percent = 100;
					else if(percent < 0)
						percent = 0;
				}
				lcd_float_print(percent,0);
				lcd_set_cursor(2,6);
				lcd_float_print(AO_VAL_float[3],2);
				break;
			case 2:
				lcd_set_cursor(1,0);
				lcd_display_string("Valve: 12345678");
				lcd_set_cursor(2,0);
				lcd_display_string("Status:");

				union_Datatypes D1;
				char buffer[9];   // 8 bits + null
				D1.sh = (uint16_t) gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+16];
				for (int i = 0; i < 8; i++)	// SOV 1-8
			    {
			        buffer[i] = ((D1.sh >> (i+6)) & 1) ? '1' : '0';
			    }
			    buffer[8] = '\0';
			    lcd_display_string(buffer);
				break;
			case 3:
				lcd_set_cursor(1,0);
				lcd_display_string("FR1 L/S:");
				lcd_set_cursor(2,0);
				lcd_display_string("TF1 m3 :");
				lcd_set_cursor(1,8);
				lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[0].Pulse_DI_Flow_Calculated,3);
				lcd_set_cursor(2,8);
				lcd_float_print(gFinalAnaValF[712],2);
				break;
			case 4:
				lcd_set_cursor(1,0);
				lcd_display_string("FR2 L/S:");
				lcd_set_cursor(2,0);
				lcd_display_string("TF2 m3 :");
				lcd_set_cursor(1,8);
				lcd_float_print(EPROM_General.Pulse_DO_DI_Detail.Pulse_DI_Detail[1].Pulse_DI_Flow_Calculated,3);
				lcd_set_cursor(2,8);
				lcd_float_print(gFinalAnaValF[713],2);
				break;
			case 5:
				lcd_set_cursor(1,0);
				lcd_display_string("S1 Bar:");
				lcd_set_cursor(2,0);
				lcd_display_string("S2 Bar:");
				lcd_set_cursor(1,7);
				lcd_float_print(gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+1],2);
				lcd_set_cursor(2,7);
				lcd_float_print(gFinalAnaValF[START_IDX_GEN_ANA_PARA_TAG+2],2);
				break;
			case 6:
				if (EPROM_General.Modem_EC200_presence)
				{
					char dispString[6];
					lcd_set_cursor(1,0);
					lcd_display_string("GSM Signal:");
					lcd_set_cursor(2,0);
					lcd_display_string("LoRA Nodes:");
					lcd_set_cursor(1,11);
					lcd_float_print((float) Modem_gsm_rssi,0);
					lcd_set_cursor(2,11);
					sprintf(dispString, "%d/%d", NoConnNodes, maxConnNode);
					lcd_display_string(dispString);
				}
				else
					dispState = 0;
				break;
			}
		}
	}

}


/* Timer Call back Function */
/*******************************************************************************/
volatile unsigned int DO_pulseIsSet=0;
volatile unsigned int Dual_DO_pulseIsSet=0;

char DO1_PulseIsSet = 0;
unsigned short int DO1_actual_PulseWidth = 0;
unsigned short int DO1_actual_PulseWidth_Count = 0;

char DO2_PulseIsSet = 0;
unsigned short int DO2_actual_PulseWidth = 0;
unsigned short int DO2_actual_PulseWidth_Count = 0;

char DO3_PulseIsSet = 0;
unsigned short int DO3_actual_PulseWidth = 0;
unsigned short int DO3_actual_PulseWidth_Count = 0;

char DO4_PulseIsSet = 0;
unsigned short int DO4_actual_PulseWidth = 0;
unsigned short int DO4_actual_PulseWidth_Count = 0;

char DO5_PulseIsSet = 0;
unsigned short int DO5_actual_PulseWidth = 0;
unsigned short int DO5_actual_PulseWidth_Count = 0;

char DO6_PulseIsSet = 0;
unsigned short int DO6_actual_PulseWidth = 0;
unsigned short int DO6_actual_PulseWidth_Count = 0;

char DO7_PulseIsSet = 0;
unsigned short int DO7_actual_PulseWidth = 0;
unsigned short int DO7_actual_PulseWidth_Count = 0;

char DO8_PulseIsSet = 0;
unsigned short int DO8_actual_PulseWidth = 0;
unsigned short int DO8_actual_PulseWidth_Count = 0;

char DO9_PulseIsSet = 0;
unsigned short int DO9_actual_PulseWidth = 0;
unsigned short int DO9_actual_PulseWidth_Count = 0;

char DO10_PulseIsSet = 0;
unsigned short int DO10_actual_PulseWidth = 0;
unsigned short int DO10_actual_PulseWidth_Count = 0;

char DO11_PulseIsSet = 0;
unsigned short int DO11_actual_PulseWidth = 0;
unsigned short int DO11_actual_PulseWidth_Count = 0;

char DO12_PulseIsSet = 0;
unsigned short int DO12_actual_PulseWidth = 0;
unsigned short int DO12_actual_PulseWidth_Count = 0;

char DO13_PulseIsSet = 0;
unsigned short int DO13_actual_PulseWidth = 0;
unsigned short int DO13_actual_PulseWidth_Count = 0;

char DO14_PulseIsSet = 0;
unsigned short int DO14_actual_PulseWidth = 0;
unsigned short int DO14_actual_PulseWidth_Count = 0;

char DO15_PulseIsSet = 0;
unsigned short int DO15_actual_PulseWidth = 0;
unsigned short int DO15_actual_PulseWidth_Count = 0;

char DO16_PulseIsSet = 0;
unsigned short int DO16_actual_PulseWidth = 0;
unsigned short int DO16_actual_PulseWidth_Count = 0;

char DO17_PulseIsSet = 0;
unsigned short int DO17_actual_PulseWidth = 0;
unsigned short int DO17_actual_PulseWidth_Count = 0;

char DO18_PulseIsSet = 0;
unsigned short int DO18_actual_PulseWidth = 0;
unsigned short int DO18_actual_PulseWidth_Count = 0;

char DO19_PulseIsSet = 0;
unsigned short int DO19_actual_PulseWidth = 0;
unsigned short int DO19_actual_PulseWidth_Count = 0;

char DO20_PulseIsSet = 0;
unsigned short int DO20_actual_PulseWidth = 0;
unsigned short int DO20_actual_PulseWidth_Count = 0;

char DO21_PulseIsSet = 0;
unsigned short int DO21_actual_PulseWidth = 0;
unsigned short int DO21_actual_PulseWidth_Count = 0;

char DO22_PulseIsSet = 0;
unsigned short int DO22_actual_PulseWidth = 0;
unsigned short int DO22_actual_PulseWidth_Count = 0;

char DO23_PulseIsSet = 0;
unsigned short int DO23_actual_PulseWidth = 0;
unsigned short int DO23_actual_PulseWidth_Count = 0;

char DO24_PulseIsSet = 0;
unsigned short int DO24_actual_PulseWidth = 0;
unsigned short int DO24_actual_PulseWidth_Count = 0;

char DO25_PulseIsSet = 0;
unsigned short int DO25_actual_PulseWidth = 0;
unsigned short int DO25_actual_PulseWidth_Count = 0;

char DO26_PulseIsSet = 0;
unsigned short int DO26_actual_PulseWidth = 0;
unsigned short int DO26_actual_PulseWidth_Count = 0;

unsigned short int DI_Frequency_Counter=0;

unsigned int DI1_waveLength = 0;
unsigned int DI2_waveLength = 0;



void pro_checkDIDOState()
{
	unsigned char i,pro_Do_State_off[26],pro_Do_State_on[26],pro_Do_State_off1[26];

	if(pro_DO_DI_TestFinish == 0 && EPROM_General.slot_id !=0)
	{
		for(unsigned char i=1;i<=4;i++)
		{
			DoTurnOn(i); // it will make pin low
		}

		osDelay(2000);
		ScanDI();
		for(i=0;i<4;i++)
		{
			pro_Do_State_off[i]=test_DI_Final_value[i];
		}

		for(i=1;i<=4;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(2000);
		ScanDI();
		for(i=0;i<4;i++)
		{
			pro_Do_State_on[i]=test_DI_Final_value[i];
		}

		for(unsigned char i=5;i<=12;i++)
		{
			DoTurnOn(i); // it will make pin low
		}
		osDelay(2000);
		ScanDI();

			pro_Do_State_off[4]=test_DI_Final_value[0];
			pro_Do_State_off[5]=test_DI_Final_value[0];

			pro_Do_State_off[6]=test_DI_Final_value[1];
			pro_Do_State_off[7]=test_DI_Final_value[1];

			pro_Do_State_off[8]=test_DI_Final_value[2];
			pro_Do_State_off[9]=test_DI_Final_value[2];

			pro_Do_State_off[10]=test_DI_Final_value[3];
			pro_Do_State_off[11]=test_DI_Final_value[3];

		for(i=5;i<=12;i++)
		{
			DoTurnOff(i); // it will make pin high
		}
		osDelay(2000);
		ScanDI();


			pro_Do_State_on[4]=test_DI_Final_value[0];
			pro_Do_State_on[5]=test_DI_Final_value[0];

			pro_Do_State_on[6]=test_DI_Final_value[1];
			pro_Do_State_on[7]=test_DI_Final_value[1];

			pro_Do_State_on[8]=test_DI_Final_value[2];
			pro_Do_State_on[9]=test_DI_Final_value[2];

			pro_Do_State_on[10]=test_DI_Final_value[3];
			pro_Do_State_on[11]=test_DI_Final_value[3];

		for(i=5;i<=12;i++)
		{
			Do_RESET_SET(i);
		}
		osDelay(2000);
		ScanDI();
			pro_Do_State_off1[4]=test_DI_Final_value[0];
			pro_Do_State_off1[5]=test_DI_Final_value[0];

			pro_Do_State_off1[6]=test_DI_Final_value[1];
			pro_Do_State_off1[7]=test_DI_Final_value[1];

			pro_Do_State_off1[8]=test_DI_Final_value[2];
			pro_Do_State_off1[9]=test_DI_Final_value[2];

			pro_Do_State_off1[10]=test_DI_Final_value[3];
			pro_Do_State_off1[11]=test_DI_Final_value[3];

		for(i=5;i<=12;i++)
		{
			DoTurnOff(i);
		}
		osDelay(1000);

		for(unsigned char i=13;i<=20;i++)
				{
					DoTurnOn(i); // it will make pin low
				}
				osDelay(2000);
				ScanDI();

					pro_Do_State_off[12]=test_DI_Final_value[0];
					pro_Do_State_off[13]=test_DI_Final_value[0];

					pro_Do_State_off[14]=test_DI_Final_value[1];
					pro_Do_State_off[15]=test_DI_Final_value[1];

					pro_Do_State_off[16]=test_DI_Final_value[2];
					pro_Do_State_off[17]=test_DI_Final_value[2];

					pro_Do_State_off[18]=test_DI_Final_value[3];
					pro_Do_State_off[19]=test_DI_Final_value[3];

				for(i=13;i<=20;i++)
				{
					DoTurnOff(i); // it will make pin high
				}
				osDelay(2000);
				ScanDI();


					pro_Do_State_on[12]=test_DI_Final_value[0];
					pro_Do_State_on[13]=test_DI_Final_value[0];

					pro_Do_State_on[14]=test_DI_Final_value[1];
					pro_Do_State_on[15]=test_DI_Final_value[1];

					pro_Do_State_on[16]=test_DI_Final_value[2];
					pro_Do_State_on[17]=test_DI_Final_value[2];

					pro_Do_State_on[18]=test_DI_Final_value[3];
					pro_Do_State_on[19]=test_DI_Final_value[3];

				for(i=13;i<=20;i++)
				{
					Do_RESET_SET(i);
				}
				osDelay(2000);
				ScanDI();
					pro_Do_State_off1[12]=test_DI_Final_value[0];
					pro_Do_State_off1[13]=test_DI_Final_value[0];

					pro_Do_State_off1[14]=test_DI_Final_value[1];
					pro_Do_State_off1[15]=test_DI_Final_value[1];

					pro_Do_State_off1[16]=test_DI_Final_value[2];
					pro_Do_State_off1[17]=test_DI_Final_value[2];

					pro_Do_State_off1[18]=test_DI_Final_value[3];
					pro_Do_State_off1[19]=test_DI_Final_value[3];

				for(i=13;i<=20;i++)
				{
					DoTurnOff(i);
				}
				osDelay(1000);

				HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_SET);
				osDelay(2000);
				ScanDI();
				pro_Do_State_off[20]=test_DI_Final_value[0];

				HAL_GPIO_WritePin(VOUT_EN_GPIO_Port, VOUT_EN_Pin, GPIO_PIN_RESET);
				osDelay(2000);
				ScanDI();
				pro_Do_State_on[20]=test_DI_Final_value[0];

		for(i=0;i<4;i++)
		{
			pro_DI_State[i]=1;
		}

		for(i=0;i<=3;i++)
		{
			if(((pro_Do_State_off[i] == 0)&&(pro_Do_State_on[i] == 1)))
			{
				pro_DO_State[i] = 1;
			}
			else
			{
				pro_DO_State[i] = 0;
				if(i<4)
				{
					pro_DI_State[i] = 0;
				}
			}
		}

		for(i=4;i<=19;i++)
		{
			if(((pro_Do_State_off[i] == 0)&&(pro_Do_State_on[i] == 1)&&(pro_Do_State_off1[i] == 0)))
			{
				pro_DO_State[i] = 1;
			}
			else
			{
				pro_DO_State[i] = 0;

			}
		}

			if(((pro_Do_State_off[20] == 0)&&(pro_Do_State_on[20] == 1)))
			{
				pro_DO_State[20] = 1;
			}
			else
			{
				pro_DO_State[20] = 0;
			}
			pro_DO_DI_TestFinish = 1;
			flagLORAPubLogData=1;

	}

	if(EPROM_General.slot_id==0 || key_LED_flag == 1)
	{

		key_LED_flag=0;
		for(;;)
		{


			if(HAL_GPIO_ReadPin(KEY_IN_GPIO_Port, KEY_IN_Pin))
			{
				HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, RESET);
				HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_RESET);
				lcd_clear();
				lcd_set_cursor(1,0);
				lcd_display_string("   PRODUCTION   ");
				lcd_set_cursor(2,0);
				lcd_display_string("  KEY NOT PRESS  ");
				osDelay(1000);

			}
			else
			{
				HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, SET);
				HAL_GPIO_WritePin(LED_LORA_uC_GPIO_Port, LED_LORA_uC_Pin, GPIO_PIN_SET);
				lcd_clear();
				lcd_set_cursor(1,0);
				lcd_display_string("   PRODUCTION   ");
				lcd_set_cursor(2,0);
				lcd_display_string("  KEY PRESS ");
				osDelay(1000);

			}
		}
	}
}

/***************************************************************
Function Name : DO 											   *
Inputs:			OnBoard , TurnOn/Off , BitNumber)	   *
OutPuts:		It Alter a Bit on specified DO				   *
****************************************************************/
void DO_On_Off(short State,short BitNumber)
{
	if(State==SET)
	{
		DoTurnOn(BitNumber);
	}
	else if(State==RESET)
	{
		DoTurnOff(BitNumber);
	}
	else
	{
	}
}

/***************************************************************
This Function SET a bit on  DO(8-Bit) for a                *
															   *						   *															   *
Input:	Bit no												   *
****************************************************************/
void DoTurnOn(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, SET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, SET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, SET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, SET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, SET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, RESET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, SET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, RESET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, SET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, RESET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, SET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, RESET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, SET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, RESET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, SET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, RESET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, SET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, RESET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, SET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, RESET);
				break;
		  default:
		            // Invalid mode
		            break;
	}
}

/***************************************************************
This Function Re-SET a bit on EXT DO(8-Bit) for a             *
															   *
Input:	Bit no												   *
****************************************************************/
void DoTurnOff(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, RESET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, RESET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, RESET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, RESET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, RESET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, RESET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, RESET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, RESET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, RESET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, RESET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, RESET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, RESET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, RESET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, RESET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, RESET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, RESET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, RESET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, RESET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, RESET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, RESET);
				break;
		  default:
		            // Invalid mode
		            break;
	}
}

void Do_RESET_SET(unsigned short bit_no)
{
	switch(bit_no)
	{
		case 1:
				HAL_GPIO_WritePin(DO1_GPIO_Port, DO1_Pin, RESET);
				break;
		case 2:
				HAL_GPIO_WritePin(DO2_GPIO_Port, DO2_Pin, RESET);
				break;
		case 3:
				HAL_GPIO_WritePin(DO3_GPIO_Port, DO3_Pin, RESET);
				break;
		case 4:
				HAL_GPIO_WritePin(DO4_GPIO_Port, DO4_Pin, RESET);
				break;
		case 5:
				HAL_GPIO_WritePin(DO5_GPIO_Port, DO5_Pin, RESET);
				break;
		case 6:
				HAL_GPIO_WritePin(DO6_GPIO_Port, DO6_Pin, SET);
				break;
		case 7:
				HAL_GPIO_WritePin(DO7_GPIO_Port, DO7_Pin, RESET);
				break;
		case 8:
				HAL_GPIO_WritePin(DO8_GPIO_Port, DO8_Pin, SET);
				break;
		case 9:
				HAL_GPIO_WritePin(DO9_GPIO_Port, DO9_Pin, RESET);
				break;
		case 10:
				HAL_GPIO_WritePin(DO10_GPIO_Port, DO10_Pin, SET);
				break;
		case 11:
				HAL_GPIO_WritePin(DO11_GPIO_Port, DO11_Pin, RESET);
				break;
		case 12:
				HAL_GPIO_WritePin(DO12_GPIO_Port, DO12_Pin, SET);
				break;
		case 13:
				HAL_GPIO_WritePin(DO13_GPIO_Port, DO13_Pin, RESET);
				break;
		case 14:
				HAL_GPIO_WritePin(DO14_GPIO_Port, DO14_Pin, SET);
				break;
		case 15:
				HAL_GPIO_WritePin(DO15_GPIO_Port, DO15_Pin, RESET);
				break;
		case 16:
				HAL_GPIO_WritePin(DO16_GPIO_Port, DO16_Pin, SET);
				break;
		case 17:
				HAL_GPIO_WritePin(DO17_GPIO_Port, DO17_Pin, RESET);
				break;
		case 18:
				HAL_GPIO_WritePin(DO18_GPIO_Port, DO18_Pin, SET);
				break;
		case 19:
				HAL_GPIO_WritePin(DO19_GPIO_Port, DO19_Pin, RESET);
				break;
		case 20:
				HAL_GPIO_WritePin(DO20_GPIO_Port, DO20_Pin, SET);
				break;
		  default:
		            // Invalid mode
		            break;
	}
}
/***************************************************************
This Function Scans(Read) On Board DI for all DI   *
									   *
															   *
Input:	void											   *
Return	void									   *
****************************************************************/


void ScanDI(void)
{

	if((Pro_Application_flag == 1)&&((pro_DO_DI_TestFinish == 0)))
	{
		if(HAL_GPIO_ReadPin(DI1_GPIO_Port, DI1_Pin))
		{
			test_DI_Final_value[0] = 1;
		}
		else
		{
			test_DI_Final_value[0] = 0;
		}

		if(HAL_GPIO_ReadPin(DI2_GPIO_Port, DI2_Pin))
		{
			test_DI_Final_value[1] = 1;
		}
		else
		{
			test_DI_Final_value[1] = 0;
		}

		if(HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin))
		{
			test_DI_Final_value[2] = 1;
		}
		else
		{
			test_DI_Final_value[2] = 0;
		}

		if(HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin))
		{
			test_DI_Final_value[3] = 1;
		}
		else
		{
			test_DI_Final_value[3] = 0;
		}
	}
	else
	{

		if(HAL_GPIO_ReadPin(DI3_GPIO_Port, DI3_Pin))
		{
			DI_Final_value[0] = 1;
		}
		else
		{
			DI_Final_value[0] = 0;
		}

		if(HAL_GPIO_ReadPin(DI4_GPIO_Port, DI4_Pin))
		{
			DI_Final_value[1] = 1;
		}
		else
		{
			DI_Final_value[1] = 0;
		}

		if(HAL_GPIO_ReadPin(STAT1_PE10_GPIO_Port, STAT1_PE10_Pin))
		{
			DI_Final_value[2] = 1;
		}
		else
		{
			DI_Final_value[2] = 0;
		}

		if(HAL_GPIO_ReadPin(STAT2_PE11_GPIO_Port, STAT2_PE11_Pin))
		{
			DI_Final_value[3] = 1;
		}
		else
		{
			DI_Final_value[3] = 0;
		}

		if(HAL_GPIO_ReadPin(BAT_FAULT_PE12_GPIO_Port, BAT_FAULT_PE12_Pin))
		{
			DI_Final_value[4] = 1;
		}
		else
		{
			DI_Final_value[4] = 0;
		}
	}
}
