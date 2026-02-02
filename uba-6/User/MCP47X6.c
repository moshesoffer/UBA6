/*
 * MCP47X6.c
 *
 *  Created on: Aug 26, 2024
 *      Author: ORA
 */

#include "MCP47X6.h"
#include "uart_log.h"

// @formatter:off

#define UBA_COMP "MCP47X6"

#define MCP47X6_VOL_REF_ENABLE (0)


#define MCP47X6_TX_DELAY (50) //50 milisec
#define MCP47X6_RX_DELAY (50) //50 milisec

#define MCP47X6_UINT16_TO_DATA0(data,res) ((data >> (res - 8 )) &0xff) /*get the 8 MSB of the data*/
#define MCP47X6_UINT16_TO_DATA1(data,res) ((data << (16 - res)) &0xff) /*get the 8 LSB of the data*/

#define MCP4706_DATA_SIZE (2)
#define MCP47X6_DATA_SIZE (3)
// @formatter:on

typedef union {
	struct {
		uint16_t CMD :2;
		uint16_t PD :2;
		uint16_t DATA :12;
	} CMD;
	uint16_t value;
} MCP4726_DAC_REG_CMD;

typedef union {
	struct {
		uint8_t CMD :3;
		uint8_t VREF :2;
		uint8_t PD :2;
		uint8_t gain :1;
	} config;
	uint8_t value;
} MCP4726_CMD_config;

typedef union {
	struct {
		uint8_t RDY :1;
		uint8_t POR :1;
		uint8_t MUST_BE_0 :1;
		uint8_t VREF :2;
		uint8_t PD :2;
		uint8_t gain :1;
	} status;
	uint8_t value;
} MCP4726_status;

MCP47X6_ERROR MCP47X6_write_DAC_reg_value(MCP47X6 *mcp, uint16_t reg_value) {
	MCP47X6_ERROR rec_code = MCP47X6_ERROR_NO_ERROR;
	uint8_t data2write[2] = { 0 };
	MCP4726_DAC_REG_CMD t;
	t.CMD.CMD = (MCP47X6_CMD_WR_DAC >> 1) & 0x03;
	t.CMD.PD = mcp->PD;
	data2write[1] = reg_value & 0xff;
	if (mcp->res == 12) {
		t.CMD.DATA = reg_value >> 8;
	} else if (mcp->res == 10) {
		t.CMD.DATA = reg_value >> 6;
		data2write[1] = (reg_value << 2) & 0xff;
	}
	data2write[0] = t.value;
	if (mcp->i2c_handle != NULL) {
		if ((HAL_I2C_Master_Transmit(mcp->i2c_handle, mcp->address << 1, (uint8_t*) data2write, sizeof(data2write),
		MCP47X6_TX_DELAY))) {
			rec_code = MCP47X6_ERROR_I2C;
		}
	} else {
		rec_code = MCP47X6_ERROR_I2C;
	}
	return rec_code;

}
MCP47X6_ERROR MCP47X6_write_mem(MCP47X6 *mcp, uint16_t reg_value) {
	MCP47X6_ERROR rec_code = MCP47X6_ERROR_NO_ERROR;
	HAL_StatusTypeDef res = HAL_OK;
	MCP4726_CMD_config cfg = { 0 };
	uint8_t data2write[3] = { 0 };
	cfg.config.CMD = MCP47X6_CMD_WR_MEM;
	cfg.config.PD = mcp->PD;
	cfg.config.VREF = mcp->V_ref;
	cfg.config.gain = mcp->gain;
	data2write[0] = cfg.value;
	data2write[1] = MCP47X6_UINT16_TO_DATA0(reg_value, mcp->res); //set the 8 MSB to the fist Data Byte
	data2write[2] = MCP47X6_UINT16_TO_DATA1(reg_value, mcp->res); // set the rest of the bit with X as the LSB padiing
	if (mcp->i2c_handle != NULL) {
		if ((res = HAL_I2C_Master_Transmit(mcp->i2c_handle, mcp->address << 1, (uint8_t*) data2write,
				sizeof(data2write),
				MCP47X6_TX_DELAY))) {
			rec_code = MCP47X6_ERROR_I2C;
		}
	} else {
		rec_code = MCP47X6_ERROR_I2C;
	}
	return rec_code;

}
MCP47X6_ERROR MCP47X6_write_all(MCP47X6 *mcp, uint16_t reg_value) {
	MCP47X6_ERROR rec_code = MCP47X6_ERROR_NO_ERROR;
	HAL_StatusTypeDef res = HAL_OK;
	MCP4726_CMD_config cfg = { 0 };
	uint8_t data2write[3] = { 0 };
	cfg.config.CMD = MCP47X6_CMD_WR_ALL;
	cfg.config.VREF = mcp->V_ref;
	cfg.config.PD = mcp->PD;
	cfg.config.gain = mcp->gain;
	data2write[0] = cfg.value;
	data2write[1] = MCP47X6_UINT16_TO_DATA0(reg_value, mcp->res); //set the 8 MSB to the fist Data Byte
	data2write[2] = MCP47X6_UINT16_TO_DATA1(reg_value, mcp->res); // set the rest of the bit with X as the LSB padiing
	if (mcp->i2c_handle != NULL) {
		if ((res = HAL_I2C_Master_Transmit(mcp->i2c_handle, mcp->address << 1, (uint8_t*) data2write,
				sizeof(data2write),
				MCP47X6_TX_DELAY))) {
			rec_code = MCP47X6_ERROR_I2C;
		}
	} else {
		rec_code = MCP47X6_ERROR_I2C;
	}
	return rec_code;
}
MCP47X6_ERROR MCP47X6_write_config(MCP47X6 *mcp) {
	MCP47X6_ERROR rec_code = MCP47X6_ERROR_NO_ERROR;
	MCP4726_CMD_config cfg = { 0 };
	uint8_t data2write[1] = { 0 };
	cfg.config.CMD = MCP47X6_CMD_WR_ALL;
	cfg.config.VREF = mcp->V_ref;
	cfg.config.PD = mcp->PD;
	cfg.config.gain = mcp->gain;
	data2write[0] = cfg.value;
	if (mcp->i2c_handle != NULL) {
		if (( HAL_I2C_Master_Transmit(mcp->i2c_handle, mcp->address << 1, (uint8_t*) data2write, sizeof(data2write),
		MCP47X6_TX_DELAY))) {
			rec_code = MCP47X6_ERROR_I2C;
		}
	} else {
		rec_code = MCP47X6_ERROR_I2C;
	}
	return rec_code;
}

MCP47X6_ERROR MCP47X6_read_status(MCP47X6 *mcp, uint16_t *DAC_value) {
	MCP47X6_ERROR ret_code = MCP47X6_ERROR_NO_ERROR;
	uint8_t data2read[mcp->res == 8 ? MCP4706_DATA_SIZE : MCP47X6_DATA_SIZE];
	if (mcp->i2c_handle != NULL) {
		if ((HAL_I2C_Master_Receive(mcp->i2c_handle, mcp->address << 1, (uint8_t*) data2read, sizeof(data2read),
		MCP47X6_RX_DELAY))) {
			ret_code = MCP47X6_ERROR_I2C;
		}
	} else {
		ret_code = MCP47X6_ERROR_I2C;
	}
	return ret_code;
}
