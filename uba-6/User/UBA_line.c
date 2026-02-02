/*
 * UBA_line.c
 *
 *  Created on: Oct 8, 2024
 *      Author: ORA
 */

#include "UBA_line.h"
#include "uart_log.h"
#include "string.h"
#include "UBA_util.h"
UBA_line UBA_LINE_A = UBA_INTRENAL_LINE_DEFUALT;
UBA_line UBA_LINE_B = UBA_INTRENAL_LINE_DEFUALT;
#define UBA_LINE_IDLE_IADC_UPDATE (1000)

#define UBA_LINE_ADC_TO_VBA(adc_value, pos_value)  ((((adc_value *3300)/4096)*(2200+56000 + pos_value))/ (adc_value+2200))

#define UBA_VBA_ADC_MIN (1500)
#define UBA_VBA_ADC_MAX (2000)

#define UBA_COMP 		"LINE"

#if (UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_INFO)
#define UART_LOG_LINE_INFO(...) UART_LOG_INFO(line->name,##__VA_ARGS__)
#else
	#define UART_LOG_LINE_INFO(...)
#endif

#if UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_DEBUG
#define UART_LOG_LINE_DEBUG(...)  UART_LOG_DEBUG(line->name,##__VA_ARGS__)
#else
	#define UART_LOG_LINE_DEBUG(...)
#endif

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

void UBA_line_init_enter(UBA_line *line);
void UBA_line_init(UBA_line *line);
void UBA_line_init_exit(UBA_line *line);

void UBA_line_idle_enter(UBA_line *line);
void UBA_line_idle(UBA_line *line);
void UBA_line_idle_exit(UBA_line *line);

void UBA_line_pre_charging_enter(UBA_line *line);
void UBA_line_pre_charging(UBA_line *line);
void UBA_line_pre_charging_exit(UBA_line *line);

void UBA_line_charging_enter(UBA_line *line);
void UBA_line_charging(UBA_line *line);
void UBA_line_charging_exit(UBA_line *line);

void UBA_line_discharging_enter(UBA_line *line);
void UBA_line_discharging(UBA_line *line);
void UBA_line_discharging_exit(UBA_line *line);

void UBA_line_dead_enter(UBA_line *line);
void UBA_line_dead(UBA_line *line);
void UBA_line_dead_exit(UBA_line *line);

typedef void (*step_cb_t)(UBA_line *line);

/***
 * UBA Single Channel State Machine Assigner Rule
 */
struct UBASCSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};

/*UBA Single Channel State Machine Assigner */
#define UBASCSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

// @formatter:off
static const struct UBASCSMA_rule rule_g[UBA_LINE_STATE_MAX] ={
		UBASCSMA(UBA_LINE_STATE_INIT,		UBA_line_init_enter,		UBA_line_init,			UBA_line_init_exit),
		UBASCSMA(UBA_LINE_STATE_IDLE,		UBA_line_idle_enter,		UBA_line_idle,			UBA_line_idle_exit),
		UBASCSMA(UBA_LINE_STATE_PRE_CHARGING,UBA_line_pre_charging_enter,	UBA_line_pre_charging,		UBA_line_pre_charging_exit),
		UBASCSMA(UBA_LINE_STATE_CHARGING,	UBA_line_charging_enter,	UBA_line_charging,		UBA_line_charging_exit),
		UBASCSMA(UBA_LINE_STATE_DISCHARGING,UBA_line_discharging_enter,	UBA_line_discharging,	UBA_line_discharging_exit),
		UBASCSMA(UBA_LINE_STATE_DEAD,		UBA_line_dead_enter,		UBA_line_dead,			UBA_line_dead_exit),

};
// @formatter:on

//=================================================private functions========================================================//
void UBA_line_update_state(UBA_line *line) {
	UART_LOG_INFO(line->name, "update state %u ---> %u", line->state.current, line->state.next);
	line->state.pre = line->state.current;
	line->state.current = line->state.next;
	line->state.next = UBA_LINE_STATE_INVALID;
}

void UBA_line_read_line_values(UBA_line *line) {
	uint32_t adc_values[ADC_CH_MAX];  // Array to store the ADC values
	uint32_t adc_vbat = 0;
	uint32_t vbat = 0;
	uint32_t resistance = 0;
	if ((HAL_GetTick() - line->action_tick) > UBA_LINE_IDLE_IADC_UPDATE) {
		line->action_tick = HAL_GetTick();
		HAL_ADC_Start(line->adc_handle);
		for (int i = 0; i < ADC_CH_MAX; i++) {
			//	     Wait for the conversion to complete
			if (HAL_ADC_PollForConversion(line->adc_handle, 100) == HAL_OK) {
				adc_values[i] = HAL_ADC_GetValue(line->adc_handle);

				if (i == ADC_CH_VBAT) {
					if (adc_values[i] < UBA_VBA_ADC_MIN) {
						tpl010_set_potentiometer_b(&line->digital_potentiometer, line->digital_potentiometer.RB - 1);
					} else if (adc_values[i] > UBA_VBA_ADC_MAX) {
						tpl010_set_potentiometer_b(&line->digital_potentiometer, line->digital_potentiometer.RB + 1);
					}
					adc_vbat = ((adc_values[i] * 3300) / 0x1000);
					vbat = (adc_vbat * 108.2) / 52.2;
					resistance = tpl010_HB_resistance(&line->digital_potentiometer);
					vbat = (adc_vbat * (resistance + 2200 + 56000)) / (resistance + 2200);
					line->data.voltage = vbat;
					UART_LOG_LINE_DEBUG("ADC :%01u value:%04u adc_vbat:%04d vbat %04lu", i, adc_values[i], adc_vbat, vbat);
				}
				// Get the ADC value of the current line
			} else {
				UART_LOG_ERROR("ADC :%u fail pull ", i);
			}
		}
		HAL_ADC_Stop(line->adc_handle);  // Stop the ADC conversion
	}
	/*line->data.capacity++;
	line->data.current++;
	line->data.temperature++;
	line->data.voltage++;*/
}

//=================================================state machine functions========================================================//
void UBA_line_init_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter init");
	line->error = UBA_LINE_EWI_NO_ERROR;
	tpl010_init(&line->digital_potentiometer, line->i2c_handle);
	tpl010_set_potentiometer_b(&line->digital_potentiometer, 0x80);
	MCP3221_init(&line->EX_ADC, line->i2c_handle);
}

void UBA_line_init(UBA_line *line) {
	line->state.next = UBA_LINE_STATE_IDLE;
}

void UBA_line_init_exit(UBA_line *line) {

}

void UBA_line_idle_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter Idle");
}
void UBA_line_idle(UBA_line *line) {
	UBA_line_read_line_values(line);
}
void UBA_line_idle_exit(UBA_line *line) {
}

void UBA_line_pre_charging_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter pre Charge");
}

void UBA_line_pre_charging(UBA_line *line) {
	UBA_line_read_line_values(line);
}
void UBA_line_pre_charging_exit(UBA_line *line) {
}

void UBA_line_charging_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter Charge");

}

void UBA_line_charging(UBA_line *line) {
//	uint32_t current, voltage, temp, gen_voltage;
	UBA_line_read_line_values(line);

}

void UBA_line_charging_exit(UBA_line *line) {
}

void UBA_line_discharging_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter discharge");
}
void UBA_line_discharging(UBA_line *line) {
	UBA_line_read_line_values(line);
}
void UBA_line_discharging_exit(UBA_line *line) {
}

void UBA_line_dead_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter dead");
}

void UBA_line_dead(UBA_line *line) {
	UBA_line_read_line_values(line);
}

void UBA_line_dead_exit(UBA_line *line) {
}

void UBA_line_run(UBA_line *line) {
	if (line->state.next == UBA_LINE_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[line->state.current].run) {
			rule_g[line->state.current].run(line); // run the main function of the state
		}
	} else {
		if (line->state.current < UBA_LINE_STATE_MAX) {
			if (rule_g[line->state.current].exit) {
				rule_g[line->state.current].exit(line); // run the status exit function
			}
		}
		if (rule_g[line->state.next].enter) {
			rule_g[line->state.next].enter(line); // run the next state enter function
		}
	}
}

UBA_LINE_EWI UBA_line_set_next_state(UBA_line *line, UBA_LINE_STATE next_state) {
	line->state.next = next_state;
	return UBA_LINE_EWI_NO_ERROR;
}
void UBA_line_init_local_lines(void) {
	UBA_LINE_A.i2c_handle = &hi2c3;
	UBA_LINE_A.adc_handle = &hadc1;
	UBA_LINE_A.tim_handle = &htim1;
	memcpy(UBA_LINE_A.name, "Line A", 7);
	UBA_LINE_B.i2c_handle = &hi2c4;
	UBA_LINE_B.adc_handle = &hadc2;
	UBA_LINE_B.tim_handle = &htim8;
	memcpy(UBA_LINE_B.name, "Line B", 7);
}
