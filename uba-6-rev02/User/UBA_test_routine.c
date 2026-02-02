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
#include "UBA_PROTO_TR.pb.h"
#include "UBA_file_manager.h"
#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "UBA_PROTO_helper.h"

#define UBA_COMP "TEST ROUTINE"

#if (UBA_LOG_LEVEL_TR <= UART_LOG_LEVEL_INFO)
	#define LOG_COMP_INFO(...) UART_LOG_INFO(UBA_COMP,##__VA_ARGS__)
#else
	#define LOG_COMP_INFO(...)
#endif

#if UBA_LOG_LEVEL_TR <= UART_LOG_LEVEL_DEBUG
	#define LOG_COMP_DEBUG(...)  UART_LOG_DEBUG(UBA_COMP,##__VA_ARGS__)
#else
	#define LOG_COMP_DEBUG(...)
#endif



TR_Test_Routine_File TR_file = { .list = { UBA_TR_DEMO_DISCHRAGE, UBA_TR_DEMO_CHRAGE, UBA_TR_DEMO_CHRAGE_L, UBA_TR_DEMO, UBA_TR_DUAL_DEMO,
UBA_TR_DEMO, UBA_TR_DEMO, UBA_TR_DEMO,
UBA_TR_DEMO, UBA_TR_DEMO } };

int UBA_TR_demo_init(void) {
	UBA_channel_init_g();
	UBA_6_device_g.BPT_A.ch = &UBA_CH_A;
	UBA_6_device_g.BPT_A.type = UBA_BPT_TYPE_SINGLE_CHANNEL;
	UBA_6_device_g.BPT_A.error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	UBA_6_device_g.BPT_B.ch = &UBA_CH_B;
	UBA_6_device_g.BPT_B.type = UBA_BPT_TYPE_SINGLE_CHANNEL;
	UBA_6_device_g.BPT_B.error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	UBA_6_device_g.BPT_AB.ch = &UBA_CH_AB;
	UBA_6_device_g.BPT_AB.type = UBA_BPT_TYPE_DUAL_CHANNEL;
	if (UBA_PROTO_load_from_file(UBA_FM_FOLDER_TEST_ROUTINE, UBA_FM_FILE_NAME_TEST_ROUTINE, TR_Test_Routine_File_fields, &TR_file) == false) {
		UBA_PROTO_save_to_file(UBA_FM_FOLDER_TEST_ROUTINE, UBA_FM_FILE_NAME_TEST_ROUTINE, TR_Test_Routine_File_fields, &TR_file);
	}
	for (int index = 0; index < 10; index++) {
		UBA_TR_print(&TR_file.list[index]);
	}
	UBA_TR_unpack(&TR_file.list[1], &(UBA_6_device_g.BPT_A));
	UBA_TR_unpack(&TR_file.list[2], &(UBA_6_device_g.BPT_B));
	return 0;
}

void UBA_BPT_free(UBA_BPT *bpt) {
	UBA_BPT_step *step_to_free;
	LOG_COMP_INFO("free steps");
	bpt->current_step = bpt->head_step;
	while (bpt->current_step != NULL) {
		step_to_free = bpt->current_step;
		bpt->current_step = step_to_free->next;
		free(step_to_free);
	}
	bpt->current_step = NULL;
	bpt->head_step = NULL;
}

void UBA_TR_reset_loop_counter(TR_Test_Routine *tr, uint8_t until_index) {
	uint8_t index = 0;
	UART_LOG_DEBUG(UBA_COMP, "Reset loop counter %u/%u", until_index, tr->length);
	if (until_index > tr->length) {
		UART_LOG_ERROR(UBA_COMP, "counter index is higher then th TR length ");
		return;
	}
	for (index = 0; index < until_index; index++) {
		if (tr->config[index].type_id == TR_STEP_TYPE_STEP_TYPE_LOOP) {
			tr->config[index].type.loop.loop_counter = 0;
		}
	}
}

bool UBA_TR_set_step_and_validate(UBA_BPT_step *step, TR_config_step *step_config) {
	bool ret = false;
	if (step != NULL) {
		if (((uint8_t) step_config->type_id) < UBA_BPT_STEP_TYPE_MAX) {
			step->type_id = step_config->type_id;
			switch (step->type_id) {
				case UBA_BPT_STEP_TYPE_CHARGE:
					memcpy(&step->type.charge, &step_config->type.charge, sizeof(UBA_BPT_charge));
					ret = true;
					break;
				case UBA_BPT_STEP_TYPE_DISCHARGE:
					memcpy(&step->type.discharge, &step_config->type.discharge, sizeof(UBA_BPT_discharge));
					ret = true;
					break;
				case UBA_BPT_STEP_TYPE_DELAY:
					memcpy(&step->type.delay, &step_config->type.delay, sizeof(UBA_BPT_delay));
					if ((step->type.delay.delay_time == 0) && (step->type.delay.cool_down_emperature > 60)) {
						ret = false;
					} else {
						ret = true;
					}
					break;
				default:
					UART_LOG_CRITICAL(UBA_COMP, " step id %x is not valid", step->type_id);
			}
		}
	}
	return ret;
}
/**
 * @brief unpack test routine to a battery performance test
 *
 * @param tr
 * @param bpt
 * @return
 */
int32_t UBA_TR_unpack(TR_Test_Routine *tr, UBA_BPT *bpt) {
	int index = 0;
	UBA_BPT_step *new_step;
	UBA_BPT_step *last_step;
	bool isAdd_step = false;
	UART_LOG_INFO(UBA_COMP, "unpack TR into: %s", bpt->ch->name);
	UBA_BPT_free(bpt);
	UBA_TR_reset_loop_counter(tr, tr->length);
	bpt->state.next = UBA_BPT_STATE_INIT; // TODO: Set next state only if the BPF is not running
	bpt->log_intreval = (tr->log_interval == 0) ? 1000 : tr->log_interval;
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
				case TEST_ROUTINE_STEP_TYPE_DISCHARGE:
				case TEST_ROUTINE_STEP_TYPE_DELAY:
				if (UBA_TR_set_step_and_validate(new_step, &tr->config[index])) {
					new_step->plan_index = index;
					isAdd_step = true;
					index++;
				} else {
					UART_LOG_CRITICAL(UBA_COMP, "Step is not valid");
					UBA_BPT_free(bpt);
					return -1;
				}
				//memcpy(bpt->type, tr->type, sizeof(bpt->type));
				//TODO: copy stop param
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

void UBA_TR_print_step_config(TR_config_step *step) {
	LOG_COMP_DEBUG(UBA_COMP, "Step Type ID: %d Type: %u", step->type_id, step->which_type);
	switch (step->which_type) {
		case TR_config_step_charge_tag:
			LOG_COMP_DEBUG( "Charge - Voltage: %d, Current: %d Min Temp :%f",
					step->type.charge.voltage,
					step->type.charge.current,
					step->type.charge.min_temperature);
			LOG_COMP_DEBUG( "Cut off Current %d\tlimit Cap: %d\t Max Temp: %f, Max Time:%u",
					step->type.charge.sc.cut_off_current,
					step->type.charge.sc.limit_capacity,
					step->type.charge.sc.max_temperature,
					step->type.charge.sc.max_time
					);
			break;
		case TR_config_step_discharge_tag:
			LOG_COMP_DEBUG( "Discharge - Current: Type:0x%0X Value:%d",
					step->type.discharge.current.type, step->type.discharge.current.value);
			break;
		case TR_config_step_delay_tag:
			LOG_COMP_DEBUG( "Delay - Time (s): %d",
					step->type.delay.delay_time);
			break;
		case TR_config_step_loop_tag:
			LOG_COMP_DEBUG( "Loop - Size: %d",
					step->type.loop.loop_size);
			break;
		default:
			LOG_COMP_DEBUG( "Unknown type");
			break;
	}
}

void UBA_TR_print(TR_Test_Routine *tr) {
	LOG_COMP_INFO("Test Routine : %s Mode : %u - %s Length:%u", tr->name,tr->mode,
			tr->mode == UBA_PROTO_BPT_MODE_SINGLE_CHANNEL ? "BPT_MODE_SINGLE_CHANNEL" : "BPT_MODE_DUAL_CHANNEL", tr->length);
	for (uint8_t index = 0; index < tr->length; index++) {
		LOG_COMP_DEBUG("TR Step Config at index %u", index);
		UBA_TR_print_step_config(&tr->config[index]);
	}
}
