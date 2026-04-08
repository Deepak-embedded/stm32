/*
 * DIDO.h
 *
 *  Created on: Dec 28, 2022
 *      Author: SanketP
 */

#ifndef INC_DIDO_H_
#define INC_DIDO_H_

#define MAX_DO_CHANNEL 26
#define MAX_DI_CHANNEL 8

#define TURNON 1
#define TURNOFF 0


extern GPIO_TypeDef* DO_GPIO_Port_Array[];
extern uint16_t DO_Pin_Array[];

extern volatile unsigned int DO_pulseIsSet;
extern volatile unsigned int Dual_DO_pulseIsSet;

extern char Dual_DO_Pulse_Stage[];

extern char DO_PulseIsSet_array[];
extern unsigned short int DO_actual_PulseWidth[];
extern unsigned short int DO_actual_PulseWidth_Count[];
extern unsigned short int Dual_DO_actual_PulseWidth[];
extern unsigned short int Dual_DO_actual_PulseWidth_Count[];

extern char DO1_PulseIsSet;
extern unsigned short int DO1_actual_PulseWidth;
extern unsigned short int DO1_actual_PulseWidth_Count;

extern char DO2_PulseIsSet;
extern unsigned short int DO2_actual_PulseWidth;
extern unsigned short int DO2_actual_PulseWidth_Count;

extern char DO3_PulseIsSet;
extern unsigned short int DO3_actual_PulseWidth;
extern unsigned short int DO3_actual_PulseWidth_Count;

extern char DO4_PulseIsSet;
extern unsigned short int DO4_actual_PulseWidth;
extern unsigned short int DO4_actual_PulseWidth_Count;

extern char DO5_PulseIsSet;
extern unsigned short int DO5_actual_PulseWidth;
extern unsigned short int DO5_actual_PulseWidth_Count;

extern char DO6_PulseIsSet;
extern unsigned short int DO6_actual_PulseWidth;
extern unsigned short int DO6_actual_PulseWidth_Count;

extern char DO7_PulseIsSet;
extern unsigned short int DO7_actual_PulseWidth;
extern unsigned short int DO7_actual_PulseWidth_Count;

extern char DO8_PulseIsSet;
extern unsigned short int DO8_actual_PulseWidth;
extern unsigned short int DO8_actual_PulseWidth_Count;

extern char DO9_PulseIsSet;
extern unsigned short int DO9_actual_PulseWidth;
extern unsigned short int DO9_actual_PulseWidth_Count;

extern char DO10_PulseIsSet;
extern unsigned short int DO10_actual_PulseWidth;
extern unsigned short int DO10_actual_PulseWidth_Count;

extern char DO11_PulseIsSet;
extern unsigned short int DO11_actual_PulseWidth;
extern unsigned short int DO11_actual_PulseWidth_Count;

extern char DO12_PulseIsSet;
extern unsigned short int DO12_actual_PulseWidth;
extern unsigned short int DO12_actual_PulseWidth_Count;

extern char DO13_PulseIsSet;
extern unsigned short int DO13_actual_PulseWidth;
extern unsigned short int DO13_actual_PulseWidth_Count;

extern char DO14_PulseIsSet;
extern unsigned short int DO14_actual_PulseWidth;
extern unsigned short int DO14_actual_PulseWidth_Count;

extern char DO15_PulseIsSet;
extern unsigned short int DO15_actual_PulseWidth;
extern unsigned short int DO15_actual_PulseWidth_Count;

extern char DO16_PulseIsSet;
extern unsigned short int DO16_actual_PulseWidth;
extern unsigned short int DO16_actual_PulseWidth_Count;

extern char DO17_PulseIsSet;
extern unsigned short int DO17_actual_PulseWidth;
extern unsigned short int DO17_actual_PulseWidth_Count;

extern char DO18_PulseIsSet;
extern unsigned short int DO18_actual_PulseWidth;
extern unsigned short int DO18_actual_PulseWidth_Count;

extern char DO19_PulseIsSet;
extern unsigned short int DO19_actual_PulseWidth;
extern unsigned short int DO19_actual_PulseWidth_Count;

extern char DO20_PulseIsSet;
extern unsigned short int DO20_actual_PulseWidth;
extern unsigned short int DO20_actual_PulseWidth_Count;

extern char DO21_PulseIsSet;
extern unsigned short int DO21_actual_PulseWidth;
extern unsigned short int DO21_actual_PulseWidth_Count;

extern char DO22_PulseIsSet;
extern unsigned short int DO22_actual_PulseWidth;
extern unsigned short int DO22_actual_PulseWidth_Count;

extern char DO23_PulseIsSet;
extern unsigned short int DO23_actual_PulseWidth;
extern unsigned short int DO23_actual_PulseWidth_Count;

extern char DO24_PulseIsSet;
extern unsigned short int DO24_actual_PulseWidth;
extern unsigned short int DO24_actual_PulseWidth_Count;

extern char DO25_PulseIsSet;
extern unsigned short int DO25_actual_PulseWidth;
extern unsigned short int DO25_actual_PulseWidth_Count;

extern char DO26_PulseIsSet;
extern unsigned short int DO26_actual_PulseWidth;
extern unsigned short int DO26_actual_PulseWidth_Count;
extern unsigned short int DI_Frequency_Counter;
extern osThreadId DIDO_TaskHandle;
extern xTimerHandle PTHandle;

extern float DO_Final_value[MAX_DO_CHANNEL];
extern float DI_Final_value[MAX_DI_CHANNEL];
extern unsigned char DO_KEY_Array[32];
extern volatile unsigned int DI1_Pulse_Count,DI2_Pulse_Count,DI3_Pulse_Count,DI4_Pulse_Count,DI1_Pulse_Count_for_Frequency,DI2_Pulse_Count_for_Frequency;
extern float DI1_Freq,DI2_Freq;
extern float DI1_Freq_fromWaveLength,DI2_Freq_fromWaveLength;
extern unsigned int DI1_waveLength;
extern unsigned int DI2_waveLength;
extern uint64_t actualPulse_DI_frequency_time;
void DIDO_start();
void StartDIDOTask(void const * argument);
void pro_checkDIDOState();
void DO_On_Off(short State,short BitNumber);
void DoTurnOn(unsigned short bit_no);
void DoTurnOff(unsigned short bit_no);
void Do_RESET_SET(unsigned short bit_no);
void ScanDI(void);
void Status_led_BLE_GPS(void);
#endif /* INC_DIDO_H_ */
