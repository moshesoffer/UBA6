/*
 * MCP3221.c
 *
 *  Created on: Sep 30, 2024
 *      Author: ORA
 */

#include "MCP3221.h"
#include "uart_log.h"
#include "UBA_util.h"

#define UBA_COMP "MCP3221"

#define MCP3221_ADDRESS (MCP3221_ADRESS_BASE|MCP3221_ADDRESS_A6)
#define MCP3221_MAX_HAL_DELAY (5000)

#if (UBA_LOG_LEVEL_MCP3221 <= UART_LOG_LEVEL_INFO)
#define UART_LOG_MCP3221_INFO(...) UART_LOG_INFO(UBA_COMP,##__VA_ARGS__)
#else
#define UART_LOG_MCP3221_INFO(...)
#endif

#if (UBA_LOG_LEVEL_MCP3221 <= UART_LOG_LEVEL_DEBUG)
	#define UART_LOG_MCP3221_DEBUG(...) UART_LOG_DEBUG(UBA_COMP,##__VA_ARGS__)
#else
#define UART_LOG_MCP3221_DEBUG(...)
#endif


MCP3221_StatusTypeDef MCP3221_init(MCP3221 *mcp, I2C_HandleTypeDef *i2c_p) {
	uint16_t d;
	uint8_t try = 8;
	mcp->i2c_handle = i2c_p;
	mcp->address = MCP3221_ADDRESS;
	if (MCP3221_read(mcp, &d) != MCP3221_STATUS_OK) {
		mcp->address = MCP3221_ADRESS_BASE;
		while (try && (MCP3221_read(mcp, &d) != MCP3221_STATUS_OK)) {
			mcp->address++;
			try--;
		}
	}
	if(try){
		UART_LOG_MCP3221_DEBUG( "read Value: %02u", d);
		return MCP3221_STATUS_OK;
	}else{
		return MCP3221_STATUS_INTERFACE_ERROR;
	}
}

MCP3221_StatusTypeDef MCP3221_read(MCP3221 *mcp, uint16_t *data_out) {
	uint8_t data2read[2];
	HAL_StatusTypeDef ret = HAL_OK;
	if (mcp->i2c_handle != NULL) {
		ret = HAL_I2C_Master_Receive(mcp->i2c_handle, (uint16_t) (mcp->address << 1), &data2read[0], sizeof(data2read), MCP3221_MAX_HAL_DELAY);
		if (ret == HAL_OK) {
			*data_out = data2read[0] << 8 | data2read[1];
			UART_LOG_MCP3221_DEBUG( "read address:0x%02x value:0x%02x",mcp->address, *data_out);
		} else {
			UART_LOG_CRITICAL(UBA_COMP, "Receive Failed : 0x%x address:0x%02x", ret, mcp->address);
			return MCP3221_STATUS_INTERFACE_ERROR;
		}
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
		return MCP3221_STATUS_NULL_ERROR;
	}
	return MCP3221_STATUS_OK;
}
