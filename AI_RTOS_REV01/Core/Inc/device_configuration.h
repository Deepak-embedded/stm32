/*
 * device_configuration.h
 *
 *  Created on: Feb 2, 2026
 *      Author: 91981
 */

#ifndef INC_DEVICE_CONFIGURATION_H_
#define INC_DEVICE_CONFIGURATION_H_


typedef enum {
	V0_5,
    V0_10,
	MV0_50,
    MV0_100,
	MV0_250,
	MA0_20,
    MA4_20,
	VN5_5,
	VN10_10

} eMode_t;

typedef struct {
    int regValue;
    //const char* config;
    eMode_t config;
} MapEntry;

#define SET 	1
#define RESET	0
#define NLMB_OK 1
#define ENABLE_CHANNEL(channel) do { \
    if ((channel) > 7) { \
        /* Invalid channel - set to safe state or leave as-is */ \
        /* HAL_GPIO_WritePin(MUX_S0_GPIO_Port, MUX_S0_Pin, RESET); */ \
        /* HAL_GPIO_WritePin(MUX_S1_GPIO_Port, MUX_S1_Pin, RESET); */ \
        /* HAL_GPIO_WritePin(MUX_S2_GPIO_Port, MUX_S2_Pin, RESET); */ \
    } else { \
        HAL_GPIO_WritePin(MUX_S0_GPIO_Port, MUX_S0_Pin, ((channel) & 0x01) ? SET : RESET); \
        HAL_GPIO_WritePin(MUX_S1_GPIO_Port, MUX_S1_Pin, ((channel) & 0x02) ? SET : RESET); \
        HAL_GPIO_WritePin(MUX_S2_GPIO_Port, MUX_S2_Pin, ((channel) & 0x04) ? SET : RESET); \
    } \
} while (0)

uint8_t set_adc_conversion_mode(uint16_t conversion_mode,uint8_t channel);
#endif /* INC_DEVICE_CONFIGURATION_H_ */
