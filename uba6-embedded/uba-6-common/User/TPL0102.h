/*
 * TPL0102.h
 *
 *  Created on: Sep 26, 2024
 *      Author: ORA
 */

#ifndef TPL0102_H_
#define TPL0102_H_

#include "i2c.h"

#define TPL0102_ADDRESS_BASE 	(0x50)
#define TPL0102_ADDRESS_A2A1A0 	(0x03)
#define TPL0102_ADDRESS (TPL0102_ADDRESS_BASE|TPL0102_ADDRESS_A2A1A0)
#define TPL0102_DELAY_STARUP (100) /*100 ms*/

#define TPL0102_T_wc (20) /*20 ms Non-volatile write cycle time*/
#define TPL0102_T_WRT (1) /*600 ns Wiper response time*/

#define TPL0102_REG_IVRA (0x00)
#define TPL0102_REG_WRA (0x00)
#define TPL0102_REG_IVRB (0x01)
#define TPL0102_REG_WRB (0x01)
#define TPL0102_REG_ACR (0x10)
#define TPL0102_REG_ARV (0x10)
#define TPL0102_REG_ACR_VOL_IVAR_ACCESS (0)
#define TPL0102_REG_ACR_VOL_WR_ACCESS (1)
#define TPL0102_REG_ACR_SHDN_ENABLE (0)
#define TPL0102_REG_ACR_SHDN_DISABLE (1)
#define TPL0102_REG_ACR_WIP_AVAILABLE (0)
#define TPL0102_REG_ACR_WIP_BUSY (1)

typedef enum TPL0102_StatusTypeDef {

	TPL0102_STATUS_OK = 0,
	TPL0102_STATUS_INTERFACE_ERROR = 0x01,
	TPL0102_STATUS_VALUE_ERROR = 0x02,
	TPL0102_STATUS_NULL_ERROR = 0x04,
	TPL0102_STATUS_BUSY = 0x08,
} TPL0102_StatusTypeDef;

typedef struct TPL0102 {
	I2C_HandleTypeDef *i2c_handle; // pointer to the i2c handle
	uint8_t address;
	union {
		struct {
			uint8_t dummy :5;
			uint8_t WIP :1; /*read only if WIP in 1 operation is in progress */
			uint8_t SHDN :1; /*Shutdown mode*/
			uint8_t VOL :1; /*Volatile Write/Read*/
		} flags;
		uint8_t value;
	} ACR;
	uint8_t WRA;
	uint8_t WRB;
	uint8_t IVRA;
	uint8_t IVRB;
} TPL0102;

#define TPL0102_DEFUALT				\
{									\
	NULL,							\
	TPL0102_ADDRESS,				\
	.ACR = {.value = 0},			\
	0,								\
	0,								\
}

TPL0102_StatusTypeDef tpl010_init(TPL0102 *tpl, I2C_HandleTypeDef *i2c_p);
TPL0102_StatusTypeDef tpl010_read_reg(TPL0102 *tpl, uint8_t reg, uint8_t *reg_value);
TPL0102_StatusTypeDef tpl010_write_reg(TPL0102 *tpl, uint8_t reg, uint8_t reg_value);
TPL0102_StatusTypeDef tpl010_set_potentiometer_a(TPL0102 *tpl, uint8_t new_value);
TPL0102_StatusTypeDef tpl010_set_potentiometer_b(TPL0102 *tpl, uint8_t new_value);
TPL0102_StatusTypeDef tpl010_set_potentiometer_a_inital_value(TPL0102 *tpl, uint8_t new_value);
TPL0102_StatusTypeDef tpl010_set_potentiometer_b_inital_value(TPL0102 *tpl, uint8_t new_value);
TPL0102_StatusTypeDef tpl010_shutdown(TPL0102 *tpl);

#endif /* TPL0102_H_ */
