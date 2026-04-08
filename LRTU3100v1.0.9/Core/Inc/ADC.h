/*
 * ADC.h
 *
 *  Created on: Sep 28, 2022
 *      Author: maulin
 */
//#include"Configuration.h"
//#include"main.h"
#ifndef INC_ADC_H_
#define INC_ADC_H_

#define MAX_QUERY  			(50)
#define MAX_WRITE_QUERY		(1)
#define MAX_WRITE_QUERY_ACK	(32)  // TODO : Place At proper header file
/**************************************************************************//**
 * Constant
 *****************************************************************************/

#define REF_mV 2048.0//2048.0
#define BIT_Rer_Value 65535.0// 16bit
#define R1 33000
#define R2 5900
#define GAIN 1.30303030
#define MAX_AI_CHANNEL 5

/*Analog input type*/
#define SCALE4TO20			1
#define SCALE0TO20			2
//#define SCALE0TO10			2

/**************************************************************************//**
 * Extern Variable
 *****************************************************************************/

extern osThreadId ADC_TaskHandle;

extern char print[300];
extern uint32_t ADC_VAL_RAW[MAX_AI_CHANNEL];
extern float ADC_VAL_float[MAX_AI_CHANNEL];
extern float AO_VAL_float[MAX_AI_CHANNEL];
extern float AI_mA_VAL_float[MAX_AI_CHANNEL];

/**************************************************************************//**
 * Ennam / uninon / Structure
 *****************************************************************************/
//typedef enum
//{
//	CONF_DISABLE = 0,
//	CONF_ENABLE = 1
//} Comm_En_Dis;
//
//typedef struct
//{
//	Comm_En_Dis Sch_En_Dis;
//	char Sch_Id;
//	char StartHH;
//	char startMin;
//	char StopHH;
//	char stopMin;
//}Schedule_Data;
//
//typedef struct Schedule_Configuration
//{
//	int Total_No_Schedule;
//	Schedule_Data Schedule[85];
//}Schedule_Configuration;
//
//struct Save_Para
//{
//	int scaleType[MAX_AI_CHANNEL];
//	float calZ[MAX_AI_CHANNEL];
//	float calS[MAX_AI_CHANNEL];
//	float scaleLo[MAX_AI_CHANNEL];
//	float scaleHi[MAX_AI_CHANNEL];
//	unsigned short int mAiLowCal[MAX_AI_CHANNEL];  // = 0x0000;
//	unsigned short int mAiLowMidCal[MAX_AI_CHANNEL];   // = 0x0C80;
//	unsigned short int mAiMidCal[MAX_AI_CHANNEL];  // = 0x7FFF;
//	unsigned short int mAiHighCal[MAX_AI_CHANNEL]; // = 0xFFFF;
//
//	/** Slave ID */
//	unsigned char mSlaveId[MAX_QUERY];
//	unsigned short int mRegStartAddr[MAX_QUERY];
//	unsigned char mNoOfRegister[MAX_QUERY];
//	unsigned char mDataType[MAX_QUERY];
//	unsigned char mFunctionCode[MAX_QUERY];
//	unsigned char mPortSelection[MAX_QUERY];
//
//	Schedule_Configuration Schedule;
//};
//
//
//extern struct Save_Para EPROM;
extern float AI_Final_value[];  // TODO : Place at proper header file
extern float ADC_Temp[];
/**************************************************************************//**
 * Function Proto type
 *****************************************************************************/

void MX_ADC1_Init(void);
void ADC_start();
void StartADCTask(void const * argument);
void ADC1_Select_CH3 (void);
void ADC1_Select_CH5 (void);
void ADC1_Select_CH9 (void);
void ADC1_Select_CH15 (void);
void ADC1_Select_CH18 (void);
void ADC1_Select_CH19 (void);
void Get_ADC_Value(int, int);
void ADC1_calibration(void);
void ADC1_calibration_CH(int, int);
void ADC_calculation(float,unsigned int);
uint16_t get_adc_val(int);
void Adc_ConvertBin2Engg(int mChannelno,unsigned short int tAnalogDataBin, float *pAnalogDataEngg);
#endif /* INC_ADC_H_ */
