/*
 * dipswitch.h
 *
 *  Created on: Dec 4, 2025
 *      Author: 91981
 */

#ifndef INC_DIPSWITCH_H_
#define INC_DIPSWITCH_H_

//#include "stm32f0xx_hal.h"

void RST_HIGH_B(void);
void RST_HIGH_B(void);
void SPI_Write_B (uint8_t *data, uint8_t len);
void SPI_Read_B (uint8_t *data, uint8_t len);
void enable_MCPA_B (void);
void enable_MCPB_B (void);
void port_configB_B (void);
void port_configA_B (void);
void polarity_configB_B (void);
void latch_configA_B (void) ;
void latch_configB_B (void);
uint8_t GPIO_readA_B ();
void csHIGH_B(void);
void csLOW_B(void);

typedef enum{
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_115200
}baudrate_t;

#define MCP23_RST1_PORT		MCP_RST1_GPIO_Port
#define MCP23_RST1_PIN 		MCP_RST1_Pin

#define MCP23_CS_PIN		MCP_CS2_Pin
#define MCP23_CS_PORT 		MCP_CS2_GPIO_Port




#endif /* INC_DIPSWITCH_H_ */
