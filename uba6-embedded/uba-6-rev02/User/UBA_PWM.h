/*
 * UBA_PWM.h
 *
 *  Created on: Nov 19, 2024
 *      Author: ORA
 */

#ifndef UBA_PWM_H_
#define UBA_PWM_H_
#include "UBA_common_def.h"
#include "tim.h"
#include "stddef.h"

#define UBA_PWM_BUFFER_SIZE (16)
#define CYCLES						16
#define TIM_CYCLE					0x3FF	/* Maximum counts for one period */
#define DEAD_TIME					10	// Dead time for Timer Channel 1
#define DEAD_TIME_CH2					10	// Dead time for Timer Channel 2
#define PWM_50	 					(TIM_CYCLE-1)/2

#define T_ON_BUCK_MIN					0x40		//Minimum Buck "ON" Time
#define T_ON_BUCK_MAX					(TIM_CYCLE-1)-(2*T_ON_BUCK_MIN)	 	// Maximum Buck "ON" Time	(0x33F)
#define T_ON_BOOST_MIN					(T_ON_BUCK_MIN - 2*DEAD_TIME)			//Minimum BOOST "ON" Time, Have to be smaller than BUCK
#define T_ON_BOOST_MAX					(T_ON_BUCK_MAX- 40*DEAD_TIME)		// Maximum Boost "ON" Time, Have to be smaller than BUCK

typedef struct {
	TIM_HandleTypeDef *handle; /*the handle of the PWM*/
	uint32_t channel; /*the PWM channel */
	uint32_t acc_dma_buff[UBA_PWM_BUFFER_SIZE];
	uint32_t min_value;
	uint32_t max_value;
	uint32_t value;
	volatile uint32_t value_2_set;
} UBA_PWM;

#define UBA_PWM_DEFUALT 		\
{								\
	NULL,						\
	0,							\
	{0},						\
	0,							\
	1,							\
	1,							\
	1							\
}

void UBA_PWM_init(UBA_PWM *pwm);
void UBA_PWM_start(UBA_PWM *pwm);
void UBA_PWM_stop(UBA_PWM *pwm);
UBA_STATUS_CODE UBA_PWM_update(UBA_PWM *pwm, uint32_t new_value);

#endif /* UBA_PWM_H_ */
