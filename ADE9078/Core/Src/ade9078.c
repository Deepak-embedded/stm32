/*
 * ade9078.c
 *
 *  Created on: Nov 19, 2025
 *      Author: 91981
 */


/*16 bit command [indicates whether to perform a read or wright and which register to access followed by 16 or 32 bit data to write/read]
 * in case of read operation CRC of register data follows[ if addr not support burst reading]
 * in case of read operation data from next register follow[if addr support burst reading]
 *
 * supported mode [ mode 0  CPOL=0 and CPHA=0
 * 				    mode 3  CPOL=1 and CPHA=1]
 * supported frequency 10MHz
 *
 * CMD_HDR[12 MSBs of command header]=
 *         (ADDR<<4)|0x08 for read [retrieve Data |CRC| burst not enable ,else retrieve Data of register |Data of next register]
 *         (ADDR<<4)|0x00 for write[CMD_HDR | Data
 *
 * Data clocked Out After Addressed Data in SPI Read Operation[after 16 or 32 byte
 *         Address  | Burst_en=0  | Burst_en =1
 *
 *    0x000 to 0x4FF | CRC         				| same data is resent
 *    0x500 to 0x6FF | CRC         				| Next addresss
 *    0x800 to 0xFFF |
 *    if BURST_CHAN=1111(CRC);  |if BURST_CHAN=111,the same data is resent
 *    (waveformBuffer|otherwise,next address    | otherwise,next address
 *
 */
#include"ade9078.h"
#include<stdint.h>
#include"spi.h"
#include "ADE9078Calibrations.h"
#include<stdio.h>
#include <stdbool.h>
#include<math.h>

 static float V_FS = 0.707;//full scale voltage in RMS
// static float k=252.7;//ratio calculated as per mesurement
 static float k=300;//ratio calculated as per mesurement
 Current_X cx;
 Freq_X Lfx;
 Active_pow watts;
 Reactive_pow var;
 Apparent_pow va;
 Angle_xx_xx Anglexx;
 Power_Factorx Pfx;

 uint32_t value=0;

void ADE9078_read_Regs_Data(uint16_t addr,uint32_t *outbuffer ,Byte_t Byte){
	addr=((addr<<4)&0XFFF0)| R_BIT;
	uint8_t CMD[5]={CMD_NOP,CMD_NOP,CMD_NOP,CMD_NOP};
	uint8_t temp[4];
	uint8_t CMD_HDR1=(uint8_t)((addr>>8)&0xff);
	uint8_t CMD_HDR2=(uint8_t)(addr&0x00ff);
	CS_LOW;
	HAL_Delay(10);
//	HAL_SPI_Transmit(&hspi2, (uint8_t*)&addr, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR1, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR2, 1, HAL_MAX_DELAY);
	if(Byte==BYTE_16){
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[0], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[1], 1, HAL_MAX_DELAY);
		*outbuffer=temp[0]<<8|temp[1];
	}
	else if(Byte==BYTE_32){
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[0], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[1], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[2], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[3], 1, HAL_MAX_DELAY);
		*outbuffer=temp[0]<<24|temp[1]<<16|temp[2]<<8|temp[3];
	}
	CS_HIGH;
	HAL_Delay(10);

}

void ADE9078_wright_Regs_Data(uint16_t addr,uint32_t data ,Byte_t Byte){

	addr=addr<<4| W_BIT;
	uint8_t temp[4];
	uint8_t CMD_HDR1=(uint8_t)((addr>>8)&0xff);
	uint8_t CMD_HDR2=(uint8_t)(addr&0x00ff);
	CS_LOW;
	HAL_Delay(10);
//	HAL_SPI_Transmit(&hspi2, (uint8_t*)&addr, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR1, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR2, 1, HAL_MAX_DELAY);
	if(Byte==BYTE_16){
		temp[0]=(uint8_t)((data>>8)&0xff);
		temp[1]=(uint8_t)data&0x00ff;
		HAL_SPI_Transmit(&hspi2,&temp[0] , 1, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi2,&temp[1] , 1, HAL_MAX_DELAY);

	}
	else if(Byte==BYTE_32){
		temp[0]=(uint8_t)((data>>24)&0xff);
		temp[1]=(uint8_t)((data>>16)&0xff);
		temp[2]=(uint8_t)((data>>8)&0xff);
		temp[3]=(uint8_t)(data&0xff);
		HAL_SPI_Transmit(&hspi2,&temp[0] , 1, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi2,&temp[1] , 1, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi2,&temp[2] , 1, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi2,&temp[3] , 1, HAL_MAX_DELAY);
	}
	CS_HIGH;

}

uint16_t crc16(char* pData, int length) //https://gist.github.com/tijnkooijmans/10981093, CCITT:  CCITT-FALSE (poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1)
{
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

void ADE9078_read_Regs_CRC(uint16_t addr,uint32_t CRC_byte,Byte_t Byte){

	addr=((addr<<4)&0XFFF0)| R_BIT;
	uint8_t CMD[5]={CMD_NOP,CMD_NOP,CMD_NOP,CMD_NOP};
	uint8_t temp[4];
	uint8_t temp_CRC[2];
	uint8_t CMD_HDR1=(uint8_t)((addr>>8)&0xff);
	uint8_t CMD_HDR2=(uint8_t)(addr&0x00ff);
	CS_LOW;
	HAL_Delay(10);
//	HAL_SPI_Transmit(&hspi2, (uint8_t*)&addr, 2, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR1, 1, HAL_MAX_DELAY);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)&CMD_HDR2, 1, HAL_MAX_DELAY);
	if(Byte==BYTE_16){
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[0], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[1], 1, HAL_MAX_DELAY);
		//*outbuffer=temp[0]<<8|temp[1];
	}
	else if(Byte==BYTE_32){
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[0], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[1], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[2], 1, HAL_MAX_DELAY);
		HAL_SPI_TransmitReceive(&hspi2, CMD, &temp[3], 1, HAL_MAX_DELAY);
		//*outbuffer=temp[0]<<24|temp[1]<<16|temp[2]<<8|temp[3];
	}
	HAL_SPI_TransmitReceive(&hspi2, CMD, &temp_CRC[0], 1, HAL_MAX_DELAY);
	HAL_SPI_TransmitReceive(&hspi2, CMD, &temp_CRC[1], 1, HAL_MAX_DELAY);
	CRC_byte=temp_CRC[0]<<8|temp_CRC[1];

	CS_HIGH;



}

CRC_Status IS_CRC(uint16_t addr,Byte_t Byte){

	CRC_Status status;
	uint32_t outbuffer;
	uint16_t CRC_byte=0x0000;

	 ADE9078_read_Regs_Data(addr,&outbuffer ,Byte);
	 ADE9078_read_Regs_CRC(addr,CRC_byte,Byte);

	 if(Byte==BYTE_16)
		 (crc16((char*)&outbuffer, 2)==CRC_byte)?(status=CRC_VALID):(status=CRC_INVALID);
	 else if(Byte==BYTE_16)
		 (crc16((char*)&outbuffer, 4)==CRC_byte)?(status=CRC_VALID):(status=CRC_INVALID);

	 return status;

}

double getAVrms(){

	ADE9078_read_Regs_Data(AVRMS_32,&value ,BYTE_32);
	double ARMS=(double)value;
	double V_ARMS = (ARMS / 52866837) * V_FS;//voltage at adc without divider multiplication
	double V_ARMS_line=V_ARMS*k;//actual voltage
//	double V_ARMS_line=ARMS/75000.0;
	return V_ARMS_line;
}

double getBVrms(){
	uint32_t value=0;
	ADE9078_read_Regs_Data(BVRMS_32,&value ,BYTE_32);
	double BRMS=(double)value;
	double V_BRMS = (BRMS / 52866837) * V_FS;//voltage at adc without divider multiplication
	double V_BRMS_line=V_BRMS*k;//actual voltage
	return V_BRMS_line;
}

double getCVrms(){
	uint32_t value=0;
	ADE9078_read_Regs_Data(CVRMS_32,&value ,BYTE_32);
	double CRMS=(double)value;
	double V_CRMS = (CRMS / 52866837) * V_FS;//voltage at adc without divider multiplication
	double V_CRMS_line=V_CRMS*k;//actual voltage
	return V_CRMS_line;
}

uint32_t readVoltage()
{
	volt_t instVoltage;

     ADE9078_read_Regs_Data(AV_PCF_32,&instVoltage.a ,BYTE_32);
     ADE9078_read_Regs_Data(AV_PCF_32,&instVoltage.b ,BYTE_32);
     ADE9078_read_Regs_Data(AV_PCF_32,&instVoltage.c ,BYTE_32);

     return instVoltage.a;
}

void ReadCurrentRMSRegs(CurrentRMSRegs *Data)
{
    ADE9078_read_Regs_Data(AIRMS_32,(uint32_t*)&Data->CurrentRMSReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BIRMS_32,(uint32_t*)&Data->CurrentRMSReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CIRMS_32,(uint32_t*)&Data->CurrentRMSReg_C ,BYTE_32);
    ADE9078_read_Regs_Data(NIRMS_32,(uint32_t*)&Data->CurrentRMSReg_N ,BYTE_32);

}

void ReadPowerRMSRegs(CurrentRMSRegs *Data)
{
    ADE9078_read_Regs_Data(AIRMS_32,(uint32_t*)&Data->CurrentRMSReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BIRMS_32,(uint32_t*)&Data->CurrentRMSReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CIRMS_32,(uint32_t*)&Data->CurrentRMSReg_C ,BYTE_32);
    ADE9078_read_Regs_Data(NIRMS_32,(uint32_t*)&Data->CurrentRMSReg_N ,BYTE_32);

}
/*@brif call this function convert Current RMS Register value to current
 * xRMSRegs :-pointer to structure generated by ReadCurrentRMSRegs
 *
*/
Current_X ReadCT(CurrentRMSRegs *xRMSRegs){

  double Vadc =0.0f;
  double Rb=100.0f;
  double CTratio=550;//1000.0f;
  double Isec=0.0f;

  Vadc=(((double)(xRMSRegs->CurrentRMSReg_A)/52866837)*0.707);
  Isec=Vadc/(Rb);
  cx.Current_A=Isec*CTratio;

  Vadc=(((double)(xRMSRegs->CurrentRMSReg_B)/52866837)*0.707);
  Isec=Vadc/(Rb);
  cx.Current_B=Isec*CTratio;

  Vadc=(((double)(xRMSRegs->CurrentRMSReg_C)/52866837)*0.707);
  Isec=Vadc/(Rb);
  cx.Current_C=Isec*CTratio;
 // cx.Current_C=Isec*CTratio;


  return cx;
}


void ReadAngleRegs(AngleRegs_xx_xx *Data){


	ADE9078_read_Regs_Data(ANGL_VA_VB_16,&Data->AngleReg_VA_VB ,BYTE_16);
	HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_VB_VC_16,&Data->AngleReg_VB_VC ,BYTE_16);
    HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_VA_VC_16,&Data->AngleReg_VA_VC ,BYTE_16);
    HAL_Delay(10);

    ADE9078_read_Regs_Data(ANGL_VA_IA_16,&Data->AnlgeReg_VA_IA ,BYTE_16);
    HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_VB_IB_16,&Data->AngleReg_VB_IB ,BYTE_16);
    HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_VC_IC_16,&Data->AngleReg_VC_IC ,BYTE_16);
    HAL_Delay(10);

    ADE9078_read_Regs_Data(ANGL_IA_IB_16,&Data->AngleReg_IA_IB ,BYTE_16);
    HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_IB_IC_16,&Data->AngleReg_IB_IC ,BYTE_16);
    HAL_Delay(10);
    ADE9078_read_Regs_Data(ANGL_IA_IC_16,&Data->AngleReg_IA_IC ,BYTE_16);
    HAL_Delay(10);

}

Angle_xx_xx ReadAngle(AngleRegs_xx_xx *Data){

	Anglexx.Angle_VA_VB=Data->AngleReg_VA_VB*ANGL_RESOLUTION;
	Anglexx.Angle_VB_VC=Data->AngleReg_VB_VC*ANGL_RESOLUTION;
	Anglexx.Angle_VA_VC=Data->AngleReg_VA_VC*ANGL_RESOLUTION;

	Anglexx.Angle_VA_IA=Data->AnlgeReg_VA_IA*ANGL_RESOLUTION;
	Anglexx.Angle_VB_IB=Data->AngleReg_VB_IB*ANGL_RESOLUTION;
	Anglexx.Angle_VC_IC=Data->AngleReg_VC_IC*ANGL_RESOLUTION;

	Anglexx.Angle_IA_IB=Data->AngleReg_IA_IB*ANGL_RESOLUTION;
	Anglexx.Angle_IB_IC=Data->AngleReg_IB_IC*ANGL_RESOLUTION;
	Anglexx.Angle_IA_IC=Data->AngleReg_IA_IC*ANGL_RESOLUTION;

	return Anglexx;
}
Freq_X ReadFrequency(Line_period_Regs *period_x){
	Lfx.Freq_A=(double)(262144000/(period_x->Line_PeriodReg_A+1));
	Lfx.Freq_B=(double)(262144000/(period_x->Line_PeriodReg_B+1));
	Lfx.Freq_C=(double)(262144000/(period_x->Line_PeriodReg_C+1));

	return Lfx;
}

void ReadFrequencyRegs(Line_period_Regs *Data)
{
    ADE9078_read_Regs_Data(APERIOD_32,(uint32_t*)&Data->Line_PeriodReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BPERIOD_32,(uint32_t*)&Data->Line_PeriodReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CPERIOD_32,(uint32_t*)&Data->Line_PeriodReg_C ,BYTE_32);
}

void ReadActivePowerReg(Active_pow_Regs *Data){

    ADE9078_read_Regs_Data(AWATT_32,(uint32_t*)&Data->Active_PowerReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BWATT_32,(uint32_t*)&Data->Active_PowerReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CWATT_32,(uint32_t*)&Data->Active_PowerReg_C ,BYTE_32);

}

Active_pow ReadActivePower(Active_pow_Regs *Data){

	watts.Active_Power_A = (double)(Data->Active_PowerReg_A/416667.0);//1000000
	watts.Active_Power_B = (double)(Data->Active_PowerReg_B/416667.0);
	watts.Active_Power_C = (double)(Data->Active_PowerReg_C/41667.0);



	return watts;
}

void ReadReactivePowerReg(Reactive_pow_Regs *Data){

    ADE9078_read_Regs_Data(AVAR_32,(uint32_t*)&Data->Reactive_PowerReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BVAR_32,(uint32_t*)&Data->Reactive_PowerReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CVAR_32,(uint32_t*)&Data->Reactive_PowerReg_C ,BYTE_32);

}

Reactive_pow ReadReactivePower(Reactive_pow_Regs *Data){

	var.Reactive_Power_A = (double)(Data->Reactive_PowerReg_A/416667);//1000000
	var.Reactive_Power_B = (double)(Data->Reactive_PowerReg_B/416667);
	var.Reactive_Power_C = (double)(Data->Reactive_PowerReg_C/41667);

	return var;
}

void ReadApparentPowerReg(Apparent_pow_Reg *Data){

    ADE9078_read_Regs_Data(AVA_32,(uint32_t*)&Data->Apparent_PowerReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BVA_32,(uint32_t*)&Data->Apparent_PowerReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CVA_32,(uint32_t*)&Data->Apparent_PowerReg_C ,BYTE_32);
}

Apparent_pow ReadApparentPower(Apparent_pow_Reg *Data){
	va.Apparent_Power_A = (double)(Data->Apparent_PowerReg_A/416667);//1000000
	va.Apparent_Power_B = (double)(Data->Apparent_PowerReg_B/416667);
	va.Apparent_Power_C = (double)(Data->Apparent_PowerReg_C/41667);

	return va;

}

void ReadPowerFactorReg(Power_Factor_Regx *Data){

    ADE9078_read_Regs_Data(APF_32,(uint32_t*)&Data->Power_FactorReg_A ,BYTE_32);
    ADE9078_read_Regs_Data(BPF_32,(uint32_t*)&Data->Power_FactorReg_B ,BYTE_32);
    ADE9078_read_Regs_Data(CPF_32,(uint32_t*)&Data->Power_FactorReg_C ,BYTE_32);

}

Power_Factorx ReadPowerFactor(Power_Factor_Regx *Data){

	Pfx.Power_Factor_A= Data->Power_FactorReg_A*pow(2,-27);
	Pfx.Power_Factor_B= Data->Power_FactorReg_B*pow(2,-27);
	Pfx.Power_Factor_C= Data->Power_FactorReg_C*pow(2,-27);

	return Pfx;
}



uint16_t get_version(){

	uint32_t version;
	ADE9078_read_Regs_Data(VERSION_16, &version,BYTE_16);;
	return version;
}
