/*
 * MCP3221.h
 * Low-Power 12-Bit A/D Converter with I2C Interface
 *  Created on: Sep 30, 2024
 *      Author: ORA
 */

#ifndef MCP3221_H_
#define MCP3221_H_

#include "i2c.h"
#define MCP3221_ADRESS_BASE 	(0x48)
#define MCP3221_ADDRESS_A0 		(0x00)
#define MCP3221_ADDRESS_A1 		(0x01)
#define MCP3221_ADDRESS_A2 		(0x02)
#define MCP3221_ADDRESS_A3 		(0x03)
#define MCP3221_ADDRESS_A4 		(0x04)
#define MCP3221_ADDRESS_A5 		(0x05)
#define MCP3221_ADDRESS_A6 		(0x06)
#define MCP3221_ADDRESS_A7 		(0x07)


typedef enum MCP3221_StatusTypeDef {
	MCP3221_STATUS_OK = 0,
	MCP3221_STATUS_INTERFACE_ERROR = 1,
	MCP3221_STATUS_VALUE_ERROR = 2,
	MCP3221_STATUS_NULL_ERROR = 3,
	MCP3221_STATUS_BUSY = 4,
} MCP3221_StatusTypeDef;

typedef struct MCP3221 {
	I2C_HandleTypeDef* i2c_handle; // pointer to the i2c handle
	uint8_t address;
} MCP3221;

#define MCP3221_DEFUALT 					\
{											\
	NULL,									\
	(MCP3221_ADRESS_BASE|MCP3221_ADDRESS_A0) \
}


MCP3221_StatusTypeDef MCP3221_read(MCP3221 * mcp,uint16_t* data_out);
MCP3221_StatusTypeDef MCP3221_init(MCP3221 *mcp,I2C_HandleTypeDef *i2c_p);


#endif /* MCP3221_H_ */
