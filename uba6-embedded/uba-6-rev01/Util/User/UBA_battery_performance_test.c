/*
 * UBA_battery_performance_test.c
 *
 *  Created on: Sep 17, 2024
 *      Author: ORA
 */

#include "UBA_battery_performance_test.h"

#include "pb.h"
#include "pb_encode.h"
#include "UBA_PROTO_helper.h"

#include "stdio.h"
#include "stdlib.h"
#include "uart_log.h"
#include "UBA_util.h"
#include "rtc.h"
#include "UBA_buzzer.h"
#include "UBA_test_routine.h"
#include "UBA_PROTO_DATA_LOG.pb.h"
#include "UBA_file_manager.h"
#include "UBA_UART_comm.h"

#define UBA_COMP "BPT"

#if (UBA_LOG_LEVEL_BPT <= UART_LOG_LEVEL_INFO)
#define UART_LOG_BPT_INFO(...) UART_LOG_INFO(UBA_COMP,##__VA_ARGS__)
#else
#define UART_LOG_BPT_INFO(...)
#endif

#if UBA_LOG_LEVEL_BPT <= UART_LOG_LEVEL_DEBUG
#define UART_LOG_BPT_DEBUG(...)  UART_LOG_DEBUG(UBA_COMP ,##__VA_ARGS__)
#else
#define UART_LOG_BPT_DEBUG(...)
#endif

bool UBA_BPT_isStep_completed(UBA_BPT *bpt);

void UBA_BPT_init_enter(UBA_BPT *bpt);
void UBA_BPT_init(UBA_BPT *bpt);
void UBA_BPT_init_exit(UBA_BPT *bpt);
void UBA_BPT_standby_enter(UBA_BPT *bpt);
void UBA_BPT_standby(UBA_BPT *bpt);
void UBA_BPT_standby_exit(UBA_BPT *bpt);
void UBA_BPT_pause_enter(UBA_BPT *bpt);
void UBA_BPT_pause(UBA_BPT *bpt);
void UBA_BPT_pause_exit(UBA_BPT *bpt);
void UBA_BPT_run_step_enter(UBA_BPT *bpt);
void UBA_BPT_run_step(UBA_BPT *bpt);
void UBA_BPT_run_step_exit(UBA_BPT *bpt);
void UBA_BPT_step_compleate_enter(UBA_BPT *bpt);
void UBA_BPT_step_compleate(UBA_BPT *bpt);
void UBA_BPT_step_compleate_exit(UBA_BPT *bpt);
void UBA_BPT_failed_enter(UBA_BPT *bpt);
void UBA_BPT_failed(UBA_BPT *bpt);
void UBA_BPT_failed_exit(UBA_BPT *bpt);
void UBA_BPT_compleate_enter(UBA_BPT *bpt);
void UBA_BPT_compleate(UBA_BPT *bpt);
void UBA_BPT_compleate_exit(UBA_BPT *bpt);

bool UBA_BPT_save_data_log(UBA_BPT *bpt);

typedef void (*step_cb_t)(UBA_BPT *screen);

/***
 * UBA BPT State Machine Assigner Rule
 */
struct UBABPTSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
	char *name;
};

/*UBA BPT State Machine Assigner */
#define UBABPTSMA(step, cbe, cbr, cbx)[UBA_BPT_STATE_##step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx, .name = #step}

// @formatter:off
static const struct UBABPTSMA_rule rule_g[UBA_BPT_STATE_MAX] ={
		UBABPTSMA(INIT,				UBA_BPT_init_enter,				UBA_BPT_init,			UBA_BPT_init_exit),
		UBABPTSMA(STANDBY,			UBA_BPT_standby_enter,			UBA_BPT_standby,		UBA_BPT_standby_exit),
		UBABPTSMA(PAUSE,			UBA_BPT_pause_enter,			UBA_BPT_pause,			UBA_BPT_pause_exit),
		UBABPTSMA(RUN_STEP,			UBA_BPT_run_step_enter,			UBA_BPT_run_step,		UBA_BPT_run_step_exit),
		UBABPTSMA(STEP_COMPLEATE,	UBA_BPT_step_compleate_enter,	UBA_BPT_step_compleate,	UBA_BPT_step_compleate_exit),
		UBABPTSMA(TEST_FAILED,		UBA_BPT_failed_enter,			UBA_BPT_failed,			UBA_BPT_failed_exit),
		UBABPTSMA(TEST_COMPLEATE,	UBA_BPT_compleate_enter,		UBA_BPT_compleate,		UBA_BPT_compleate_exit),
};
// @formatter:on
#define UBA_CHANNEL_CRITICAL_ERROR (UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR |UBA_PROTO_UBA6_ERROR_CHANNEL_EMPTY)

//=================================================private functions========================================================//

bool UBA_BPT_isChannel_error_critical(UBA_BPT *bpt) {
	return (bpt->ch->error & UBA_CHANNEL_CRITICAL_ERROR) > 0;

}

void UBA_BPT_test_result_filename(UBA_BPT *bpt) {
	char datetime_str[16];
	int n = snprintf((char*) bpt->filename, sizeof(bpt->filename), "%s_%s_%s.pb",
			(char*) bpt->ch->name,
			(char*) get_rtc_date_time_str((char*) datetime_str, sizeof(datetime_str), &(bpt->start_date_time.date), &(bpt->start_date_time.time),
					UBA_RTC_STR_FORMAT_FILE),
			(char*) bpt->name);
	if (n >= UBA_BPT_FILENAME_MAX_SIZE) {
		UART_LOG_ERROR(UBA_COMP, "Buffer length is to short(%lu,%d)", UBA_BPT_FILENAME_MAX_SIZE, n);
	}
	UART_LOG_INFO(UBA_COMP, "update BPT file name to %s ", bpt->filename);

}

void UBA_BPT_update_state(UBA_BPT *bpt) {
	if ((bpt->state.current < UBA_BPT_STATE_MAX) && (bpt->state.current < UBA_BPT_STATE_MAX)) {
		UART_LOG_INFO(UBA_COMP, "update state %s ---> %s", rule_g[bpt->state.current].name, rule_g[bpt->state.next].name);
	} else {
		UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", bpt->state.current, bpt->state.next);
	}
	bpt->state.pre = bpt->state.current;
	bpt->state.current = bpt->state.next;
	bpt->state.next = UBA_BPT_STATE_INVALID;
}

uint32_t UBA_BPT_step_action_time(UBA_BPT *bpt) {
	uint32_t current_time_tic = HAL_GetTick();
	uint32_t step_start_time_tick = bpt->current_step->timing.step_action_start;
	uint32_t ret =0;
	if (bpt != NULL) {
		if (step_start_time_tick > 0) {
			ret = ((current_time_tic - step_start_time_tick) );
		} else {
			switch (bpt->current_step->type_id) {
				case UBA_BPT_STEP_TYPE_CHARGE:
					if (UBA_channel_isCharging(bpt->ch)) {
						bpt->current_step->timing.step_action_start = current_time_tic;
					}
					break;
				case UBA_BPT_STEP_TYPE_DISCHARGE:
					if (UBA_channel_isDischarging(bpt->ch)) {
						bpt->current_step->timing.step_action_start = current_time_tic;
					}
					break;
				case UBA_BPT_STEP_TYPE_DELAY:
					bpt->current_step->timing.step_action_start = current_time_tic;
					break;
				default:
					UART_LOG_ERROR(UBA_COMP, "Action State (%u) is not supported", bpt->current_step->type_id);
					break;
			}
		}
	}else{
		UART_LOG_ERROR(UBA_COMP, "BPT is NULL");
	}
	return ret;
}

bool UBA_BPT_isStep_timeout(UBA_BPT *bpt, uint32_t timeout_sec) {
	uint32_t action_time = UBA_BPT_step_action_time(bpt) /1000;
	bool ret = (action_time > timeout_sec);
	if (ret) {
		UART_LOG_BPT_INFO("=======Step reach timeout of :%lu[S] > (%lu)[S]========", timeout_sec, action_time);
	}
	return ret;
}

bool UBA_BPT_isStop_condition_met_charge_current(UBA_BPT *bpt) {
	bool ret = false;
	ret = (((int32_t) (UBA_channel_get_charge_current(bpt->ch)) < bpt->current_step->type.charge.stop_condition.cut_off_current)
			&& (((int32_t)UBA_channel_get_voltage(bpt->ch)) >= bpt->current_step->type.charge.voltage));
	if (ret) {
		UART_LOG_BPT_INFO("!!!!Cut off current has met: %05lu mA < %05lu mA  At %umV [GenV]", UBA_channel_get_charge_current(bpt->ch),
				bpt->current_step->type.charge.stop_condition.cut_off_current, UBA_channel_get_voltage(bpt->ch));
	}
	return ret;
}
bool UBA_BPT_isStop_condition_met_charge_capacity(UBA_BPT *bpt) {
	bool ret = false;
	ret = (UBA_channel_get_capacity(bpt->ch) > bpt->current_step->type.charge.stop_condition.limit_capacity);
	if (ret) {
		UART_LOG_BPT_INFO("!!!!!Charge capacity has met: %05f <%05lu ", UBA_channel_get_capacity(bpt->ch),
				bpt->current_step->type.charge.stop_condition.limit_capacity);
	}
	return ret;
}
bool UBA_BPT_isStop_condition_met_charge_temp(UBA_BPT *bpt) {
	bool ret = false;
	ret = (UBA_channel_get_temperature(bpt->ch) > bpt->current_step->type.charge.stop_condition.max_emperature);
	if (ret) {
		UART_LOG_BPT_INFO("!!!!!Temp Stop condition has met: %05f > %05f ", UBA_channel_get_temperature(bpt->ch),
				bpt->current_step->type.charge.stop_condition.max_emperature);
	}
	return ret;
}

bool UBA_BPT_isStep_completed(UBA_BPT *bpt) {
	bool isCompleted = false;
	if (bpt->current_step != NULL) {
		switch (bpt->current_step->type_id) {
			case UBA_BPT_STEP_TYPE_CHARGE:
				if (UBA_channel_isCharging(bpt->ch)) {
					isCompleted |= UBA_BPT_isStop_condition_met_charge_current(bpt);
					isCompleted |= UBA_BPT_isStop_condition_met_charge_capacity(bpt);
					isCompleted |= UBA_BPT_isStop_condition_met_charge_temp(bpt);
					isCompleted |= UBA_BPT_isStep_timeout(bpt, bpt->current_step->type.charge.stop_condition.max_time);
				}
				break;
			case UBA_BPT_STEP_TYPE_DISCHARGE:
				if (((int32_t) UBA_channel_get_voltage(bpt->ch)) < bpt->current_step->type.discharge.stop_condition.cut_off_voltage) {
					UART_LOG_WARNNING(UBA_COMP, "Reach Cut of Voltage : %u < %d", UBA_channel_get_voltage(bpt->ch),
							bpt->current_step->type.discharge.stop_condition.cut_off_voltage);
					isCompleted = true;
				}
				if ((abs((int) UBA_channel_get_capacity(bpt->ch)) > ((int) bpt->current_step->type.discharge.stop_condition.limit_capacity))) {
					UART_LOG_WARNNING(UBA_COMP, "Reach cut of discharge capacity");
					isCompleted = true;
				}

				if (UBA_BPT_isStep_timeout(bpt, bpt->current_step->type.discharge.stop_condition.max_time)) {
					UART_LOG_WARNNING(UBA_COMP, "Reach Max Time");
					isCompleted = true;
				}
				break;
			case UBA_BPT_STEP_TYPE_DELAY:
				isCompleted |= (UBA_channel_get_temperature(bpt->ch) < bpt->current_step->type.delay.cool_down_emperature);
				isCompleted |= UBA_BPT_isStep_timeout(bpt, bpt->current_step->type.delay.delay_time);
				break;
			default:
				UART_LOG_CRITICAL(UBA_COMP, "invalid step id 0x%x", bpt->current_step->type_id);
				isCompleted |= true;
		}

	}
	return isCompleted;
}

//====================================================state machine functions============================================//
void UBA_BPT_init_enter(UBA_BPT *bpt) {
	bpt->current_step = bpt->head_step; //reset head
	bpt->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	memset(bpt->filename, 0, UBA_BPT_FILENAME_MAX_SIZE);
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_INIT);
}
void UBA_BPT_init(UBA_BPT *bpt) {
	bpt->state.next = UBA_BPT_STATE_STANDBY;

}
void UBA_BPT_init_exit(UBA_BPT *bpt) {

}

void UBA_BPT_standby_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_INIT);

}
void UBA_BPT_standby(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_standby_exit(UBA_BPT *bpt) {
	if (bpt->current_step == bpt->head_step) {
		HAL_RTC_GetDate(&hrtc, &bpt->start_date_time.date, RTC_FORMAT_BIN);
		HAL_RTC_GetTime(&hrtc, &bpt->start_date_time.time, RTC_FORMAT_BIN);
	}
	UBA_BPT_test_result_filename(bpt);

}

void UBA_BPT_pause_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
}

void UBA_BPT_pause(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_pause_exit(UBA_BPT *bpt) {

}

void UBA_BPT_run_step_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	bpt->log_tick_ms = HAL_GetTick();
	if (bpt->current_step != NULL) {
		bpt->current_step->timing.step_start = HAL_GetTick();
		bpt->current_step->timing.step_action_start = 0;
		UBA_channel_reset_capacity(bpt->ch);
		switch (bpt->current_step->type_id) {
			case UBA_BPT_STEP_TYPE_CHARGE:
				UART_LOG_BPT_INFO("Start Step ==> Target current:%06d\tTarget Voltage:%06d", bpt->current_step->type.charge.current,
						bpt->current_step->type.charge.voltage);
				UBA_channel_set_charge_param(bpt->ch, bpt->current_step->type.charge.current, bpt->current_step->type.charge.voltage);
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_CHARGE);
				break;
			case UBA_BPT_STEP_TYPE_DISCHARGE:
				UBA_channel_set_discharge_param(bpt->ch, (&(bpt->current_step->type.discharge.current)));
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DISCHARGE);
				break;
			case UBA_BPT_STEP_TYPE_DELAY:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DELAY);
				break;
			default:
				UART_LOG_ERROR(UBA_COMP, "Step Type id is unknown:%u", bpt->current_step->type_id);
		}
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "enter step while the pointer in null");
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
	}
}

void UBA_BPT_run_step(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
	if (HAL_GetTick() - bpt->log_tick_ms > bpt->log_intreval) {
		UBA_BPT_save_data_log(bpt);
	}
	if (UBA_BPT_isStep_completed(bpt)) {
		bpt->state.next = UBA_BPT_STATE_STEP_COMPLEATE;
	} else if (UBA_BPT_isChannel_error_critical(bpt)) {
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
	}

}

void UBA_BPT_run_step_exit(UBA_BPT *bpt) {
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_step_compleate_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_channel_run(bpt->ch);
	bpt->current_step->timing.step_completed = HAL_GetTick();
}
void UBA_BPT_step_compleate(UBA_BPT *bpt) {
//	UBA_channel_run(bpt->ch);
	if (bpt->ch->state.current == UBA_CHANNEL_STATE_STANDBY) {
		if (bpt->current_step->next == NULL) {
			UART_LOG_BPT_INFO("next step pointer is null , test completed");
			bpt->state.next = UBA_BPT_STATE_TEST_COMPLEATE;
		} else {
			bpt->current_step = bpt->current_step->next;
			bpt->state.next = UBA_BPT_STATE_RUN_STEP; // reenter state
		}
	}
}

void UBA_BPT_step_compleate_exit(UBA_BPT *bpt) {
}

void UBA_BPT_failed_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	bpt->error = bpt->ch->error; // get the channel error that failed the experiment
	//TODO: get Errors
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_ERROR);
}

void UBA_BPT_failed(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
}
void UBA_BPT_failed_exit(UBA_BPT *bpt) {
	bpt->current_step = bpt->head_step;
}
void UBA_BPT_compleate_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_COMPLEATE);

}
void UBA_BPT_compleate(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
}
void UBA_BPT_compleate_exit(UBA_BPT *bpt) {
}
//=================================================public  functions========================================================//

bool UBA_BPT_isRunning(UBA_BPT *bpt) {
	if (bpt != NULL) {
		/*
		 * UART_LOG_BPT_DEBUG("%s :is running?: %s",bpt->name,
		 ((bpt->state.current == UBA_BPT_STATE_PAUSE) ||(bpt->state.current == UBA_BPT_STATE_RUN_STEP) || (bpt->state.current == UBA_BPT_STATE_STEP_COMPLEATE)) ? "Yes" : "No");
		 */
		return ((bpt->state.current == UBA_BPT_STATE_PAUSE) || (bpt->state.current == UBA_BPT_STATE_RUN_STEP)
				|| (bpt->state.current == UBA_BPT_STATE_STEP_COMPLEATE));
	} else {
		return false;
	}
}

bool UBA_BPT_isPause(UBA_BPT *bpt) {
	if (bpt != NULL) {
		return (((bpt->state.current == UBA_BPT_STATE_PAUSE)));
	} else {
		return false;
	}
}

bool UBA_BPT_stop(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if (UBA_BPT_isRunning(bpt)) {
			bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
			bpt->error |= UBA_PROTO_UBA6_ERROR_USER_ABORT;
		} else {
			bpt->state.next = UBA_BPT_STATE_STANDBY;
			bpt->current_step = bpt->head_step;
		}
		return true;
	} else {
		return false;
	}
}

bool UBA_BPT_start(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if (bpt->state.current == UBA_BPT_STATE_TEST_COMPLEATE) {
			bpt->state.next = UBA_BPT_STATE_INIT;
		} else if (bpt->state.current == UBA_BPT_STATE_TEST_FAILED) {
			bpt->state.next = UBA_BPT_STATE_RUN_STEP;
			bpt->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
		} else {
			bpt->state.next = UBA_BPT_STATE_RUN_STEP;
		}
		return true;
	} else {
		return false;
	}
}
bool UBA_BPT_load(UBA_BPT *bpt) {
	if (bpt != NULL) {
		bpt->state.next = UBA_BPT_STATE_INIT;
		return true;
	} else {
		return false;
	}
}

bool UBA_BPT_pause_test(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if (UBA_BPT_isRunning(bpt)) {
			bpt->state.next = UBA_BPT_STATE_PAUSE;
			return true;
		}
	}
	return false;
}
bool UBA_BPT_isUnpacked(UBA_BPT *bpt) {
	if (bpt != NULL) {
		return (bpt->head_step != NULL);
	} else {
		return false;
	}
}

void UBA_BPT_run(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if (bpt->state.next == UBA_BPT_STATE_INVALID) { // if there the next state is not define , then run this state function
			if (rule_g[bpt->state.current].run) {
				rule_g[bpt->state.current].run(bpt); // run the main function of the state
			}
		} else {
			if (bpt->state.current < UBA_BPT_STATE_MAX) {
				if (rule_g[bpt->state.current].exit) {
					rule_g[bpt->state.current].exit(bpt); // run the status exit function
				}
			} else {
				UART_LOG_CRITICAL(UBA_COMP, "current step index is OOB", bpt->state.next);
			}
			if (bpt->state.next < UBA_BPT_STATE_MAX) {
				if (rule_g[bpt->state.next].enter) {
					rule_g[bpt->state.next].enter(bpt); // run the next state enter function
				}
			} else {
				UART_LOG_CRITICAL(UBA_COMP, "next step index is OOB", bpt->state.next);
			}
		}
	}
}

UBA_PROTO_UBA6_ERROR UBA_BPT_pair(UBA_BPT *bpt, UBA_channel *ch, int list_index) {
	UBA_PROTO_UBA6_ERROR ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;

	return ret;
}

UBA_STATUS_CODE UBA_BPT_begin(UBA_BPT *bpt, uint8_t list_index) {
	UART_LOG_BPT_INFO("BPT Start index:%u", list_index);
	if (UBA_BPT_isPause(bpt)) {
		return UBA_BPT_start(bpt);
	}
	if (UBA_BPT_isRunning(bpt)) {
		UART_LOG_ERROR(UBA_COMP, "Test is already running , channel is busy");
		return UBA_STATUS_CODE_BUSY;
	} else {
		if (list_index < UBA_TR_LIST_SIZE) {
			UBA_TR_unpack(&TR_file.list[list_index], bpt); // load the test roution
			bpt->TR_selected_index = list_index;
			return UBA_BPT_start(bpt); // start the test
		} else {
			UART_LOG_ERROR(UBA_COMP, "the selected test index existed the length of the list");
			return UBA_STATUS_CODE_PARMETER;
		}

	}
}

void UBA_BPT_command_execute(UBA_BPT *bpt, UBA_PROTO_BPT_command *cmd) {
	UART_LOG_BPT_INFO("Execute Command");
	switch (cmd->id) {
		case UBA_PROTO_BPT_CMD_ID_TEST:
			UART_LOG_BPT_DEBUG("Command Execute Test");
			break;
		case UBA_PROTO_BPT_CMD_ID_SELECT:
			UBA_BPT_begin(bpt, cmd->BPT_list_entery);
			break;
		case UBA_PROTO_BPT_CMD_ID_STOP:
			UBA_BPT_stop(bpt);
			break;
		case UBA_PROTO_BPT_CMD_ID_PAUSED:
			UBA_BPT_pause_test(bpt);
			break;
		case UBA_PROTO_BPT_CMD_ID_CLEAR:
			UBA_BPT_load(bpt);
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "The command ID is unknown:%u", cmd->id);
			break;
	}
}

void UBA_BPT_update_message(UBA_BPT *bpt, UBA_PROTO_BPT_status_message *msg) {
	msg->error = UBA_channel_get_lines_errors(bpt->ch);
	msg->state = bpt->state.current;

	msg->start_time = RTC_datetime2unix_timestamp(&bpt->start_date_time.date, &bpt->start_date_time.time);

	if (bpt->current_step != NULL) {
		msg->current_step = bpt->current_step->step_index;
		msg->step_type = bpt->current_step->type_id;
		msg->total_steps = bpt->last_step_index + 1;
	}
	UBA_channel_update_message(bpt->ch, &msg->channel_status);
}

bool UBA_BPT_save_data_log(UBA_BPT *bpt) {
	uint8_t buffer[UBA_PROTO_DATA_LOG_data_log_size + 1];
	size_t message_size = 0, index;
	bool status;
	UBA_PROTO_DATA_LOG_data_log msg = UBA_PROTO_DATA_LOG_data_log_init_zero;

	msg.time = get_RTC_unix_timestamp() - RTC_datetime2unix_timestamp(&bpt->start_date_time.date, &bpt->start_date_time.time); // store only the time from start of the test
	msg.step_index = bpt->current_step->step_index;
	msg.plan_index = bpt->current_step->plan_index;
	msg.current = UBA_channel_get_current(bpt->ch);
	msg.voltage = UBA_channel_get_voltage(bpt->ch);
	msg.temp = (int16_t) (UBA_channel_get_temperature(bpt->ch) * 100);
	print_data_log(&msg);
	status = pb_get_encoded_size(&message_size, UBA_PROTO_DATA_LOG_data_log_fields, &msg);
	if (status) {
		UART_LOG_BPT_DEBUG("Encoded size: %u bytes", message_size);
	} else {
		UART_LOG_ERROR(UBA_COMP, "Error getting encoded size");
		return false;
	}
	if (message_size > sizeof(buffer)) {
		UART_LOG_ERROR(UBA_COMP, "Message Size:%u is to Big to the buffer:%u", message_size, sizeof(buffer));
		return false;
	}

	index = UBA_PROTO_helper_encode_varint(message_size, buffer);
	pb_ostream_t stream = pb_ostream_from_buffer(&buffer[index], sizeof(buffer) - index);
	status = pb_encode(&stream, UBA_PROTO_DATA_LOG_data_log_fields, &msg);
	if (!status) {
		UART_LOG_BPT_DEBUG("Encoding failed: %s", PB_GET_ERROR(&stream));
		return false;
	}
	UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, buffer, (uint32_t) (message_size + index));
	bpt->log_tick_ms = HAL_GetTick();
	return true;
}

