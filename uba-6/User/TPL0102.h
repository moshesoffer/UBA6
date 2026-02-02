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

#define TPL0102_REG_IVRA (0x00)
#define TPL0102_REG_WRA (0x00)
#define TPL0102_REG_IVRB (0x01)
#define TPL0102_REG_WRB (0x01)
#define TPL0102_REG_ACR (0x10)
#define TPL0102_REG_ARV (0x10)




typedef struct TPL0102 {
	I2C_HandleTypeDef *i2c_handle; // pointer to the i2c handle
	uint8_t	address;
	struct{
		uint8_t dummy: 5;
		uint8_t WIP:1; /*read only if WIP in 1 operation is in progress */
		uint8_t SHFN:1; /*Shutdown mode*/
		uint8_t VOL:1;  /*Volatile Write/Read*/
	}ACR;
	uint8_t RA;
	uint8_t RB;
} TPL0102;

#define TPL0102_DEFUALT				\
{									\
	NULL,							\
	TPL0102_ADDRESS,				\
	{0},							\
	0,								\
	0,								\
}

void tpl010_init(TPL0102 *tpl, I2C_HandleTypeDef *i2c_p);
void tpl010_read_reg(TPL0102* tpl ,uint8_t reg,uint8_t * reg_value);
void tpl010_write_reg(TPL0102 *tpl, uint8_t reg, uint8_t reg_value);
void tpl010_set_potentiometer_a(TPL0102 *tpl, uint8_t new_value);
void tpl010_set_potentiometer_b(TPL0102 *tpl, uint8_t new_value);
uint32_t tpl010_HA_resistance(TPL0102 *tpl);
uint32_t tpl010_HB_resistance(TPL0102 *tpl);
#endif /* TPL0102_H_ */
