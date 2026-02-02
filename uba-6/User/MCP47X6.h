/*
 * MCP47X6.h
 *
 *  Created on: Aug 26, 2024
 *      Author: ORA
 *      Driver for MCP47X6
 */

#ifndef MCP47X6_H_
#define MCP47X6_H_
// @formatter:off

#include "i2c.h"

#define MCP47X6_ADDRESS_BASE 	((0x0c)<<3)
#define MCP47X6_ADDRESS_A0 		(0x00)
#define MCP47X6_ADDRESS_A1 		(0x01)
#define MCP47X6_ADDRESS_A2 		(0x02)
#define MCP47X6_ADDRESS_A3 		(0x03)
#define MCP47X6_ADDRESS_A4 		(0x04)
#define MCP47X6_ADDRESS_A5 		(0x05)
#define MCP47X6_ADDRESS_A6 		(0x06)
#define MCP47X6_ADDRESS_A7 		(0x07)

#define MCP47X6_CMD_WR_DAC		(0x00)
#define MCP47X6_CMD_WR_MEM		(0x02)
#define MCP47X6_CMD_WR_ALL		(0x03)
#define MCP47X6_CMD_WR_CONF		(0x04)

#define MCP47X6_GAIN			(0x01)  /*VREF pin is NOT used as voltage source*/
#define MCP47X6_MAX_BIT_RES		(0x0c) /*the max bit res is 12bit*/

// @formatter:on

typedef enum MCP47X6_ERROR {
	MCP47X6_ERROR_NO_ERROR = 0x00,
	MCP47X6_ERROR_I2C = 0x01,

} MCP47X6_ERROR;

typedef enum MCP47X6_POWER_DOWN_OP {
	MCP47X6_POWER_DOWN_OP_NORMAL = 0x00,
	MCP47X6_POWER_DOWN_1K = 0x01,
	MCP47X6_POWER_DOWN_125K = 0x02,
	MCP47X6_POWER_DOWN_640K = 0x03,
} MCP47X6_POWER_DOWN_OP;

typedef enum MCP47X6_RESOLUTION {
	MCP47X6_RESOLUTION_INVALID = 0x00,
	MCP47X6_RESOLUTION_8BIT = 0x08,
	MCP47X6_RESOLUTION_10BIT = 0x0A,
	MCP47X6_RESOLUTION_12BIT = 0x0C,
} MCP47X6_RESOLUTION;


typedef struct MCP47X6 {
	I2C_HandleTypeDef* i2c_handle; // pointer to the i2c handle
	uint8_t address;
	MCP47X6_POWER_DOWN_OP PD;
	uint8_t gain;
	uint8_t V_ref;
	uint8_t res ;
} MCP47X6;

#define MCP47X6_DEFUALT 			\
{									\
	NULL,							\
	MCP47X6_ADDRESS_BASE,			\
	MCP47X6_POWER_DOWN_OP_NORMAL,	\
	0,								\
	0,								\
	0,								\
}


MCP47X6_ERROR MCP47X6_write_DAC_reg_value(MCP47X6 *mcp, uint16_t reg_value);
MCP47X6_ERROR MCP47X6_write_mem(MCP47X6 *mcp, uint16_t reg_value);
MCP47X6_ERROR MCP47X6_write_all(MCP47X6 *mcp, uint16_t reg_value);
MCP47X6_ERROR MCP47X6_write_config(MCP47X6 *mcp);

#endif /* MCP47X6_H_ */

