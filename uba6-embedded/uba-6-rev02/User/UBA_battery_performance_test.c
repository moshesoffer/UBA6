/*
 * UBA_battery_performance_test.c
 *
 *  Created on: Sep 17, 2024
 *      Author: ORA
 */
#undef UART_LOG_DISABLE

#include "UBA_battery_performance_test.h"
#include "uart_log.h"

#include "pb.h"
#include "pb_encode.h"
#include "UBA_PROTO_helper.h"

#include "stdio.h"
#include "stdlib.h"
#include "uart_log.h"
#include "UBA_util.h"
#include "rtc.h"
#include "UBA_6.h"
#include "UBA_buzzer.h"
#include "UBA_test_routine.h"
#include "UBA_PROTO_DATA_LOG.pb.h"
#include "UBA_file_manager.h"
#include "UBA_UART_comm.h"
#include "LCD.h"
#include "UBA_LCD_screen.h"

extern void peripheralsInit();

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

#define MAX_CONSECUTIVE_ERRORS 5

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
void UBA_BPT_step_complete_enter(UBA_BPT *bpt);
void UBA_BPT_step_complete(UBA_BPT *bpt);
void UBA_BPT_step_complete_exit(UBA_BPT *bpt);
void UBA_BPT_failed_enter(UBA_BPT *bpt);
void UBA_BPT_failed(UBA_BPT *bpt);
void UBA_BPT_failed_exit(UBA_BPT *bpt);
void UBA_BPT_complete_enter(UBA_BPT *bpt);
void UBA_BPT_complete(UBA_BPT *bpt);
void UBA_BPT_complete_exit(UBA_BPT *bpt);

bool UBA_BPT_save_data_log(UBA_BPT *bpt);
bool UBA_BPT_isChannelState(UBA_channel *ch, UBA_CHANNEL_STATE state);

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
		UBABPTSMA(STEP_COMPLETE,	UBA_BPT_step_complete_enter,	UBA_BPT_step_complete,	UBA_BPT_step_complete_exit),
		UBABPTSMA(TEST_FAILED,		UBA_BPT_failed_enter,			UBA_BPT_failed,			UBA_BPT_failed_exit),
		UBABPTSMA(TEST_COMPLETE,	UBA_BPT_complete_enter,			UBA_BPT_complete,		UBA_BPT_complete_exit),
};
// @formatter:on
#define UBA_CHANNEL_CRITICAL_ERROR (UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR |UBA_PROTO_UBA6_ERROR_CHANNEL_EMPTY)

//=================================================private functions========================================================//

bool UBA_BPT_isChannel_error_critical(UBA_BPT *bpt) {
	return (bpt->ch->error & UBA_CHANNEL_CRITICAL_ERROR) > 0;

}

void UBA_BPT_test_result_filename(UBA_BPT *bpt) {
	RTC_DateTypeDef date;
	RTC_TimeTypeDef time;

	char datetime_str[16];
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	int n = snprintf((char*) bpt->filename, sizeof(bpt->filename), "%s_%s_%s.pb",
			(char*) bpt->ch->name,
			(char*) get_rtc_date_time_str((char*) datetime_str, sizeof(datetime_str), &(date), &(time),
					UBA_RTC_STR_FORMAT_FILE),
			(char*) ((TR_Test_Routine *)bpt->tr)->name);
	if (n >= UBA_BPT_FILENAME_MAX_SIZE) {
		UART_LOG_ERROR(UBA_COMP, "Buffer length is to short(%lu,%d)", UBA_BPT_FILENAME_MAX_SIZE, n);
	}
	UART_LOG_INFO(UBA_COMP, "update BPT file name to %s ", bpt->filename);
}

void UBA_BPT_update_state(UBA_BPT *bpt) {
	if ((bpt->state.current < UBA_BPT_STATE_MAX) && (bpt->state.current < UBA_BPT_STATE_MAX)) {
		UART_LOG(UBA_COMP, "(bpt)update state %s ---> %s", rule_g[bpt->state.current].name, rule_g[bpt->state.next].name);
	} else {
		UART_LOG(UBA_COMP, "(bpt)update state %u ---> %u", bpt->state.current, bpt->state.next);
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
		strcpy (bpt->complete_reason, "Reach Max Time");

		UART_LOG_BPT_INFO("=======Step reach timeout of :%lu[S] > (%lu)[S]========", timeout_sec, action_time);
	}
	return ret;
}

bool UBA_BPT_isStop_condition_met_charge_current(UBA_BPT *bpt) {
	bool ret = false;
	//Moshe
	UBA_LCD_screen *current_screen = (UBA_LCD_screen *)bpt->ch->current_screen;
	ret = (((int32_t) (UBA_channel_get_current(bpt->ch)) < bpt->current_step->type.charge.stop_condition.cut_off_current)
			&& (((int32_t)UBA_channel_get_voltage(bpt->ch)) >= (bpt->current_step->type.charge.voltage * ((TR_Test_Routine *)bpt->tr)->battery.num_cells_in_serial)));
	if (ret) {
		strcpy (bpt->complete_reason, "Cut-Off Current");

		UART_LOG_BPT_INFO("!!!!Cut off current has met: %05lu mA < %05lu mA ; %u mV >= %05lu mV; numCells %d ch-name %s TR-name %s %s", 
				UBA_channel_get_current(bpt->ch),
				bpt->current_step->type.charge.stop_condition.cut_off_current, 
				UBA_channel_get_voltage(bpt->ch),
				bpt->current_step->type.charge.voltage,
				((TR_Test_Routine *)bpt->tr)->battery.num_cells_in_serial,
				bpt->ch->name,
				((TR_Test_Routine *)bpt->tr)->name, current_screen->tr->name
			);
	}
	return ret;
}
bool UBA_BPT_isStop_condition_met_charge_capacity(UBA_BPT *bpt) {
	bool ret = false;
	ret = (UBA_channel_get_capacity(bpt->ch) > bpt->current_step->type.charge.stop_condition.charge_limit);
	if (ret) {
		strcpy (bpt->complete_reason, "Reach Charge Limit");

		UART_LOG_BPT_INFO("!!!!!Charge capacity has met: %05f <%05lu ", 
				UBA_channel_get_capacity(bpt->ch),
				bpt->current_step->type.charge.stop_condition.charge_limit);
	}
	return ret;
}
bool UBA_BPT_isStop_condition_met_charge_temp(UBA_BPT *bpt) {
	bool ret = false;
	ret = (UBA_channel_get_temperature(bpt->ch) > bpt->current_step->type.charge.stop_condition.max_emperature);
	if (ret) {
		strcpy (bpt->complete_reason, "Reach Temp Limit");

		UART_LOG_BPT_INFO("!!!!!Temp Stop condition has met: %05f > %05f ", 
				UBA_channel_get_temperature(bpt->ch),
				bpt->current_step->type.charge.stop_condition.max_emperature);
	}
	return ret;
}

bool UBA_BPT_isStep_completed(UBA_BPT *bpt) {
	bool isCompleted = false;
	if (bpt->current_step != NULL) {
		if (bpt->force_step_stop == true) {
			isCompleted = true;
		}

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
				int32_t voltage = UBA_channel_get_voltage(bpt->ch);
				if ((voltage > 0) &&
					(voltage < bpt->current_step->type.discharge.stop_condition.cut_off_voltage)) {
					UART_LOG_WARNNING(UBA_COMP, "Reach Cut of Voltage : %u < %d", UBA_channel_get_voltage(bpt->ch),
							bpt->current_step->type.discharge.stop_condition.cut_off_voltage);
					strcpy (bpt->complete_reason, "Cut-Off Voltage");
					isCompleted = true;
				}

				if ((abs((int) UBA_channel_get_capacity(bpt->ch)) > ((int) bpt->current_step->type.discharge.stop_condition.charge_limit))) {
					UART_LOG_WARNNING(UBA_COMP, "Cut-off Capacity");
					strcpy (bpt->complete_reason, "Cut-Off Capacity");
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
				strcpy (bpt->complete_reason, "invalid Step Id");
				isCompleted |= true;
		}

	}
	return isCompleted;
}

//====================================================state machine functions============================================//
void UBA_BPT_init_enter(UBA_BPT *bpt) {
	bpt->current_step = bpt->head_step; //reset head
	bpt->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	bpt->ch->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	memset(bpt->filename, 0, UBA_BPT_FILENAME_MAX_SIZE);
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_INIT);
	bpt->start_date_time.update_pause_seconds = true;
	bpt->ch->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;

	//init cache status message
	UBA_BPT_init_cached_status_msg(bpt);

	//select TR file
	UBA_BPT_STATE state = bpt->state.next;

//already done: see UBA_6_init_enter	
//	bpt->TR_selected_index = 0;
//	UBA_TR_unpack(&TR_file.list[bpt->TR_selected_index], bpt); // load the test roution

	bpt->state.next = state;

	bpt->wr_from = 0;
}

void UBA_BPT_init(UBA_BPT *bpt) {
	bpt->state.next = UBA_BPT_STATE_STANDBY;

	if ((bpt->state.current < UBA_BPT_STATE_MAX) && (bpt->state.current < UBA_BPT_STATE_MAX)) {
		UART_LOG_BPT_INFO(UBA_COMP, "(bpt)update state %s ---> %s", rule_g[bpt->state.current].name, rule_g[bpt->state.next].name);
	} else {
		UART_LOG_BPT_INFO(UBA_COMP, "(bpt)update state %u ---> %u", bpt->state.current, bpt->state.next);
	}
}

void UBA_BPT_init_exit(UBA_BPT *bpt) {

}

void UBA_BPT_standby_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_INIT);

	bpt->complete_reason[0] = '\0';
	UBA_LCD_screen *current_screen = (UBA_LCD_screen *)bpt->ch->current_screen;
	memset(current_screen->pages.screen_bpt.EWI_msg.elemnt.text.text, ' ', UBA_GFX_TEXT_MAX_LENGTH-8);
}

void UBA_BPT_standby(UBA_BPT *bpt) {
	//cache status message
	UBA_BPT_set_cached_status_msg(bpt, /*start test=*/false);
	
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_standby_exit(UBA_BPT *bpt) {
	if (bpt->current_step == bpt->head_step) {
		UBA_BPT_test_result_filename(bpt);
	}
}

void UBA_BPT_pause_enter(UBA_BPT *bpt) {	
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);

	UBA_6_fan_on(&UBA_6_device_g, false);
}

void UBA_BPT_pause(UBA_BPT *bpt) {
	//cache status message
	UBA_BPT_set_cached_status_msg(bpt, /*start test=*/false);

	UBA_channel_run(bpt->ch);

	uint32_t curr_tick_ms = HAL_GetTick(); 
	if (curr_tick_ms - bpt->log_tick_ms > bpt->log_intreval) {
//UART_LOG(UBA_COMP, "run step: tick %d, ms %d delta %d, interval %d", curr_tick_ms, bpt->log_tick_ms, curr_tick_ms - bpt->log_tick_ms, bpt->log_intreval);
		UBA_BPT_save_data_log(bpt);
		bpt->log_tick_ms = curr_tick_ms;
	}
}

void UBA_BPT_pause_exit(UBA_BPT *bpt) {
	if (bpt->state.next != UBA_BPT_STATE_STANDBY) {
		switch (bpt->current_step->type_id) {
			case UBA_BPT_STEP_TYPE_CHARGE:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_CHARGE);
				UBA_6_fan_on(&UBA_6_device_g, true);
				break;
			case UBA_BPT_STEP_TYPE_DISCHARGE:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DISCHARGE);
				UBA_6_fan_on(&UBA_6_device_g, true);
				break;
			case UBA_BPT_STEP_TYPE_DELAY:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DELAY);
				UBA_6_fan_on(&UBA_6_device_g, true);
				break;
			default:
				UART_LOG_ERROR(UBA_COMP, "Step Type id is unknown:%u", bpt->current_step->type_id);
		}
	}
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
				UART_LOG_BPT_INFO("Start Step ==> Target current:%06d\tTarget Voltage:%06d", bpt->current_step->type.charge.current,
						bpt->current_step->type.charge.voltage);
				UBA_channel_set_discharge_param(bpt->ch, (&(bpt->current_step->type.discharge.current)));
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DISCHARGE);
				break;

			case UBA_BPT_STEP_TYPE_DELAY:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DELAY);
				break;

			default:
				UART_LOG_ERROR(UBA_COMP, "Step Type id is unknown:%u", bpt->current_step->type_id);
		}
		bpt->force_step_stop = false;

//		if (bpt->current_step == bpt->head_step) {
//			HAL_RTC_GetDate(&hrtc, &bpt->start_date_time.date, RTC_FORMAT_BIN);
//			HAL_RTC_GetTime(&hrtc, &bpt->start_date_time.time, RTC_FORMAT_BIN);
//		}

	} else {
		UART_LOG_CRITICAL(UBA_COMP, "enter step while the pointer in null");
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
	}

	//init cache status message
	UBA_BPT_init_cached_status_msg(bpt);

	//init stablization error mechanism
	bpt->ch->num_consecutive_errors = 0;

}

void UBA_BPT_run_step(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);

	uint32_t curr_tick_ms = HAL_GetTick(); 
	if (curr_tick_ms - bpt->log_tick_ms > bpt->log_intreval) {
//UART_LOG(UBA_COMP, "run step: tick %d, ms %d delta %d, interval %d", curr_tick_ms, bpt->log_tick_ms, curr_tick_ms - bpt->log_tick_ms, bpt->log_intreval);
		UBA_BPT_save_data_log(bpt);
		bpt->log_tick_ms = curr_tick_ms;
	}

	if (UBA_BPT_isStep_completed(bpt)) {
		bpt->state.next = UBA_BPT_STATE_STEP_COMPLETE;
		bpt->ch->num_consecutive_errors  = 0;

	} else if (UBA_BPT_isChannel_error_critical(bpt)) {
		bpt->ch->num_consecutive_errors += 1;
		if (bpt->ch->num_consecutive_errors >= MAX_CONSECUTIVE_ERRORS) {
			UART_LOG_CRITICAL(UBA_COMP, "run step not completed, critical error %d on id %d %s", bpt->ch->error, bpt->ch->id, bpt->ch->name);
			bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
		}

		if (bpt->wr_from > 0) {
UART_LOG(UBA_COMP, "run step err exit: bpt->wr_from %d [%s]", bpt->wr_from, bpt->filename);
			UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, bpt->buffer, (uint32_t) bpt->wr_from); 
			bpt->wr_from = 0;
		}

	} else if (UBA_channel_are_lines_connected(bpt->ch) == false) {
		UART_LOG_CRITICAL(UBA_COMP, "run step not completed, lines disconnected on id %d %s", bpt->ch->id, bpt->ch->name);
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;

		if (bpt->wr_from > 0) {
UART_LOG(UBA_COMP, "run step disconnected exit: bpt->wr_from %d [%s]", bpt->wr_from, bpt->filename);
			UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, bpt->buffer, (uint32_t) bpt->wr_from); 
			bpt->wr_from = 0;
		}
	}

	//cache status message
	UBA_BPT_set_cached_status_msg(bpt, /*start test=*/false);
}

void UBA_BPT_run_step_exit(UBA_BPT *bpt) {
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_step_complete_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_channel_run(bpt->ch);
	bpt->current_step->timing.step_completed = HAL_GetTick();

	UBA_6_fan_on(&UBA_6_device_g, false);
}

void UBA_BPT_step_complete(UBA_BPT *bpt) {
	if (UBA_BPT_isChannelState(bpt->ch, UBA_CHANNEL_STATE_STANDBY)) {
		if (bpt->current_step->next == NULL) {
			UART_LOG_BPT_INFO("next step pointer is null , test completed");
			bpt->state.next = UBA_BPT_STATE_TEST_COMPLETE;

		} else {
			bpt->current_step = bpt->current_step->next;
			bpt->state.next = UBA_BPT_STATE_RUN_STEP; // reenter state
		} 
		
		//cache status message
		UBA_BPT_set_cached_status_msg(bpt, /*start test=*/false);
		
		UBA_LCD_screen_update(bpt->ch->current_screen);

	} 
}

void UBA_BPT_step_complete_exit(UBA_BPT *bpt) {
}

void UBA_BPT_failed_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	bpt->error = bpt->ch->error; // get the channel error that failed the experiment
	//TODO: get Errors
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_ERROR);
}

void UBA_BPT_failed(UBA_BPT *bpt) {
	//UBA_channel_run(bpt->ch);
	UBA_BPT_stop(bpt); // stop the test
}

void UBA_BPT_failed_exit(UBA_BPT *bpt) {
	bpt->current_step = bpt->head_step;
}

void UBA_BPT_complete_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_COMPLETE);
}

void UBA_BPT_complete(UBA_BPT *bpt) {
	//cache status message
	UBA_BPT_set_cached_status_msg(bpt, /*start test=*/false);

	UBA_channel_run(bpt->ch);
}

void UBA_BPT_complete_exit(UBA_BPT *bpt) {
	bpt->complete_reason[0] = '\0';
}

//=================================================public  functions========================================================//

bool UBA_BPT_isChannelState(UBA_channel *ch, UBA_CHANNEL_STATE state) {
	if (ch->id == UBA_CHANNLE_ID_AB) {
		UBA_channel *cha, *chb;
		cha = &UBA_CH_A;
		chb = &UBA_CH_B;

		if ((cha->state.current == state) && (chb->state.current == state)) {
			return true;
		}

	} else {
		if (ch->state.current == state) {
			return true;
		}
	}
	return false;
}

bool UBA_BPT_isRunning(UBA_BPT *bpt) { 
	if (bpt != NULL) {
		/*
		 * UART_LOG_BPT_DEBUG("%s :is running?: %s",bpt->tr->name,
		 ((bpt->state.current == UBA_BPT_STATE_PAUSE) ||(bpt->state.current == UBA_BPT_STATE_RUN_STEP) || (bpt->state.current == UBA_BPT_STATE_STEP_COMPLETE)) ? "Yes" : "No");
		 */
		return ((bpt->state.current == UBA_BPT_STATE_PAUSE) || 
				(bpt->state.current == UBA_BPT_STATE_RUN_STEP) ||
				(bpt->state.current == UBA_BPT_STATE_STEP_COMPLETE));

	} else {
		return false;
	}
}

bool UBA_BPT_isChannelRunning(UBA_channel *ch) {
	if (ch != NULL) {
		return ((ch->state.current == UBA_BPT_STATE_PAUSE) || 
				(ch->state.current == UBA_BPT_STATE_RUN_STEP) ||
				(ch->state.current == UBA_BPT_STATE_STEP_COMPLETE));
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
			bpt->error |= UBA_PROTO_UBA6_ERROR_USER_ABORT;
		} 
		bpt->state.next = UBA_BPT_STATE_STANDBY;//UBA_BPT_STATE_STEP_COMPLETE
		bpt->current_step = bpt->head_step;

		UBA_6_fan_on(&UBA_6_device_g, false);

//		if (bpt->wr_from > 0) {
//UART_LOG(UBA_COMP, "last step complete exit: bpt->wr_from %d [%s]", bpt->wr_from, bpt->filename);
//			UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, bpt->buffer, (uint32_t) bpt->wr_from); 
//			bpt->wr_from = 0;
//		}
		//UBA_BPT_test_result_filename(bpt);

		int list_index = bpt->TR_selected_index;
		if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			//if (bpt->ch->id == UBA_CHANNLE_ID_A) {
				UBA_LCD_g.screen_ch_A.shadow.ch_control = UBA_LCD_g.screen_ch_A.ch_control;
				UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_A;
				UBA_LCD_g.screen_ch_A.bpt->ch->state.current = UBA_CHANNEL_STATE_STANDBY;

			//} else if (bpt->ch->id == UBA_CHANNLE_ID_B) {
				UBA_LCD_g.screen_ch_B.shadow.ch_control = UBA_LCD_g.screen_ch_B.ch_control;
				UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_B;
				UBA_LCD_g.screen_ch_B.bpt->ch->state.current = UBA_CHANNEL_STATE_STANDBY;
			//}

			if (bpt->ch->id == UBA_CHANNLE_ID_B) {
				//shadow
//				UBA_LCD_g.screen_ch_B.main_buttons.btn_up_p = UBA_LCD_g.screen_ch_A.shadow.main_buttons.btn_up_p;
//				UBA_LCD_g.screen_ch_B.main_buttons.btn_down_p = UBA_LCD_g.screen_ch_A.shadow.main_buttons.btn_down_p;
//				UBA_LCD_g.screen_ch_B.main_buttons.btn_select_p = UBA_LCD_g.screen_ch_A.shadow.main_buttons.btn_select_p;
//
//				UBA_LCD_g.screen_ch_B.secondery_buttons.btn_up_p = UBA_LCD_g.screen_ch_A.shadow.secondery_buttons.btn_up_p;
//				UBA_LCD_g.screen_ch_B.secondery_buttons.btn_down_p = UBA_LCD_g.screen_ch_A.shadow.secondery_buttons.btn_down_p;
//				UBA_LCD_g.screen_ch_B.secondery_buttons.btn_select_p = UBA_LCD_g.screen_ch_A.shadow.secondery_buttons.btn_select_p;
//				
//				memcpy (&UBA_LCD_g.screen_ch_B.pages.screen_bpt.btn_back_stop, &UBA_LCD_g.screen_ch_A.shadow.btn_back_stop, sizeof (UBA_GFX));
//				memcpy (&UBA_LCD_g.screen_ch_B.pages.screen_bpt.btn_pause_start, &UBA_LCD_g.screen_ch_A.shadow.btn_pause_start, sizeof (UBA_GFX));
//				memcpy (&UBA_LCD_g.screen_ch_B.pages.screen_bpt.btn_next, &UBA_LCD_g.screen_ch_A.shadow.btn_next, sizeof (UBA_GFX));
			}
		}
		return true;

	} else {
		return false;
	}
}

bool UBA_BPT_next(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if (UBA_BPT_isRunning(bpt)) {
			bpt->force_step_stop = true;
		} 
		return true;

	} else {
		return false;
	}
}

bool UBA_BPT_start(UBA_BPT *bpt) {
	if (bpt != NULL) {
		//verify battery (lines) are connected
		if (UBA_channel_are_lines_connected (bpt->ch) == false) {
			bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
			return false;
		}

//peripheralsInit();
		if (bpt->state.current == UBA_BPT_STATE_TEST_COMPLETE) {
			bpt->state.next = UBA_BPT_STATE_INIT;

		} else if (bpt->state.current == UBA_BPT_STATE_TEST_FAILED) {
			bpt->state.current = UBA_BPT_STATE_RUN_STEP;
			bpt->state.next = UBA_BPT_STATE_RUN_STEP;
			bpt->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;

		} else {
			bpt->state.next = UBA_BPT_STATE_RUN_STEP;
		}

		int list_index = bpt->TR_selected_index;
		if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			//if (bpt->ch->id == UBA_CHANNLE_ID_A) {
				UBA_LCD_g.screen_ch_A.shadow.ch_control = UBA_LCD_g.screen_ch_A.ch_control;
				UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_AB;

			//} else if (bpt->ch->id == UBA_CHANNLE_ID_B) {
				UBA_LCD_g.screen_ch_B.shadow.ch_control = UBA_LCD_g.screen_ch_B.ch_control;
				UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_AB;

			//}

		} 
		bpt->error &= ~UBA_PROTO_UBA6_ERROR_USER_ABORT;

		UBA_6_fan_on(&UBA_6_device_g, true);

		return true;

	} else {
		return false;
	}
}

bool UBA_BPT_load(UBA_BPT *bpt) {
	int list_index = bpt->TR_selected_index;
	if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
		if (bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
			//return true;
		}
	}

	if (bpt != NULL) {
		bpt->state.next = UBA_BPT_STATE_INIT;
		return true;
	} else {
		return false;
	}
}

bool UBA_BPT_pause_test(UBA_BPT *bpt) {
	if (bpt != NULL) {
		//avoid channel B when bpt is running TR with mode = UBA_PROTO_BPT_MODE_DUAL_CHANNEL
		int list_index = bpt->TR_selected_index;
		if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			if (bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
				//return true;
			}
		}
	
		if (UBA_BPT_isRunning(bpt)) {
			//if (bpt->ch->current_screen != NULL) {
			//	UBA_LCD_screen_event(bpt->ch->current_screen, UBA_LCD_SCREEN_DISPLAY_EXE_CMD, UBA_LCD_SCREEN_DISPLAY_EVENT_PAUSE);
			//	bpt->start_date_time.update_pause_seconds = true;
			bpt->state.next = UBA_BPT_STATE_PAUSE;
			bpt->start_date_time.update_pause_seconds = true;
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
			} /*else {
				UART_LOG_CRITICAL(UBA_COMP, "current step index is OOB", bpt->state.next);
			}*/
			
			if (bpt->state.next < UBA_BPT_STATE_MAX) {
				if (rule_g[bpt->state.next].enter) {
					rule_g[bpt->state.next].enter(bpt); // run the next state enter function
				}
			} /*else {
				UART_LOG_CRITICAL(UBA_COMP, "next step index %d (0f MAX %d) is OOB", bpt->state.next, UBA_BPT_STATE_MAX);
			}*/
		}
	}
}

UBA_PROTO_UBA6_ERROR UBA_BPT_pair(UBA_BPT *bpt, UBA_channel *ch, int list_index) {
	UBA_PROTO_UBA6_ERROR ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;

	return ret;
}

UBA_STATUS_CODE UBA_BPT_begin(UBA_BPT *bpt, uint8_t list_index) {
	UART_LOG_BPT_INFO("BPT Start index:%u", list_index);
	//avoid channel B when bpt is running TR with mode = UBA_PROTO_BPT_MODE_DUAL_CHANNEL
	if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
		if (bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
			//return true;
		}
	}

	if (UBA_BPT_isPause(bpt)) {
		if (bpt->ch->current_screen != NULL) {
			UBA_LCD_screen_event(bpt->ch->current_screen, UBA_LCD_SCREEN_DISPLAY_EXE_CMD, UBA_LCD_SCREEN_DISPLAY_EVENT_SELECT);
		}
		return UBA_BPT_start(bpt); // resume the test
	}

	if (UBA_BPT_isRunning(bpt)) {
		UART_LOG_ERROR(UBA_COMP, "Test is already running , channel is busy");
		return UBA_STATUS_CODE_BUSY;

	} else {
		if (list_index < UBA_TR_LIST_SIZE) {
			//set by web-console
			UBA_TR_unpack(&TR_file.list[list_index], bpt); // load the test roution
			bpt->TR_selected_index = list_index;

			if (bpt->ch->current_screen != NULL) {
				UBA_LCD_screen_event(bpt->ch->current_screen, UBA_LCD_SCREEN_DISPLAY_EXE_CMD, UBA_LCD_SCREEN_DISPLAY_EVENT_SELECT);

				((UBA_LCD_screen*)bpt->ch->current_screen)->tr = bpt->tr;
				((UBA_LCD_screen*)bpt->ch->current_screen)->tr_list_select_index = bpt->TR_selected_index;
			}

			HAL_RTC_GetDate(&hrtc, &bpt->start_date_time.date, RTC_FORMAT_BIN);
			HAL_RTC_GetTime(&hrtc, &bpt->start_date_time.time, RTC_FORMAT_BIN);

			bpt->start_date_time.add_pause_seconds = 0;
			return UBA_BPT_start(bpt); // start the test
			
		} else {
			UART_LOG_ERROR(UBA_COMP, "the selected test index existed the length of the list");
			return UBA_STATUS_CODE_PARMETER;
		}
	}
}

UBA_STATUS_CODE UBA_BPT_next_step(UBA_BPT *bpt) {
	if (bpt != NULL) {
		//avoid channel B when bpt is running TR with mode = UBA_PROTO_BPT_MODE_DUAL_CHANNEL
		int list_index = bpt->TR_selected_index;
		if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			if (bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
				//return true;
			}
		}

		if (bpt->ch->current_screen != NULL) {
			UBA_BPT_next(bpt); // run test next-step

			if (!UBA_LCD_screen_isLastStep (bpt->ch->current_screen)) {
				UBA_LCD_screen_event(bpt->ch->current_screen, UBA_LCD_SCREEN_DISPLAY_EXE_CMD, UBA_LCD_SCREEN_DISPLAY_EVENT_STEP);
			}
			return true;
		}
	}
	UART_LOG_ERROR(UBA_COMP, "can't run next step");
	return false;
}

UBA_STATUS_CODE UBA_BPT_end(UBA_BPT *bpt) {
	if (bpt != NULL) {
		//avoid channel B when bpt is running TR with mode = UBA_PROTO_BPT_MODE_DUAL_CHANNEL
		int list_index = bpt->TR_selected_index;
		if (TR_file.list[list_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			if (bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
				//return true;
			}
		}
	
		UBA_BPT_stop(bpt); // stop the test

		if (bpt->ch->current_screen != NULL) {
			UBA_LCD_screen_event(bpt->ch->current_screen, UBA_LCD_SCREEN_DISPLAY_EXE_CMD, UBA_LCD_SCREEN_DISPLAY_EVENT_STOP);
		}

		return true;
	} else {
		return false;
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
		case UBA_PROTO_BPT_CMD_ID_STEP:
			UBA_BPT_next_step(bpt);
			break;
		case UBA_PROTO_BPT_CMD_ID_STOP:
			UBA_BPT_end(bpt);
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
	RTC_TimeTypeDef time;

	msg->error = UBA_channel_get_lines_errors(bpt->ch);
	if (((UBA_LCD_screen*)bpt->ch->current_screen)->ch_control == UBA_CHANNLE_ID_AB) {
    	msg->state = (&UBA_6_device_g.BPT_A)->state.current;
	} else {
		msg->state = bpt->state.current;
	}

	//update run time
	UBA_LCD_screen_getRunTime(bpt, &time);
	//UART_LOG_INFO("====> (msgUpdate) ch: %s, time: %d:%d:%d", bpt->ch->name, time.Hours, time.Minutes, time.Seconds);
	//msg->start_time = RTC_datetime2unix_timestamp(&bpt->start_date_time.date, &time);
	msg->start_time = time.Seconds;

	//update step
	if (((UBA_LCD_screen*)bpt->ch->current_screen)->ch_control == UBA_CHANNLE_ID_AB) {
    	msg->current_step = (&UBA_6_device_g.BPT_A)->current_step->step_index;
    	msg->total_steps = (&UBA_6_device_g.BPT_A)->current_step->plan_index;
	} else {
    	msg->current_step = bpt->current_step->step_index;
    	msg->total_steps = bpt->current_step->plan_index;
	}
	
	TR_Test_Routine *tr = &TR_file.list[((UBA_LCD_screen*)bpt->ch->current_screen)->tr_list_select_index];
	TR_config_step *tr_step = &tr->config[bpt->current_step->step_index];
   	msg->step_type = (tr_step->type_id == UBA_PROTO_BPT_STEP_TYPE_CHARGE) ? UBA_PROTO_BPT_STEP_TYPE_CHARGE :
					 (tr_step->type_id == UBA_PROTO_BPT_STEP_TYPE_DISCHARGE) ? UBA_PROTO_BPT_STEP_TYPE_DISCHARGE : 
					 (tr_step->type_id == UBA_PROTO_BPT_STEP_TYPE_DELAY) ? UBA_PROTO_BPT_STEP_TYPE_DELAY :  UBA_PROTO_BPT_STEP_TYPE_INVALID;
	UART_LOG_BPT_INFO(UBA_COMP, "==> (msgUpdate) ch %s, ch_control %d, tr-name %s, tr-index %d, step: %d %d of %d, step_type %d", 
		bpt->ch->name, ((UBA_LCD_screen*)bpt->ch->current_screen)->ch_control, tr->name, ((UBA_LCD_screen*)bpt->ch->current_screen)->tr_list_select_index, bpt->current_step->step_index,
		msg->current_step, msg->total_steps, msg->step_type);

	if (((UBA_LCD_screen*)bpt->ch->current_screen)->ch_control == UBA_CHANNLE_ID_AB) {
		UBA_channel_update_message((&UBA_6_device_g.BPT_A)->ch, &msg->channel_status);
	} else {
		UBA_channel_update_message(bpt->ch, &msg->channel_status);
	}
}

bool UBA_BPT_save_data_log(UBA_BPT *bpt) {
	//uint8_t buffer[UBA_PROTO_DATA_LOG_data_log_size + 1];
	size_t message_size = 0;
	size_t index;
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
	//if (message_size > sizeof(buffer)) {
	if (message_size > (WR_BUFFER_LEN - bpt->wr_from)) {
		UART_LOG_ERROR(UBA_COMP, "Message Size:%u is to Big to the buffer:%u", message_size, (WR_BUFFER_LEN - bpt->wr_from));
		return false;
	}

	index = UBA_PROTO_helper_encode_varint(message_size, &bpt->buffer[bpt->wr_from]);
	//pb_ostream_t stream = pb_ostream_from_buffer(&buffer[index], sizeof(buffer) - index);
	pb_ostream_t stream = pb_ostream_from_buffer(&bpt->buffer[bpt->wr_from + index], (WR_BUFFER_LEN - bpt->wr_from) - index);
	bpt->wr_from += message_size+index;
	status = pb_encode(&stream, UBA_PROTO_DATA_LOG_data_log_fields, &msg);
	if (!status) {
		UART_LOG_BPT_DEBUG("Encoding failed: %s", PB_GET_ERROR(&stream));
		return false;
	}

	if (bpt->wr_from >= WR_BUFFER_LEN) {
UART_LOG(UBA_COMP, "save log: length %d [%s]", bpt->wr_from, bpt->filename);
		UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, bpt->buffer, (uint32_t) bpt->wr_from); 
		bpt->wr_from = 0;
	}
	return true;
}

void UBA_BPT_get_tr_filename(UBA_BPT *bpt, char *tr_filename) {
	sprintf(tr_filename, "%s", ((TR_Test_Routine *) bpt->tr)->name);
}


void UBA_BPT_init_cached_status_msg(UBA_BPT *bpt) {
	UART_LOG_BPT_INFO(UBA_COMP, "UBA_BPT_INIT_cached_status_msg");
	memset (&bpt->start_status_msg, 0, sizeof (UBA_PROTO_BPT_status_message));
	memset (&bpt->next_status_msg, 0, sizeof (UBA_PROTO_BPT_status_message));
	bpt->set_start_msg = true;
	bpt->get_start_msg = true;
}

void UBA_BPT_set_cached_status_msg(UBA_BPT *bpt, bool start) {
	if (start == true) {
 		UBA_BPT_update_message(bpt, &bpt->start_status_msg);
		UART_LOG_BPT_INFO(UBA_COMP, "UBA_BPT_SET_cached_status_msg-0, time: %d", bpt->start_status_msg.start_time);
		bpt->set_start_msg = false;
		bpt->get_start_msg = true;
	}
	else {
		UBA_BPT_update_message(bpt, &bpt->next_status_msg);
		UART_LOG_BPT_INFO(UBA_COMP, "UBA_BPT_SET_cached_status_msg-1, time: %d", bpt->next_status_msg.start_time);
	}
}

void UBA_BPT_get_cached_status_msg(UBA_BPT *bpt, UBA_PROTO_BPT_status_message *msg) {
	if (bpt->get_start_msg == true) {
		//UART_LOG_BPT_INFO(UBA_COMP, "UBA_BPT_GET_cached_status_msg-0, time: %d", bpt->start_status_msg.start_time);
 		memcpy (msg, &bpt->start_status_msg, sizeof (UBA_PROTO_BPT_status_message));
		bpt->get_start_msg = false;
	}
	else {
		//UART_LOG_BPT_INFO(UBA_COMP, "UBA_BPT_GET_cached_status_msg-1, time: %d", bpt->next_status_msg.start_time);
 		memcpy (msg, &bpt->next_status_msg, sizeof (UBA_PROTO_BPT_status_message));
	}
	//UART_LOG_BPT_INFO(UBA_COMP, "==> (msgGet) time: %d, step: %d of %d, step_type %d", msg->start_time, msg->current_step, msg->total_steps, msg->step_type);
}

void UBA_BPT_save_log(UBA_BPT *bpt) {	
#define LOG_BUF_SIZE 4096

	if (UBA_BPT_isRunning(bpt) == false) {
		uint32_t chunk_length = LOG_BUF_SIZE;

		if (bpt->wr_from > 0) {
			chunk_length = bpt->wr_from > chunk_length ? chunk_length : bpt->wr_from;
UART_LOG_BPT_INFO(UBA_COMP, "==> last step complete exit: bpt->wr_from %d [%s]", chunk_length, bpt->filename);
			UBA_FM_apppned_data(UBA_FM_FOLDER_TEST_RESULTS, (char*) bpt->filename, bpt->buffer, chunk_length); 
			bpt->wr_from -= chunk_length;
		}
	}
}
