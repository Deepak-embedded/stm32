/*
 * ADS1247.c
 *
 *  Created on: Oct 13, 2025
 *      Author: Nelumbo
 */


#ifndef INC_ADS1247_H_
#define INC_ADS1247_H_
#include "stdint.h"
/*register map for ADS1247*/

#define REG_MUX0	0x00
#define REG_VBIAS	0x01
#define	REG_MUX1	0x02
#define	REG_SYS0	0x03
#define	REG_OFC0	0x04
#define	REG_OFC1	0x05
#define	REG_OFC2	0x06
#define REG_FSC0	0x07
#define	REG_FSC1	0x08
#define	REG_FSC2	0x09
#define REG_IDAC0	0x0A
#define	REG_IDAC1	0x0B
#define REG_GPIOCFG	0x0C
#define	REG_GPIODIR	0x0D
#define REG_GPIODAT	0x0E


/* MUX - Multiplexer Control Register 0 (see p31 - bring together with bitwise OR | */
/* BIT7  - BIT6  -  BIT5   -  BIT4   -  BIT3   -  BIT2   -  BIT1   -  BIT0 */
/* PSEL3 - PSEL2 -  PSEL1  -  PSEL0  -  NSEL3  -  NSEL2   - NSEL1   - NSEL0 */
#define MUX_RESET 0x01      // Reset MUX0 Register
/* PSEL3:0 Positive input channel selection bits */
/* Positive input channel selection bits */
#define P_AIN0  0x00  // (default)
#define P_AIN1  0x10
#define P_AIN2  0x20
#define P_AIN3  0x30
#define P_AIN4  0x40
#define P_AIN5  0x50
#define P_AIN6  0x60
#define P_AIN7  0x70
#define P_AINCOM 0x80

/* Negative input channel selection bits (NSEL3:0) */
#define N_AIN0  0x00
#define N_AIN1  0x01  // (default)
#define N_AIN2  0x02
#define N_AIN3  0x03
#define N_AIN4  0x04
#define N_AIN5  0x05
#define N_AIN6  0x06
#define N_AIN7  0x07
#define N_AINCOM 0x08

/* MUX1 - Multiplexer Control Register 1 */
/*  BIT7   -   BIT6   -   BIT5   -   BIT4   -   BIT3   -  BIT2   -  BIT1   -  BIT0 */
/* CLKSTAT - VREFCON1 - VREFCON0 - REFSELT1 - REFSELT0 - MUXCAL2 - MUXCAL1 - MUXCAL0 */
#define			MUX1_RESET		0x00      // Reset MUX1 Register
/* CLKSTAT This bit is read-only and indicates whether the internal or external oscillator is being used
0 = internal, 1 = external */
/* VREFCON1 These bits control the internal voltage reference. These bits allow the reference to be turned on or
off completely, or allow the reference state to follow the state of the device. Note that the internal
reference is required for operation the IDAC functions.*/
/* Voltage Reference Control (VREFCON1[1:0]) */
#define VREFCON1_OFF  0x00  // Internal reference always off (default)
#define VREFCON1_ON   0x20  // Internal reference always on
#define VREFCON1_PS   0x60  // Internal reference on during conversion

/* Reference Input Selection (REFSELT1[1:0]) */
#define REFSELT1_REF0      0x00  // REFP0 and REFN0 reference inputs selected  (default)
#define REFSELT1_REF1      0x08  // REF1 input pair selected applicabe on ads1248
#define REFSELT1_ON        0x10  // Onboard reference selected
#define REFSELT1_ON_REF0   0x18  // Onboard reference connected to REF0

/* Multiplexer Calibration (MUXCAL2[2:0]) */
#define MUXCAL2_NORMAL  0x00  // Normal operation (default)
#define MUXCAL2_OFFSET  0x01  // Offset measurement
#define MUXCAL2_GAIN    0x02  // Gain measurement
#define MUXCAL2_TEMP    0x03  // Temperature diode
#define MUXCAL2_REF1    0x04  // External REF1 measurement
#define MUXCAL2_REF0    0x05  // External REF0 measurement
#define MUXCAL2_AVDD    0x06  // AVDD measurement
#define MUXCAL2_DVDD    0x07  // DVDD measurement



/* SYS0 - System Control Register 0  */
/* BIT7 - BIT6 - BIT5 - BIT4 - BIT3 - BIT2 - BIT1 - BIT0 */
/*  0   - PGA2 - PGA1 - PGA0 - DOR3 - DOR2 - DOR1 - DOR0 */
/* Programmable Gain Amplifier settings (PGA2[2:0]) */
#define PGA2_0   0x00  // Gain = 1 (default)
#define PGA2_2   0x10  // Gain = 2
#define PGA2_4   0x20  // Gain = 4
#define PGA2_8   0x30  // Gain = 8
#define PGA2_16  0x40  // Gain = 16
#define PGA2_32  0x50  // Gain = 32
#define PGA2_64  0x60  // Gain = 64
#define PGA2_128 0x70  // Gain = 128

/* Data Output Rate settings (DOR3[3:0]) */
#define DOR3_5    0x00  // 5 SPS (default)
#define DOR3_10   0x01  // 10 SPS
#define DOR3_20   0x02  // 20 SPS
#define DOR3_40   0x03  // 40 SPS
#define DOR3_80   0x04  // 80 SPS
#define DOR3_160  0x05  // 160 SPS
#define DOR3_320  0x06  // 320 SPS
#define DOR3_640  0x07  // 640 SPS
#define DOR3_1000 0x08  // 1000 SPS
#define DOR3_2000 0x09  // 2000 SPS


/*cmd for ADS1247*/

/*When the START pin is low or the device is in power-down mode, only the RDATA, RDATAC, SDATAC, WAKEUP, and NOP
 commands can be issued.*/
/*@brife1*/
#define CMD_WAKEUP		0x00// Exit power-down mode
#define CMD_SLEEP		0x02// Enter power-down mode
#define CMD_SYNC		0x04//Synchronize ADC conversions
#define CMD_RESET		0x06// Reset to default values
#define CMD_NOP			0xFF//No operation
#define CMD_RDATA		0x12//Read data once
#define CMD_RDATAC		0x14//Read data continuous mode
#define CMD_SDATAC		0x16// Stop read data continuous mode
#define CMD_RREG    	0x20//Read from register CMD_RREG|addr
#define CMD_WREG		0x40//Write to register CMD_RREG|addr
#define CMD_SYSOCAL		0x60// System offset calibration
#define CMD_SYSGCAL		0x61//System gain calibration
#define CMD_SELFOCAL	0x62//Self offset calibration

#define CS_ENABLE  		1
#define CS_DISABLE 		0
#define RESET_ENABLE	1
#define RESET_DISABLE	0
#define START_ENABLE    1
#define START_DISABLE   0


#define CS_LOW 		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, CS_DISABLE)
#define CS_HIGH     HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, CS_ENABLE)
#define RESET_LOW   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, RESET_DISABLE)
#define RESET_HIGH  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, RESET_ENABLE)
#define START_HIGH  HAL_GPIO_WritePin(GPIOB,  GPIO_PIN_12, START_ENABLE)
#define START_LOW 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, START_DISABLE)
#define DRDY_PIN    GPIO_PIN_13
#define DRDY_PORT   GPIOB

uint8_t ADS1247_read_register(uint8_t addr,uint8_t byte);
uint8_t ADS1247_write_register(uint8_t addr,uint8_t byte,uint8_t data);
uint8_t write_cmd(uint8_t cmd);
int32_t ADS1247_ReadData(void);
void ADS1247_begin(void);
float ads1247_raw_to_voltage(int32_t raw, float vref, uint8_t pga);
#endif
