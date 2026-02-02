/*
 * TPL0102.c
 *
 *  Created on: Sep 29, 2024
 *      Author: ORA
 */

#include "TPL0102.h"
#include "uart_log.h"
#include "UBA_util.h"

#define UBA_COMP "TPL0102"
#define TPL0102_MAX_HAL_DELAY (50)

#if (UBA_LOG_LEVEL_TPL0102 <= UART_LOG_LEVEL_INFO)
	#define UART_LOG_TPL0102_INFO(...) UART_LOG_INFO(UBA_COMP,##__VA_ARGS__)
#else
	#define UART_LOG_TPL0102_INFO(...)
#endif

#if UBA_LOG_LEVEL_TPL0102 <= UART_LOG_LEVEL_DEBUG
	#define UART_LOG_TPL0102_DEBUG(...)  UART_LOG_DEBUG(UBA_COMP ,##__VA_ARGS__)
#else
	#define UART_LOG_TPL0102_DEBUG(...)
#endif

void tpl010_init(TPL0102 *tpl, I2C_HandleTypeDef *i2c_p) {
	uint8_t data;
	tpl->i2c_handle = i2c_p;
	tpl->address = TPL0102_ADDRESS;
	tpl010_write_reg(tpl, TPL0102_REG_WRA, 0x80);
	tpl010_write_reg(tpl, TPL0102_REG_WRB, 0x80);
	tpl010_write_reg(tpl, TPL0102_REG_ACR, 0x40);
	tpl010_read_reg(tpl, TPL0102_REG_IVRA, &tpl->RA);
	tpl010_read_reg(tpl, TPL0102_REG_IVRB, &tpl->RB);
	tpl010_read_reg(tpl, TPL0102_REG_ACR, &data);
}

void tpl010_read_reg(TPL0102 *tpl, uint8_t reg, uint8_t *reg_value) {
	uint8_t data2read = 0;
	uint8_t data2write = reg;
	if (tpl->i2c_handle != NULL) {
		if (HAL_I2C_Master_Transmit(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2write, sizeof(data2write), TPL0102_MAX_HAL_DELAY)
				== HAL_OK) {
			if (HAL_I2C_Master_Receive(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2read, sizeof(data2read), TPL0102_MAX_HAL_DELAY)
					== HAL_OK) {
				UART_LOG_TPL0102_DEBUG( "read REG:0x%02x -> 0x%02x", reg, data2read);
			} else {
				UART_LOG_CRITICAL(UBA_COMP, "Receive Failed");
			}
		} else {
			UART_LOG_CRITICAL(UBA_COMP, "Transmit Failed");
		}

	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
	}
}

void tpl010_write_reg(TPL0102 *tpl, uint8_t reg, uint8_t reg_value) {
	uint8_t data2write[2] = { reg, reg_value };
	if (tpl->i2c_handle != NULL) {
		if (HAL_I2C_Master_Transmit(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2write[0], sizeof(data2write), TPL0102_MAX_HAL_DELAY)
				== HAL_OK) {
			UART_LOG_TPL0102_DEBUG( "Write REG:0x%02x -> 0x%02x", reg, reg_value);
		} else {
			UART_LOG_CRITICAL(UBA_COMP, "Transmit Failed");
		}

	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
	}
}

void tpl010_set_potentiometer_a(TPL0102 *tpl, uint8_t new_value) {
	tpl010_write_reg(tpl, TPL0102_REG_WRA, new_value);
	tpl->RA = new_value;
}

void tpl010_set_potentiometer_b(TPL0102 *tpl, uint8_t new_value) {
	tpl010_write_reg(tpl, TPL0102_REG_WRB, new_value);
	tpl->RB = new_value;
}
/*return the resistance in OM */
uint32_t tpl010_HA_resistance(TPL0102 *tpl) {
	uint32_t ret = 0;
	if (tpl->RA) {
		ret = 100000 - ((100000 / 0xff) * tpl->RA);
	}
	UART_LOG_TPL0102_DEBUG( "RA:0x%02x res:%u", tpl->RA, ret);
	return ret;
}
uint32_t tpl010_HB_resistance(TPL0102 *tpl) {
	uint32_t ret = 0;
	if (tpl->RB) {
		ret = 100000 - ((100000 / 0xff) * tpl->RB);
	}
	UART_LOG_TPL0102_DEBUG( "RB:0x%02x res:%u", tpl->RB, ret);
	return ret;
}

