/*
 * UBA_line.h
 *
 *  Created on: Oct 8, 2024
 *      Author: ORA
 */

#ifndef UBA_LINE_H_
#define UBA_LINE_H_

#include "gpio.h"
#include "i2c.h"
#include "stdbool.h"
#include "stddef.h"
#include "MCP47X6.h"
#include "TPL0102.h"
#include "MCP3221.h"
#include "tim.h"
#include "adc.h"
#include "UBA_common_def.h"


typedef enum UBA_LINE_STATE {
	UBA_LINE_STATE_INIT ,
	UBA_LINE_STATE_IDLE ,
	UBA_LINE_STATE_PRE_CHARGING,
	UBA_LINE_STATE_CHARGING,
	UBA_LINE_STATE_DISCHARGING ,
	UBA_LINE_STATE_DEAD,
	UBA_LINE_STATE_MAX,
	UBA_LINE_STATE_INVALID = 0xff,

} UBA_LINE_STATE;

typedef enum UBA_LINE_EWI {
	UBA_LINE_EWI_NO_ERROR = 0x0000,
	UBA_LINE_EWI_LEVEL_WARNING = UBA_WARNING,
	UBA_LINE_EWI_LEVEL_ERROR = UBA_ERROR,
	UBA_LINE_EWI_LEVEL_CRITICAL = UBA_CRITICAL|UBA_LINE_EWI_LEVEL_ERROR,
	UBA_LINE_ERROR_NOT_AVAILABLE = 0x0001|UBA_LINE_EWI_LEVEL_CRITICAL,
	UBA_LINE_ERROR_TEMP = 0x0002|UBA_LINE_EWI_LEVEL_ERROR,
	UBA_LINE_ERROR_DUAL_CHANNEL_TEST = 0x0004 |UBA_LINE_EWI_LEVEL_WARNING,
	UBA_LINE_ERROR_DUAL_CHANNEL_BUSY = 0x0008 |UBA_LINE_EWI_LEVEL_ERROR,
} UBA_LINE_EWI;

typedef struct UBA_line {
	UBA_LINE_EWI error;
	uint8_t name[10];
	uint32_t action_tick;
	struct {
		UBA_LINE_STATE pre;
		UBA_LINE_STATE current;
		UBA_LINE_STATE next;
	} state;
	MCP47X6 EX_DAC;
	MCP3221 EX_ADC;
	TPL0102 digital_potentiometer;
	ADC_HandleTypeDef* adc_handle;
	I2C_HandleTypeDef* i2c_handle;
	TIM_HandleTypeDef* tim_handle;
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} enable;
	struct {
		int32_t voltage;
		int32_t temperature;
		int32_t current;
		int32_t capacity;
	} data;
	bool isBusy;
} UBA_line;
// @formatter:off

#define UBA_INTRENAL_LINE_DEFUALT 	\
{									\
	UBA_LINE_EWI_NO_ERROR,			\
	"LINE ?",						\
	0,								\
	{								\
		UBA_LINE_STATE_INIT,		\
		UBA_LINE_STATE_INIT,		\
		UBA_LINE_STATE_INIT			\
	},								\
	MCP47X6_DEFUALT,				\
	MCP3221_DEFUALT,				\
	TPL0102_DEFUALT,				\
	NULL,							\
	NULL,							\
	NULL,							\
	{0},							\
	{0},							\
	false							\
}
// @formatter:on


void UBA_line_run(UBA_line * line);
void UBA_line_init_local_lines(void);

UBA_LINE_EWI UBA_line_set_next_state(UBA_line * line, UBA_LINE_STATE next_state);

extern UBA_line UBA_LINE_A;
extern UBA_line UBA_LINE_B;

#endif /* UBA_LINE_H_ */
