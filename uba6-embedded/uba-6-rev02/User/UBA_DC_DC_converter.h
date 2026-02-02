/*
 * UBA_buck_boost.h
 *
 *  Created on: Jan 29, 2025
 *      Author: ORA
 */

#ifndef UBA_DC_DC_CONVERTER_H_
#define UBA_DC_DC_CONVERTER_H_
#include "UBA_PWM.h"

#define UBA_DC_DC_BUCK_MIN_DUTY_CYCLE 10
#define UBA_DC_DC_BUCK_MAX_DUTY_CYCLE 95
#define UBA_DC_DC_BOOST_MIN_DUTY_CYCLE (UBA_DC_DC_BUCK_MIN_DUTY_CYCLE -5)
#define UBA_DC_DC_BOOST_MAX_DUTY_CYCLE (UBA_DC_DC_BUCK_MAX_DUTY_CYCLE -5)


typedef enum {
	UBA_DC_DC_CONVERTER_STATE_INIT,
	UBA_DC_DC_CONVERTER_STATE_BUCK,/*sw1 Ton sw Toff sw3 1 sw4 0 */
	UBA_DC_DC_CONVERTER_STATE_BOOST,/**/
	UBA_DC_DC_CONVERTER_STATE_BUCK_BOOST,/**/
	UBA_DC_DC_CONVERTER_STATE_BUCK_BOOST_UP,/**/
	UBA_DC_DC_CONVERTER_STATE_BUCK_BOOST_DOWN,/**/
	UBA_DC_DC_CONVERTER_STATE_MAX,
	UBA_DC_DC_CONVERTER_STATE_INVALID,
}UBA_DC_DC_STATE;

typedef enum {
	UBA_DC_DC_CONVERTER_MODE_VOLTAGE,
	UBA_DC_DC_CONVERTER_MODE_PWM,
}UBA_DC_DC_CONVERTER_MODE;

typedef struct UBA_DC_DC{
	struct{
		UBA_DC_DC_STATE pre;/*the previous state*/
		UBA_DC_DC_STATE current; /*the current state*/
		UBA_DC_DC_STATE next;/*next state id there is any*/
		int32_t tick;/*the time the line enter a new state*/
	}state;
	UBA_PWM SW1_2_BUCK; /*switch 1 and switch 2 PWM SW1 =!SW2*/
	UBA_PWM SW3_4_BOOST;/*switch 3 and switch 4 PWM  SW3 =!SW4*/
	uint32_t pwm_value;
	int32_t * p_vps;
	uint32_t update_time;
}UBA_DC_DC;

#define UBA_DC_DC_DEFUALT 				\
{										\
	{									\
		UBA_DC_DC_CONVERTER_STATE_INVALID,	\
		UBA_DC_DC_CONVERTER_STATE_INVALID,	\
		UBA_DC_DC_CONVERTER_STATE_INIT,	\
		0,								\
	},									\
	UBA_PWM_DEFUALT,					\
	UBA_PWM_DEFUALT,					\
	0,									\
	NULL,								\
	0									\
}


void UBA_DCDC_run(UBA_DC_DC *dcdc);
uint32_t UBA_DCDC_bock_boost_down(UBA_DC_DC *dcdc);
uint32_t UBA_DCDC_bock_boost_up(UBA_DC_DC *dcdc, uint8_t step_size);
#endif /* UBA_DC_DC_CONVERTER_H_ */

