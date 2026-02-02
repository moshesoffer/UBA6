/*
 * UBA_6.c
 *
 *  Created on: Sep 24, 2024
 *      Author: ORA
 */

#include "UBA_6.h"
#include "LCD.h"
#include "uart_log.h"
#include "UBA_battery_performance_test.h"

#define UBA_COMP "UBA"
UBA_6 UBA_6_device_g = UBA_6_DEFUALT;

void UBA_6_init_enter(UBA_6 *uba);
void UBA_6_init(UBA_6 *uba);
void UBA_6_init_exit(UBA_6 *uba);
void UBA_6_single_channels_enter(UBA_6 *uba);
void UBA_6_single_channels(UBA_6 *uba);
void UBA_6_single_channels_exit(UBA_6 *uba);
void UBA_6_dual_channel_enter(UBA_6 *uba);
void UBA_6_dual_channel(UBA_6 *uba);
void UBA_6_dual_channel_exit(UBA_6 *uba);

typedef void (*step_cb_t)(UBA_6 *uba);
/***
 * UBA State Machine Assigner Rule
 */
struct UBASMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};
/*UBA State Machine Assigner */
#define UBASMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

//@formatter:off

static const struct UBASMA_rule rule_g[UBA_6_STATE_MAX] ={
		//				State
		UBASMA(UBA_6_STATE_INIT,				UBA_6_init_enter,				UBA_6_init,			UBA_6_init_exit),
		UBASMA(UBA_6_STATE_SINGLE_CHANNELS,		UBA_6_single_channels_enter,	UBA_6_single_channels,	UBA_6_single_channels_exit),
		UBASMA(UBA_6_STATE_DUAL_CHANNEL,		UBA_6_dual_channel_enter,		UBA_6_dual_channel,	UBA_6_dual_channel_exit),
};
//@formatter:on

void UBA_6_update_state(UBA_6 *uba) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", uba->state.current, uba->state.next);
	uba->state.pre = uba->state.current;
	uba->state.current = uba->state.next;
	uba->state.next = UBA_6_STATE_INVALID;
}

void UBA_6_init_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
}
void UBA_6_init(UBA_6 *uba) {
	UBA_6_set_next_state(uba, UBA_6_STATE_SINGLE_CHANNELS);
}
void UBA_6_init_exit(UBA_6 *uba) {
}
void UBA_6_single_channels_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
	UBA_LCD_g.state.next = UBA_LCD_STATE_SIDE_BY_SIDE;
}

void UBA_6_single_channels(UBA_6 *uba) {
	if (uba->BPT_A.type == UBA_BPT_TYPE_SINGLE_CHANNEL) {
		UBA_BPT_run(&uba->BPT_A);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "A Test is not single channel");
	}
	if (uba->BPT_B.type == UBA_BPT_TYPE_SINGLE_CHANNEL) {
		UBA_BPT_run(&uba->BPT_B);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "B Test is not single channel");
	}
}

void UBA_6_single_channels_exit(UBA_6 *uba) {
}
void UBA_6_dual_channel_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
	UBA_LCD_g.state.next = UBA_LCD_STATE_FULL_SCREEN;
}

void UBA_6_dual_channel(UBA_6 *uba) {
	if (uba->BPT_AB.type == UBA_BPT_TYPE_DUAL_CHANNEL) {
		UBA_BPT_run(&uba->BPT_AB);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "AB Test is not dual channel");
	}

}

void UBA_6_dual_channel_exit(UBA_6 *uba) {
}

void UBA_6_run(UBA_6 *uba) {
	if (uba->state.next == UBA_6_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[uba->state.current].run) {
			rule_g[uba->state.current].run(uba); // run the main function of the state
		}
	} else {
		if (uba->state.current < UBA_6_STATE_MAX) {
			if (rule_g[uba->state.current].exit) {
				rule_g[uba->state.current].exit(uba); // run the status exit function
			}
		}
		if (rule_g[uba->state.next].enter) {
			rule_g[uba->state.next].enter(uba); // run the next state enter function
		}
	}
}

UBA_6_ERROR UBA_6_set_next_state(UBA_6 *uba, UBA_6_STATE next_state) {
	UBA_6_ERROR ret = UBA_6_ERROR_NO_ERROR;
	uba->state.next = next_state;
	return ret;
}
