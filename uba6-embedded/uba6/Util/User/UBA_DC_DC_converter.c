/*
 * UBA_buck_boost.c
 *
 *  Created on: Jan 29, 2025
 *      Author: ORA
 */

#include "UBA_DC_DC_converter.h"
#include "uart_log.h"
#include "UBA_6.h"
#include "UBA_line.h"
typedef void (*step_cb_t)(UBA_DC_DC *dcdc);

/***
 * UBA DC DC  State Machine Assigner Rule
 */
struct UBADCDCSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
	char *name;
};

static void UBA_DCDC_init_enter(UBA_DC_DC *dcdc);
static void UBA_DCDC_init(UBA_DC_DC *dcdc);
static void UBA_DCDC_init_exit(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck_enter(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck_exit(UBA_DC_DC *dcdc);
static void UBA_DCDC_boost_enter(UBA_DC_DC *dcdc);
static void UBA_DCDC_boost(UBA_DC_DC *dcdc);
static void UBA_DCDC_boost_exit(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck_boost_enter(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck_boost(UBA_DC_DC *dcdc);
static void UBA_DCDC_buck_boost_exit(UBA_DC_DC *dcdc);

#define MAX_VOLTAGE (20000)
#define MAX_EXPECTED_VOLTAGE (66000) /*66V*/
#define UPDATED_TIME 100

#define UBA_COMP "DC2DC"

#define PWM_VALUE(tim_handle,x_value) (((__HAL_TIM_GET_AUTORELOAD(tim_handle))*x_value /100))

/*UBA DC DC State Machine Assigner */
#define UBADCDCSMA(step, cbe, cbr, cbx)[UBA_DC_DC_CONVERTER_STATE_##step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx ,.name = #step}

// @formatter:off
static const struct UBADCDCSMA_rule rule_g[UBA_DC_DC_CONVERTER_STATE_MAX] ={
		UBADCDCSMA(INIT,		UBA_DCDC_init_enter,		UBA_DCDC_init,		UBA_DCDC_init_exit),
		UBADCDCSMA(BUCK,		UBA_DCDC_buck_enter,		UBA_DCDC_buck,		UBA_DCDC_buck_exit),
		UBADCDCSMA(BOOST,		UBA_DCDC_boost_enter,		UBA_DCDC_boost,		UBA_DCDC_boost_exit),
		UBADCDCSMA(BUCK_BOOST,	UBA_DCDC_buck_boost_enter,	UBA_DCDC_buck_boost,UBA_DCDC_buck_boost_exit),
		UBADCDCSMA(BUCK_BOOST_UP,	UBA_DCDC_buck_boost_enter,	UBA_DCDC_buck_boost,UBA_DCDC_buck_boost_exit),
		UBADCDCSMA(BUCK_BOOST_DOWN,	UBA_DCDC_buck_boost_enter,	UBA_DCDC_buck_boost,UBA_DCDC_buck_boost_exit),
};
// @formatter:on

//================== private function=========================================================

/**
 * @brief claclulte the expected value of Vgem in mV
 *
 * @param dcdc a UBA_DC_DC pointer
 * @param v_in the value of the voltage input in mV
 * @return the expected vgen in mV
 */
uint32_t UBA_DCDC_expected_vgen(UBA_DC_DC *dcdc, uint32_t v_in) {
	uint32_t expected_vgen = 0;
	return expected_vgen;
}

bool UBA_DCDC_calc_duty_cycles(UBA_DC_DC *dcdc, uint32_t v_in, uint32_t v_gen, uint32_t *duty_cycle_buck, uint32_t *duty_cycle_boost) {
	uint32_t d_boost = UBA_DC_DC_BOOST_MIN_DUTY_CYCLE;
	uint32_t d_buck = UBA_DC_DC_BUCK_MIN_DUTY_CYCLE;

	do {
		d_buck = (v_gen * (100 - d_boost++) / v_in);
	} while ((d_buck < UBA_DC_DC_BUCK_MIN_DUTY_CYCLE) || (d_buck > UBA_DC_DC_BUCK_MAX_DUTY_CYCLE));

	return true;
}

static void UBA_DCDC_update_state(UBA_DC_DC *dcdc) {
	if ((dcdc->state.current < UBA_DC_DC_CONVERTER_STATE_MAX) && (dcdc->state.next < UBA_DC_DC_CONVERTER_STATE_MAX)) {
		UART_LOG_INFO(UBA_COMP, "update state %s ---> %s", rule_g[dcdc->state.current].name, rule_g[dcdc->state.next].name);
	} else {
		UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", dcdc->state.current, dcdc->state.next);
	}
	dcdc->state.pre = dcdc->state.current;
	dcdc->state.current = dcdc->state.next;
	dcdc->state.next = UBA_DC_DC_CONVERTER_STATE_INVALID;
	dcdc->state.tick = HAL_GetTick();
}

/**
 * @brief turn off the HV and the line to the battery
 * SW1 off SW3 is off
 * @param dcdc
 */
static void UBA_DCDC_init_enter(UBA_DC_DC *dcdc) {
	UBA_DCDC_update_state(dcdc);
}

static void UBA_DCDC_init(UBA_DC_DC *dcdc) {
	if (HAL_GetTick() - dcdc->state.tick > 1000) {
		dcdc->state.next = UBA_DC_DC_CONVERTER_STATE_BUCK_BOOST;
	}
}

static void UBA_DCDC_init_exit(UBA_DC_DC *dcdc) {

}

static void UBA_DCDC_buck_enter(UBA_DC_DC *dcdc) {
	UBA_DCDC_update_state(dcdc);
	UBA_PWM_init(&dcdc->SW1_2_BUCK);
	UBA_PWM_init(&dcdc->SW3_4_BOOST);
	dcdc->SW1_2_BUCK.min_value = 0;
	dcdc->SW1_2_BUCK.max_value = __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle);
	UBA_PWM_update(&dcdc->SW1_2_BUCK, __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle) / 2);
	dcdc->SW3_4_BOOST.min_value = __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle);
	dcdc->SW3_4_BOOST.max_value = __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle);
//	UBA_PWM_update(&dcdc->SW3_4, (__HAL_TIM_GET_AUTORELOAD(dcdc->SW3_4_BOOST.handle)/2)); // SW3 ON SW4 OFF
	UBA_PWM_update(&dcdc->SW3_4_BOOST, 0); // SW3 ON SW4 OFF

	UBA_PWM_start(&dcdc->SW1_2_BUCK);
	HAL_Delay(3);
	UBA_PWM_start(&dcdc->SW3_4_BOOST);
}

static void UBA_DCDC_buck(UBA_DC_DC *dcdc) {
	/*	if (HAL_GetTick() - dcdc->state.tick > 10000) {
	 dcdc->state.next = UBA_DC_DC_CONVERTER_STATE_BOOST;
	 }
	 */
}

static void UBA_DCDC_buck_exit(UBA_DC_DC *dcdc) {
	HAL_GPIO_WritePin(CHRG_EN_CH2_GPIO_Port, CHRG_EN_CH2_Pin, GPIO_PIN_RESET);
	UBA_PWM_init(&dcdc->SW1_2_BUCK);
	HAL_Delay(1);
	UBA_PWM_init(&dcdc->SW3_4_BOOST);
}

static void UBA_DCDC_boost_enter(UBA_DC_DC *dcdc) {
	UBA_DCDC_update_state(dcdc);
	UBA_PWM_init(&dcdc->SW1_2_BUCK);
	UBA_PWM_init(&dcdc->SW3_4_BOOST);
	UBA_PWM_update(&dcdc->SW1_2_BUCK, 0); //SW1 on SW2 OFF
	UBA_PWM_update(&dcdc->SW3_4_BOOST, ((__HAL_TIM_GET_AUTORELOAD(dcdc->SW3_4_BOOST.handle)) / 2));
	UBA_PWM_start(&dcdc->SW3_4_BOOST);
	HAL_Delay(10);
	UBA_PWM_start(&dcdc->SW1_2_BUCK);
}
static void UBA_DCDC_boost(UBA_DC_DC *dcdc) {
	/*if (HAL_GetTick() - dcdc->state.tick > 10000) {
	 dcdc->state.next = UBA_DC_DC_CONVERTER_STATE_BUCK;
	 }*/
}
static void UBA_DCDC_boost_exit(UBA_DC_DC *dcdc) {

}
static void UBA_DCDC_buck_boost_enter(UBA_DC_DC *dcdc) {
	UBA_DCDC_update_state(dcdc);
	dcdc->SW1_2_BUCK.min_value = PWM_VALUE(dcdc->SW1_2_BUCK.handle, UBA_DC_DC_BUCK_MIN_DUTY_CYCLE);
	dcdc->SW1_2_BUCK.max_value = PWM_VALUE(dcdc->SW1_2_BUCK.handle, UBA_DC_DC_BUCK_MAX_DUTY_CYCLE);
	dcdc->SW3_4_BOOST.min_value = PWM_VALUE(dcdc->SW3_4_BOOST.handle, UBA_DC_DC_BOOST_MIN_DUTY_CYCLE);
	dcdc->SW3_4_BOOST.max_value = PWM_VALUE(dcdc->SW3_4_BOOST.handle, UBA_DC_DC_BOOST_MAX_DUTY_CYCLE);
	UBA_PWM_init(&dcdc->SW1_2_BUCK);
	UBA_PWM_init(&dcdc->SW3_4_BOOST);
	dcdc->SW1_2_BUCK.value_2_set = PWM_VALUE(dcdc->SW1_2_BUCK.handle, 15);
	dcdc->SW3_4_BOOST.value_2_set = PWM_VALUE(dcdc->SW3_4_BOOST.handle, 10);

	UBA_PWM_update(&dcdc->SW1_2_BUCK, PWM_VALUE(dcdc->SW1_2_BUCK.handle, 15)); //SW1 on SW2 OFF
	UBA_PWM_update(&dcdc->SW3_4_BOOST, PWM_VALUE(dcdc->SW3_4_BOOST.handle, 10));
	UBA_PWM_start(&dcdc->SW3_4_BOOST);
	HAL_Delay(10);
	UBA_PWM_start(&dcdc->SW1_2_BUCK);
}

static void UBA_DCDC_buck_boost(UBA_DC_DC *dcdc) {
	if (HAL_GetTick() - dcdc->update_time > UPDATED_TIME) {
		dcdc->update_time = HAL_GetTick();
		if (dcdc->SW1_2_BUCK.value != dcdc->SW1_2_BUCK.value_2_set) {
			UBA_PWM_update(&dcdc->SW1_2_BUCK, dcdc->SW1_2_BUCK.value_2_set);
		} else if (dcdc->SW3_4_BOOST.value != dcdc->SW3_4_BOOST.value_2_set) {
			UBA_PWM_update(&dcdc->SW3_4_BOOST, dcdc->SW3_4_BOOST.value_2_set);
		}
	}

}
static void UBA_DCDC_buck_boost_exit(UBA_DC_DC *dcdc) {

}

//========================================== public functions===========================================================

uint32_t UBA_DCDC_bock_boost_down(UBA_DC_DC *dcdc) {
	uint32_t buck_duty_cycle = (dcdc->SW1_2_BUCK.value * 100) / __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle);
	uint32_t boost_duty_cycle = (dcdc->SW3_4_BOOST.value * 100) / __HAL_TIM_GET_AUTORELOAD(dcdc->SW3_4_BOOST.handle);
	if (*(dcdc->p_vps) < UBA_VPS_MIN_VALUE) {
		UART_LOG_CRITICAL(UBA_COMP, "Can't calculate Value (%u) VPS low", *(dcdc->p_vps));
		return UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE;
	}
	uint32_t expected_vgen = (*(dcdc->p_vps) * buck_duty_cycle / (100 - boost_duty_cycle));
	if (expected_vgen > MAX_EXPECTED_VOLTAGE) {
		UART_LOG_CRITICAL(UBA_COMP, "Expected Vgen (%lu) is OOB", expected_vgen);
		return UBA_PROTO_UBA6_ERROR_LINE_VGEN_EXPECTED_MAX_VOLTAGE;
	}
	UART_LOG_INFO(UBA_COMP, "Step Down Buck Duty Cycle:%03lu%% Boost Duty Cycle:%03lu%% Expected:%03lu mV", buck_duty_cycle, boost_duty_cycle,
			expected_vgen);
	if (buck_duty_cycle < (boost_duty_cycle + 5)) {
		UART_LOG_CRITICAL(UBA_COMP, "Delta Duty Cycle is OOB");
		return UBA_PROTO_UBA6_ERROR_LINE_VGEN_FAILED;
	}

	if (boost_duty_cycle > UBA_DC_DC_BOOST_MIN_DUTY_CYCLE) {
		dcdc->SW3_4_BOOST.value_2_set = dcdc->SW3_4_BOOST.value - 1;
	} else if (buck_duty_cycle > UBA_DC_DC_BUCK_MIN_DUTY_CYCLE) {
		dcdc->SW1_2_BUCK.value_2_set = dcdc->SW1_2_BUCK.value - 1;
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "Duty Cycle at the limits");
		return UBA_PROTO_UBA6_ERROR_LINE_VGEN_LIMITES;
	}
	if ((dcdc->SW3_4_BOOST.value_2_set < dcdc->SW3_4_BOOST.max_value) && (dcdc->SW3_4_BOOST.value_2_set > dcdc->SW3_4_BOOST.min_value)) {

	}
	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

uint32_t UBA_DCDC_bock_boost_up(UBA_DC_DC *dcdc , uint8_t step_size) {
	uint32_t buck_duty_cycle = (dcdc->SW1_2_BUCK.value * 100) / __HAL_TIM_GET_AUTORELOAD(dcdc->SW1_2_BUCK.handle);
	uint32_t boost_duty_cycle = (dcdc->SW3_4_BOOST.value * 100) / __HAL_TIM_GET_AUTORELOAD(dcdc->SW3_4_BOOST.handle);
	if (*(dcdc->p_vps) < UBA_VPS_MIN_VALUE) {
		UART_LOG_CRITICAL(UBA_COMP, "Can't calculate Value (%u) VPS low", *(dcdc->p_vps));
		return UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE;
	}
	uint32_t expected_vgen = (*(dcdc->p_vps) * buck_duty_cycle / (100 - boost_duty_cycle));

	if (expected_vgen > MAX_EXPECTED_VOLTAGE) {
		UART_LOG_CRITICAL(UBA_COMP, "Expected Vgen (%lu) is OOB", expected_vgen);
		return UBA_PROTO_UBA6_ERROR_LINE_VGEN_EXPECTED_MAX_VOLTAGE;
	}

	UART_LOG_INFO(UBA_COMP, "Step Up Buck Duty Cycle:%03lu%% Boost Duty Cycle:%03lu%% Expected:%03lu mV", buck_duty_cycle, boost_duty_cycle,
			expected_vgen);
	if (buck_duty_cycle < UBA_DC_DC_BUCK_MAX_DUTY_CYCLE - step_size) {
		dcdc->SW1_2_BUCK.value_2_set = dcdc->SW1_2_BUCK.value + step_size;
	} else if (boost_duty_cycle < UBA_DC_DC_BOOST_MAX_DUTY_CYCLE - step_size) {
		dcdc->SW3_4_BOOST.value_2_set = dcdc->SW3_4_BOOST.value + step_size;
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "Duty Cycle at the limits");
		return UBA_PROTO_UBA6_ERROR_LINE_VGEN_LIMITES;
	}
	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

void UBA_DCDC_run(UBA_DC_DC *dcdc) {
	if (dcdc->state.next == UBA_DC_DC_CONVERTER_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[dcdc->state.current].run) {
			rule_g[dcdc->state.current].run(dcdc); // run the main function of the state
		}
	} else {
		if (dcdc->state.current < UBA_DC_DC_CONVERTER_STATE_MAX) {
			if (rule_g[dcdc->state.current].exit) {
				rule_g[dcdc->state.current].exit(dcdc); // run the status exit function
			}
		}
		if (rule_g[dcdc->state.next].enter) {
			rule_g[dcdc->state.next].enter(dcdc); // run the next state enter function
		}
	}
}

