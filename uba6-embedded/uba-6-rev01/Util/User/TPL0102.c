/*
 * TPL0102.c
 *
 *  Created on: Sep 29, 2024
 *      Author: ORA
 */

#include "TPL0102.h"
#include "uart_log.h"
#include "UBA_util.h"
#include "stdbool.h"

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

/**
 *
 * @param tpl - a TPL0102 strict to init
 * @param i2c_p - a I2C pointer , the interface to init the tplo with
 * @return TPL0102_STATUS_OK if the init was successful
 */
TPL0102_StatusTypeDef tpl010_init(TPL0102 *tpl, I2C_HandleTypeDef *i2c_p) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	if ((i2c_p == NULL) || (tpl == NULL)) {
		ret = TPL0102_STATUS_NULL_ERROR;
	} else {
		tpl->i2c_handle = i2c_p;
	}
	tpl->address = TPL0102_ADDRESS;
	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);

	if (ret == TPL0102_STATUS_OK) {
		if (tpl->ACR.flags.VOL == TPL0102_REG_ACR_VOL_WR_ACCESS) {
			tpl->ACR.flags.VOL = TPL0102_REG_ACR_VOL_IVAR_ACCESS;
			tpl->ACR.flags.SHDN = TPL0102_REG_ACR_SHDN_DISABLE;
			tpl->ACR.flags.WIP = TPL0102_REG_ACR_WIP_AVAILABLE;
			ret = tpl010_write_reg(tpl, TPL0102_REG_ACR, tpl->ACR.value);
		}
	}
	if (ret == TPL0102_STATUS_OK) {
		if (tpl->ACR.flags.VOL == TPL0102_REG_ACR_VOL_IVAR_ACCESS) {
			ret |= tpl010_read_reg(tpl, TPL0102_REG_IVRA, &tpl->IVRA);
			ret |= tpl010_read_reg(tpl, TPL0102_REG_IVRB, &tpl->IVRB);
			tpl->ACR.flags.VOL = TPL0102_REG_ACR_VOL_WR_ACCESS;
			ret |= tpl010_write_reg(tpl, TPL0102_REG_ACR, tpl->ACR.value);
			ret |= tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
			ret |= tpl010_read_reg(tpl, TPL0102_REG_WRA, &tpl->WRA);
			ret |= tpl010_read_reg(tpl, TPL0102_REG_WRB, &tpl->WRA);
		} else {
			UART_LOG_ERROR(UBA_COMP, "Already In IVAR mode");
		}
	}

	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_set_potentiometer_a_inital_value(tpl, 0xff);
	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_set_potentiometer_b_inital_value(tpl, 0xff);
	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_set_potentiometer_a(tpl, 0xfe);
	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_set_potentiometer_b(tpl, 0xfe);
	if (ret == TPL0102_STATUS_OK)
		ret = tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
	return ret;
}

TPL0102_StatusTypeDef tpl010_read_reg(TPL0102 *tpl, uint8_t reg, uint8_t *reg_value) {
	uint8_t data2read = 0;
	uint8_t data2write = reg;
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	if (tpl->i2c_handle != NULL) {
		if (HAL_I2C_Master_Transmit(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2write, sizeof(data2write), TPL0102_MAX_HAL_DELAY)
				== HAL_OK) {
			if (HAL_I2C_Master_Receive(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2read, sizeof(data2read), TPL0102_MAX_HAL_DELAY)
					== HAL_OK) {
				*reg_value = data2read;
				UART_LOG_TPL0102_DEBUG("read REG:0x%02x -> 0x%02x", reg, *reg_value);
				ret = TPL0102_STATUS_OK;
			} else {
				UART_LOG_CRITICAL(UBA_COMP, "Receive Failed");
				ret = TPL0102_STATUS_INTERFACE_ERROR;
			}
		} else {
			UART_LOG_CRITICAL(UBA_COMP, "Transmit Failed");
			ret = TPL0102_STATUS_INTERFACE_ERROR;
		}
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
		ret = TPL0102_STATUS_NULL_ERROR;
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_write_reg(TPL0102 *tpl, uint8_t reg, uint8_t reg_value) {
	uint8_t data2write[2] = { reg, reg_value };
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	if (tpl->i2c_handle != NULL) {
		if (HAL_I2C_Master_Transmit(tpl->i2c_handle, (uint16_t) (tpl->address << 1), &data2write[0], sizeof(data2write), TPL0102_MAX_HAL_DELAY)
				== HAL_OK) {
			UART_LOG_TPL0102_DEBUG("Write REG:0x%02x -> 0x%02x", reg, reg_value);
			ret = TPL0102_STATUS_OK;
		} else {
			UART_LOG_CRITICAL(UBA_COMP, "Transmit Failed");
			ret = TPL0102_STATUS_INTERFACE_ERROR;
		}
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "i2c handle is null");
		ret = TPL0102_STATUS_NULL_ERROR;
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_set_potentiometer(TPL0102 *tpl, bool isA, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	uint8_t reg = TPL0102_REG_WRA;
	if (isA) {
		reg = TPL0102_REG_WRA;
	} else {
		reg = TPL0102_REG_WRB;
	}
	ret = tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
	if (ret == TPL0102_STATUS_OK) {
		if (tpl->ACR.flags.VOL != TPL0102_REG_ACR_VOL_WR_ACCESS) {
			UART_LOG_WARNNING(UBA_COMP, "Wiper is not Available");
			tpl->ACR.flags.VOL = TPL0102_REG_ACR_VOL_WR_ACCESS;
			tpl010_write_reg(tpl, TPL0102_REG_ACR, tpl->ACR.value);
			tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
		}
		if (tpl->ACR.flags.VOL != TPL0102_REG_ACR_VOL_WR_ACCESS) {
			UART_LOG_CRITICAL(UBA_COMP, "Wiper is not Available");
			return TPL0102_STATUS_BUSY;
		}
		ret = tpl010_write_reg(tpl, reg, new_value);
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_set_potentiometer_a(TPL0102 *tpl, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	ret = tpl010_set_potentiometer(tpl, true, new_value);
	if (ret == TPL0102_STATUS_OK) {
		tpl->WRA = new_value;
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_set_potentiometer_b(TPL0102 *tpl, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	ret = tpl010_set_potentiometer(tpl, false, new_value);
	if (ret == TPL0102_STATUS_OK) {
		tpl->WRB = new_value;
	}
	return ret;
}
TPL0102_StatusTypeDef tpl010_set_potentiometer_inital_value(TPL0102 *tpl, bool isA, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	uint8_t reg = TPL0102_REG_IVRA;
	uint8_t old_value = 0;
	if (isA) {
		reg = TPL0102_REG_IVRA;
	} else {
		reg = TPL0102_REG_IVRB;
	}
	ret = tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
	if (ret == TPL0102_STATUS_OK) {
		if (tpl->ACR.flags.VOL != TPL0102_REG_ACR_VOL_IVAR_ACCESS) {
			UART_LOG_WARNNING(UBA_COMP, "IVAR is not Available");
			tpl->ACR.flags.VOL = TPL0102_REG_ACR_VOL_IVAR_ACCESS;
			ret = tpl010_write_reg(tpl, TPL0102_REG_ACR, tpl->ACR.value);
			ret = tpl010_read_reg(tpl, TPL0102_REG_ACR, &tpl->ACR.value);
		}
		if (tpl->ACR.flags.VOL != TPL0102_REG_ACR_VOL_IVAR_ACCESS) {
			return TPL0102_STATUS_BUSY;
		}
	}
	ret = tpl010_read_reg(tpl, reg, &old_value);
	if (old_value != new_value) { // write the EEPROM only if need EEPROM only good for 100K writes
		ret = tpl010_write_reg(tpl, reg, new_value);
		HAL_Delay(TPL0102_T_wc);
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_set_potentiometer_a_inital_value(TPL0102 *tpl, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	ret = tpl010_set_potentiometer_inital_value(tpl, true, new_value);
	if (ret == TPL0102_STATUS_OK) {
		tpl->IVRA = new_value;
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_set_potentiometer_b_inital_value(TPL0102 *tpl, uint8_t new_value) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	ret = tpl010_set_potentiometer_inital_value(tpl, false, new_value);
	if (ret == TPL0102_STATUS_OK) {
		tpl->IVRB = new_value;
	}
	return ret;
}

TPL0102_StatusTypeDef tpl010_shutdown(TPL0102 *tpl) {
	TPL0102_StatusTypeDef ret = TPL0102_STATUS_OK;
	tpl->ACR.flags.SHDN = TPL0102_REG_ACR_SHDN_ENABLE;
	ret = tpl010_write_reg(tpl, TPL0102_REG_ACR, tpl->ACR.value);
	return ret;
}
