/*
 * HC595.h
 *
 *  Created on: Dec 16, 2025
 *      Author: 91981
 */

#ifndef INC_HC595_H_
#define INC_HC595_H_

typedef enum{
	_A_ENABLE=0x01,//enable A pin only and disable all
	_B_ENABLE=0x02,//enable B pin only and disable all
	_C_ENABLE=0x04,//enable C pin only and disable all
	_D_ENABLE=0x08,//enable D pin only and disable all
	_E_ENABLE=0x10,//enable E pin only and disable all
	_F_ENABLE=0x20,//enable F pin only and disable all
	_G_ENABLE=0x40,//enable G pin only and disable all
	_H_ENABLE=0x80,//enable H pin only and disable all
	_AB_ENABLE=_A_ENABLE|_B_ENABLE,
	_ABC_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE,
	_ABCD_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE|_D_ENABLE,
	_ABCDE_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE|_D_ENABLE|_E_ENABLE,
	_ABCDEF_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE|_D_ENABLE|_E_ENABLE|_F_ENABLE,
	_ABCDEFG_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE|_D_ENABLE|_E_ENABLE|_F_ENABLE|_G_ENABLE,
	_ABCDEFGH_ENABLE=_A_ENABLE|_B_ENABLE|_C_ENABLE|_D_ENABLE|_E_ENABLE|_F_ENABLE|_G_ENABLE|_H_ENABLE,
}HC595_OUTPUT_EN;

//typedef    8_BIT_SHIFT_REG uint8_t;
#define DATA_Pin         GPIO_PIN_5
#define DATA_GPIO_Port 	 GPIOB

#define LTCH_Pin         GPIO_PIN_4
#define LTCH_GPIO_Port   GPIOB

#define CLK_Pin			GPIO_PIN_3
#define CLK_GPIO_Port   GPIOB





#endif /* INC_HC595_H_ */
