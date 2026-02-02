/*
 * UBA_channel.c
 *
 *  Created on: Aug 29, 2024
 *      Author: ORA
 */

#include <UBA_channel.h>
#include "uart_log.h"
#include "adc.h"
#include "UBA_util.h"
#include "string.h"

UBA_channel UBA_CH_A;
UBA_channel UBA_CH_B;
UBA_channel UBA_CH_AB;

#define UBA_CHANNEL_ADC_TO_VBA(adc_value, pos_value)  ((((adc_value *3300)/4096)*(2200+56000 + pos_value))/ (adc_value+2200))

#if (UBA_LOG_LEVEL_CHANNEL < UART_LOG_LEVEL_INFO)
	#define UART_LOG_CHANNEL_INFO(...) UART_LOG_INFO(##__VA_ARGS__)
#else
#define UART_LOG_CHANNEL_INFO(...)
#endif

#if (UBA_LOG_LEVEL_CHANNEL < UART_LOG_LEVEL_DEBUG)
	#define UART_LOG_CHANNEL_DEBUG(...) UART_LOG_DEBUG(##__VA_ARGS__)
#else
#define UART_LOG_CHANNEL_DEBUG(...)
#endif

#define UBA_VBA_ADC_MIN (1500)
#define UBA_VBA_ADC_MAX (2000)

#define UBA_COMP 		"Channel"
/*#define ADC_CH_VBAT 		(0)
 #define ADC_CH_VGEN 		(1)
 #define ADC_CH_AMB_TEMP 	(2)
 #define ADC_CH_NTC_BAT 		(3)
 #define ADC_CH_DSCH_CURR 	(4)*/
typedef enum ADC_CH {
	ADC_CH_VBAT = 0x00, /*ADC1_IN1,	ADC2_IN14	*/
	ADC_CH_VGEN = 0x01, /*ADC1_IN2,	ADC2_IN3 	*/
	ADC_CH_AMB_TEMP = 0x02, /*ADC1_IN8,	ADC2_IN15 	*/
	ADC_CH_NTC_BAT = 0x03, /*ADC1_IN7,	ADC2_IN4 	*/
	ADC_CH_DSCH_CURR = 0x04,/*ADC1_IN6,	ADC2_IN11	*/
	ADC_CH_MAX,
	ADC_CH_T,
	ADC_CH_temp,
} ADC_CH;

void UBA_channel_init_enter(UBA_channel *ch);
void UBA_channel_init(UBA_channel *ch);
void UBA_channel_init_exit(UBA_channel *ch);

void UBA_channel_standby_enter(UBA_channel *ch);
void UBA_channel_standby(UBA_channel *ch);
void UBA_channel_standby_exit(UBA_channel *ch);

void UBA_channel_delay_enter(UBA_channel *ch);
void UBA_channel_delay(UBA_channel *ch);
void UBA_channel_delay_exit(UBA_channel *ch);

void UBA_channel_charging_enter(UBA_channel *ch);
void UBA_channel_charging(UBA_channel *ch);
void UBA_channel_charging_exit(UBA_channel *ch);

void UBA_channel_discharging_enter(UBA_channel *ch);
void UBA_channel_discharging(UBA_channel *ch);
void UBA_channel_discharging_exit(UBA_channel *ch);

void UBA_channel_dead_enter(UBA_channel *ch);
void UBA_channel_dead(UBA_channel *ch);
void UBA_channel_dead_exit(UBA_channel *ch);

typedef void (*step_cb_t)(UBA_channel *ch);

/***
 * UBA Channel State Machine Assigner Rule
 */
struct UBACSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};

/*UBA Channel State Machine Assigner */
#define UBACSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

// @formatter:off
static const struct UBACSMA_rule rule_g[UBA_CHANNEL_STATE_MAX] ={
		UBACSMA(UBA_CHANNEL_STATE_INIT,		UBA_channel_init_enter,			UBA_channel_init,			UBA_channel_init_exit),
		UBACSMA(UBA_CHANNEL_STATE_STANDBY,	UBA_channel_standby_enter,		UBA_channel_standby,		UBA_channel_standby_exit),
		UBACSMA(UBA_CHANNEL_STATE_DELAY,	UBA_channel_delay_enter,		UBA_channel_delay,			UBA_channel_delay_exit),
		UBACSMA(UBA_CHANNEL_STATE_CHARGE,	UBA_channel_charging_enter,		UBA_channel_charging,		UBA_channel_charging_exit),
		UBACSMA(UBA_CHANNEL_STATE_DISCHARGE,UBA_channel_discharging_enter,	UBA_channel_discharging,	UBA_channel_discharging_exit),
		UBACSMA(UBA_CHANNEL_STATE_OFF,		UBA_channel_dead_enter,			UBA_channel_dead,			UBA_channel_dead_exit),

};
// @formatter:on

//=================================================private functions========================================================//
void UBA_channel_update_state(UBA_channel *ch) {
	UART_LOG_INFO(ch->name, "update state %u ---> %u", ch->state.current, ch->state.next);
	ch->state.pre = ch->state.current;
	ch->state.current = ch->state.next;
	ch->state.next = UBA_CHANNEL_STATE_INVALID;
}

//=================================================state machine functions========================================================//
void UBA_channel_init_enter(UBA_channel *ch) {
	uint8_t line_index = 0;
	UBA_channel_update_state(ch);
	ch->error = UBA_CHANNLE_ERROR_NO_ERROR;
	if ((ch->id & UBA_CHANNLE_ID_A) == UBA_CHANNLE_ID_A) {
		ch->lines_p[line_index++] = &UBA_LINE_A;
	}
	if ((ch->id & UBA_CHANNLE_ID_B) == UBA_CHANNLE_ID_B) {
		ch->lines_p[line_index++] = &UBA_LINE_B;
	}
	if (line_index == 0) {
		UART_LOG_CRITICAL(ch->name, "init Failed , channel dose not have any connectec lines");
	}
	ch->line_size = line_index;

}

void UBA_channel_init(UBA_channel *ch) {
	if (ch->line_size) {
		ch->state.next = UBA_CHANNEL_STATE_STANDBY;
	} else {
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
}

void UBA_channel_init_exit(UBA_channel *ch) {
	UNUSED(ch);
}

void UBA_channel_standby_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->state.next = UBA_LINE_STATE_IDLE;
	}
}

void UBA_channel_standby(UBA_channel *ch) {

}

void UBA_channel_standby_exit(UBA_channel *ch) {

}

void UBA_channel_delay_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->state.next = UBA_LINE_STATE_IDLE;
	}
}

void UBA_channel_delay(UBA_channel *ch) {

}

void UBA_channel_delay_exit(UBA_channel *ch) {

}

void UBA_channel_charging_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->state.next = UBA_LINE_STATE_PRE_CHARGING;
	}
}

void UBA_channel_charging(UBA_channel *ch) {
}
void UBA_channel_charging_exit(UBA_channel *ch) {
}

void UBA_channel_discharging_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->state.next = UBA_LINE_STATE_DISCHARGING;
	}
}
void UBA_channel_discharging(UBA_channel *ch) {
}
void UBA_channel_discharging_exit(UBA_channel *ch) {
}

void UBA_channel_dead_enter(UBA_channel *ch) {
	UBA_channel_update_state(ch);
}
void UBA_channel_dead(UBA_channel *ch) {
}
void UBA_channel_dead_exit(UBA_channel *ch) {
}

//=================================================public functions========================================================//

uint32_t UBA_channel_get_voltage(UBA_channel *ch) {
	uint32_t voltage = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		voltage += ch->lines_p[index]->data.voltage;
	}
	if (ch->line_size) {
		voltage /= ch->line_size;
		UART_LOG_CHANNEL_INFO(ch->name, "Voltage:%u", voltage);
	} else {
		UART_LOG_ERROR(ch->name, "line size is zero ");
	}
	return voltage;
}
uint32_t UBA_channel_get_temperature(UBA_channel *ch) {
	uint32_t temp = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		temp = UBA_LCD_MAX(temp, ch->lines_p[index]->data.temperature);
	}
	UART_LOG_CHANNEL_INFO(ch->name, "Temperature:%u", temp);
	return temp;
}
uint32_t UBA_channel_get_current(UBA_channel *ch) {
	uint32_t current = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		current += ch->lines_p[index]->data.current;
	}
	UART_LOG_CHANNEL_INFO(ch->name, "Current:%u", current);
	return current;
}
uint32_t UBA_channel_get_capacity(UBA_channel *ch) {
	uint32_t capacity = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		capacity += ch->lines_p[index]->data.capacity;
	}
	UART_LOG_CHANNEL_INFO(ch->name, "Capacity:%u", capacity);
	return capacity;
}
uint32_t UBA_channel_set_next_state(UBA_channel *ch, UBA_CHANNEL_STATE next_state) {
	ch->state.next = next_state;
	return 0;
}

void UBA_channel_run(UBA_channel *ch) {
	if (ch->state.next == UBA_CHANNEL_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[ch->state.current].run) {
			rule_g[ch->state.current].run(ch); // run the main function of the state
		}
	} else {
		if (ch->state.current < UBA_CHANNEL_STATE_MAX) {
			if (rule_g[ch->state.current].exit) {
				rule_g[ch->state.current].exit(ch); // run the status exit function
			}
		}
		if (rule_g[ch->state.next].enter) {
			rule_g[ch->state.next].enter(ch); // run the next state enter function
		}
	}
}

bool UBA_channel_isInCriticalError(UBA_channel *ch) {
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		if ((ch->lines_p[index]->error & UBA_LINE_EWI_LEVEL_CRITICAL) == UBA_LINE_EWI_LEVEL_CRITICAL) {
			return true;
		}
	}
	return false;
}
void UBA_channel_init_g(void) {

	UBA_CH_A.id = UBA_CHANNLE_ID_A;
	memcpy(UBA_CH_A.name, "Channel A", 10);
	UBA_CH_B.id = UBA_CHANNLE_ID_B;
	memcpy(UBA_CH_B.name, "Channel B", 10);
	UBA_CH_AB.id = UBA_CHANNLE_ID_AB;
	memcpy(UBA_CH_AB.name, "Channel AB", 11);
}

void UBA_channle_start_cmd(UBA_channel *uba_ch) {

}

