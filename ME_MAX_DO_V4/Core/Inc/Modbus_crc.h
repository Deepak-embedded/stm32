/*
 * Modbus_crc.h
 *
 *  Created on: Dec 2, 2025
 *      Author: Dell
 */

#ifndef INC_MODBUS_CRC_H_
#define INC_MODBUS_CRC_H_


#include "stdint.h"

uint16_t crc16_1(uint8_t *buffer, uint16_t buffer_length);
uint16_t crc16(uint8_t *buffer, uint16_t buffer_length);

#endif /* INC_MODBUS_CRC_H_ */
