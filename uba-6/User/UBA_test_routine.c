/*
 * UBA_test_routine.c
 *
 *  Created on: Sep 25, 2024
 *      Author: ORA
 */

#include "UBA_test_routine.h"
#include "stdlib.h"
#include "UBA_battery_performance_test.h"
#include "string.h"
#include "uart_log.h"
#include "UBA_6.h"

#define UBA_COMP "TEST ROUTINE"

UBA_TR TR_list[UBA_TR_LIST_SIZE] = { UBA_TR_DEMO, UBA_TR_DUAL_DEMO, UBA_TR_DEMO, UBA_TR_DEMO, UBA_TR_DEMO, UBA_TR_DEMO, UBA_TR_DEMO, UBA_TR_DEMO,
UBA_TR_DEMO, UBA_TR_DEMO };

static UBA_TR tr_demo = UBA_TR_DEMO;
int UBA_TR_demo_init(void) {
	UBA_channel_init_g();
	UBA_6_device_g.BPT_A.ch = &UBA_CH_A;
	UBA_6_device_g.BPT_A.type = UBA_BPT_TYPE_SINGLE_CHANNEL;
	UBA_6_device_g.BPT_A.ewi = UBA_BPT_EWI_LEVEL_ERROR;
	UBA_6_device_g.BPT_B.ch = &UBA_CH_B;
	UBA_6_device_g.BPT_B.type = UBA_BPT_TYPE_SINGLE_CHANNEL;
	UBA_6_device_g.BPT_B.ewi = UBA_BPT_EWI_LEVEL_WARNING;
	UBA_6_device_g.BPT_AB.ch = &UBA_CH_AB;
	UBA_6_device_g.BPT_AB.type = UBA_BPT_TYPE_DUAL_CHANNEL;

	/*UBA_TR_unpack(&tr_demo, &(UBA_6_device_g.BPT_A));
	UBA_TR_unpack(&tr_demo, &(UBA_6_device_g.BPT_B));
	UBA_TR_unpack(&tr_demo, &(UBA_6_device_g.BPT_AB));*/
	return 0;
}

void UBA_BPT_free(UBA_BPT *bpt) {
	UBA_BPT_step *step_to_free;
	UART_LOG_INFO(UBA_COMP, "free steps");
	bpt->current_step = bpt->head_step;
	while (bpt->current_step != NULL) {
		step_to_free = bpt->current_step;
		bpt->current_step = step_to_free->next;
		free(step_to_free);
	}
	bpt->current_step = NULL;
	bpt->head_step = NULL;
}

void UBA_TR_reset_loop_counter(UBA_TR *tr, uint8_t until_index) {
	uint8_t index = 0;
	UART_LOG_DEBUG(UBA_COMP, "Reset loop counter %u/%u", until_index, tr->length);
	if (until_index > tr->length) {
		UART_LOG_ERROR(UBA_COMP, "counter index is higher then th TR length ");
		return;
	}
	for (index = 0; index < until_index; index++) {
		if (tr->config[index].type_id == TEST_ROUTINE_STEP_TYPE_LOOP) {
			tr->config[index].type.loop.loop_counter = 0;
		}
	}
}

int32_t UBA_TR_unpack(UBA_TR *tr, UBA_BPT *bpt) {
	int index = 0;
	UBA_BPT_step *new_step;
	UBA_BPT_step *last_step;
	bool isAdd_step = false;
	UART_LOG_INFO(UBA_COMP, "unpack TR into: %s", bpt->ch->name);
	UBA_BPT_free(bpt);
	UBA_TR_reset_loop_counter(tr, tr->length);
	bpt->state.next = UBA_BPT_STATE_INIT;
	bpt->state.current = UBA_BPT_STATE_INIT;
	memcpy(bpt->name, tr->name, sizeof(bpt->name));
	int step_index = 0;
	for (index = 0; index < tr->length;) {
		new_step = (UBA_BPT_step*) malloc(sizeof(UBA_BPT_step));
		if (new_step == NULL) { // if one of the malloc fail
			UART_LOG_CRITICAL(UBA_COMP, "malloc Failed");
			UBA_BPT_free(bpt);
			return -1;
		}
		new_step->next = NULL;
		switch (tr->config[index].type_id) {
			case TEST_ROUTINE_STEP_TYPE_CHARGE:
				new_step->type = UBA_BPT_STEP_TYPE_CHARGE;
				isAdd_step = true;
				index++;
				break;
			case TEST_ROUTINE_STEP_TYPE_DISCHARGE:
				new_step->type = UBA_BPT_STEP_TYPE_DISCHARGE;
				isAdd_step = true;
				index++;
				break;
			case TEST_ROUTINE_STEP_TYPE_DELAY:
				new_step->type = UBA_BPT_STEP_TYPE_DELAY;
				isAdd_step = true;
				index++;
				break;
			case TEST_ROUTINE_STEP_TYPE_LOOP:
				if (tr->config[index].type.loop.loop_counter < tr->config[index].type.loop.loop_size) {
					tr->config[index].type.loop.loop_counter++;
					index = tr->config[index].type.loop.loop_to_step;
				} else {
					index++;
					UBA_TR_reset_loop_counter(tr, index);
				}
				isAdd_step = false;
				break;
			default:
				UART_LOG_CRITICAL(UBA_COMP, "Type Id Is Unknoun");
				return 1;
		}
		if (isAdd_step) {
			new_step->step_index = step_index++;
			if (new_step->step_index == 0) {
				bpt->current_step = new_step;
				bpt->head_step = new_step;
				last_step = new_step;
			} else {
				last_step->next = new_step;
				last_step = new_step;
			}
		} else {
			free(new_step);
		}
		isAdd_step = false;
	}
	bpt->last_step_index = step_index - 1;

	return 0;
}
