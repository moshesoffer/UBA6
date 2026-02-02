/*
 * MCP3221.c
 *
 *  Created on: Sep 30, 2024
 *      Author: ORA
 */

#include "MCP3221.h"
#include "uart_log.h"

#define UBA_COMP "MCP3221"

#define MCP3221_ADDRESS (MCP3221_ADRESS_BASE|MCP3221_ADDRESS_A6)
#define MCP3221_MAX_HAL_DELAY (50)

void MCP3221_init(MCP3221 *mcp,I2C_HandleTypeDef *i2c_p){
	uint16_t d;
	mcp->i2c_handle = i2c_p;
	mcp->address = MCP3221_ADDRESS;
	MCP3221_read(mcp,&d);
	UART_LOG_DEBUG(UBA_COMP, "read Value: %02u", d);
}


void MCP3221_read(MCP3221 *mcp, uint16_t *data_out) {
	uint8_t data2read[2];
	if (mcp->i2c_handle != NULL) {
			if (HAL_I2C_Master_Receive(mcp->i2c_handle, (uint16_t) (mcp->address << 1), &data2read[0], sizeof(data2read), MCP3221_MAX_HAL_DELAY)
					== HAL_OK) {
				*data_out = data2read[0]<<8 |data2read[1];
				UART_LOG_DEBUG(UBA_COMP, "read 0x%02x", *data_out);
			} else {
				UART_LOG_CRITICAL(UBA_COMP, "Receive Failed");
			}

	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
	}

}
