/*
 * UBA_battery_performance_test.c
 *
 *  Created on: Sep 17, 2024
 *      Author: ORA
 */

#include "UBA_battery_performance_test.h"

#include "uart_log.h"
#include "stdio.h"
#include "UBA_util.h"
#include "rtc.h"

#define UBA_COMP "BPT"
#define DEMO 1
#define UBA_BPT_MAX_STEP_DEMO_TIME (3000)

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

typedef void (*step_cb_t)(UBA_BPT *screen);

/***
 * UBA BPT State Machine Assigner Rule
 */
struct UBABPTSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};

/*UBA BPT State Machine Assigner */
#define UBABPTSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

// @formatter:off
static const struct UBABPTSMA_rule rule_g[UBA_BPT_STATE_MAX] ={
		UBABPTSMA(UBA_BPT_STATE_INIT,			UBA_BPT_init_enter,				UBA_BPT_init,			UBA_BPT_init_exit),
		UBABPTSMA(UBA_BPT_STATE_STANDBY,		UBA_BPT_standby_enter,			UBA_BPT_standby,		UBA_BPT_standby_exit),
		UBABPTSMA(UBA_BPT_STATE_PAUSE,			UBA_BPT_pause_enter,			UBA_BPT_pause,			UBA_BPT_pause_exit),
		UBABPTSMA(UBA_BPT_STATE_RUN_STEP,		UBA_BPT_run_step_enter,			UBA_BPT_run_step,		UBA_BPT_run_step_exit),
		UBABPTSMA(UBA_BPT_STATE_STEP_COMPLEATE,	UBA_BPT_step_compleate_enter,	UBA_BPT_step_compleate,	UBA_BPT_step_compleate_exit),
		UBABPTSMA(UBA_BPT_STATE_TEST_FAILED,	UBA_BPT_failed_enter,			UBA_BPT_failed,			UBA_BPT_failed_exit),
		UBABPTSMA(UBA_BPT_STATE_TEST_COMPLEATE,	UBA_BPT_compleate_enter,		UBA_BPT_compleate,		UBA_BPT_compleate_exit),
};
// @formatter:on
//=================================================private functions========================================================//

void UBA_BPT_update_state(UBA_BPT *bpt) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", bpt->state.current, bpt->state.next);
	bpt->state.pre = bpt->state.current;
	bpt->state.current = bpt->state.next;
	bpt->state.next = UBA_BPT_STATE_INVALID;
}
bool UBA_BPT_isStep_completed(UBA_BPT *bpt) {
	//TODO: return true or false according to step type and RT data
#ifdef DEMO
	return (HAL_GetTick() - bpt->start_time) > UBA_BPT_MAX_STEP_DEMO_TIME;
#else
	return true;
#endif
}


//====================================================state machine functions============================================//
void UBA_BPT_init_enter(UBA_BPT *bpt) {
	bpt->current_step = bpt->head_step; //reset head
	bpt->ewi = UBA_BPT_EWI_OK;
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
	if(bpt->current_step == bpt->head_step){
		HAL_RTC_GetDate(&hrtc, &bpt->start_date_time.date, RTC_FORMAT_BIN);
		HAL_RTC_GetTime(&hrtc, &bpt->start_date_time.time, RTC_FORMAT_BIN);
	}
}

void UBA_BPT_pause_enter(UBA_BPT *bpt){
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
}

void UBA_BPT_pause(UBA_BPT *bpt){
	UBA_channel_run(bpt->ch);
}

void UBA_BPT_pause_exit(UBA_BPT *bpt){

}

void UBA_BPT_run_step_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	bpt->start_time = HAL_GetTick();
	if (bpt->current_step != NULL) {
		switch (bpt->current_step->type) {
			case UBA_BPT_STEP_TYPE_CHARGE:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_CHARGE);
				break;
			case UBA_BPT_STEP_TYPE_DISCHARGE:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DISCHARGE);
				break;
			case UBA_BPT_STEP_TYPE_DELAY:
				UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_DELAY);
				break;
			default:
				UART_LOG_ERROR(UBA_COMP, "Step Type id is unknown:%u", bpt->current_step->type);
		}
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "enter step while the pointer in null");
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
	}
}

void UBA_BPT_run_step(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
	if (UBA_BPT_isStep_completed(bpt)) {
		if (bpt->current_step->next == NULL) {
			UART_LOG_BPT_INFO(UBA_COMP, "next step pointer is null , test completed");
			bpt->state.next = UBA_BPT_STATE_TEST_COMPLEATE;
		} else {
			bpt->current_step = bpt->current_step->next;
			bpt->state.next = UBA_BPT_STATE_RUN_STEP; // reenter state
		}
	} else if (UBA_channel_isInCriticalError(bpt->ch)) {
		bpt->state.next = UBA_BPT_STATE_TEST_FAILED;
	}
}

void UBA_BPT_run_step_exit(UBA_BPT *bpt) {

}

void UBA_BPT_step_compleate_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
}
void UBA_BPT_step_compleate(UBA_BPT *bpt) {
}
void UBA_BPT_step_compleate_exit(UBA_BPT *bpt) {
}
void UBA_BPT_failed_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	bpt->ewi |=UBA_BPT_EWI_TEST_FAIL;
}
void UBA_BPT_failed(UBA_BPT *bpt) {
}
void UBA_BPT_failed_exit(UBA_BPT *bpt) {
}
void UBA_BPT_compleate_enter(UBA_BPT *bpt) {
	UBA_BPT_update_state(bpt);
	UBA_channel_set_next_state(bpt->ch, UBA_CHANNEL_STATE_STANDBY);
	bpt->ewi = UBA_BPT_EWI_COMPLETED;
}
void UBA_BPT_compleate(UBA_BPT *bpt) {
	UBA_channel_run(bpt->ch);
}
void UBA_BPT_compleate_exit(UBA_BPT *bpt) {
}
//=================================================public  functions========================================================//

bool UBA_BPT_isRunning(UBA_BPT *bpt) {
	if (bpt != NULL) {
		UART_LOG_BPT_DEBUG(UBA_COMP, "is running %s",
				((bpt->state.current == UBA_BPT_STATE_RUN_STEP) || (bpt->state.current == UBA_BPT_STATE_STEP_COMPLEATE)) ? "Yes" : "No");
		return (((bpt->state.current == UBA_BPT_STATE_RUN_STEP) || (bpt->state.current == UBA_BPT_STATE_STEP_COMPLEATE)));
	} else {
		return false;
	}
}

bool UBA_BPT_isPause(UBA_BPT *bpt){
	if (bpt != NULL) {
			return (((bpt->state.current == UBA_BPT_STATE_PAUSE)));
		} else {
			return false;
		}
}

bool UBA_BPT_stop(UBA_BPT *bpt) {
	if (bpt != NULL) {
		bpt->state.next = UBA_BPT_STATE_STANDBY;
		bpt->current_step = bpt->head_step;
		return true;
	} else {
		return false;
	}
}

bool UBA_BPT_start(UBA_BPT *bpt) {
	if (bpt != NULL) {
		if(bpt->state.current ==  UBA_BPT_STATE_TEST_COMPLEATE){
			bpt->state.next = UBA_BPT_STATE_INIT;
		}else{
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
		bpt->state.next = UBA_BPT_STATE_PAUSE;
		return true;
	} else {
		return false;
	}
}
bool UBA_BPT_isUnpacked(UBA_BPT *bpt){
	if (bpt != NULL) {
			return (bpt->head_step!=NULL);
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

UBA_BPT_EWI UBA_BPT_pair(UBA_BPT *bpt, UBA_channel *ch, int list_index) {
	UBA_BPT_EWI ret = UBA_BPT_EWI_OK;

	return ret;
}
