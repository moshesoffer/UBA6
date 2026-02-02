/*
 * UBA_battery_performance_test.h
 *
 *  Created on: Sep 17, 2024
 *      Author: ORA
 */

#ifndef UBA_BATTERY_PERFORMANCE_TEST_H_
#define UBA_BATTERY_PERFORMANCE_TEST_H_

#include "stdbool.h"
#include "stdint.h"
#include "UBA_channel.h"
#include "UBA_common_def.h"

#define UBA_BPT_STMP_MAX_TIME (0xffffffff)
#define UBA_BPT_NAME_MAX_SIZE (32)

typedef enum UBA_BPT_STEP_TYPE {
	UBA_BPT_STEP_TYPE_CHARGE,
	UBA_BPT_STEP_TYPE_DISCHARGE,
	UBA_BPT_STEP_TYPE_DELAY,
	UBA_BPT_STEP_TYPE_MAX,
	UBA_BPT_STEP_TYPE_INVALID

} UBA_BPT_STEP_TYPE;

typedef struct UBA_BPT_step {
	uint8_t step_index;
	UBA_BPT_STEP_TYPE type;
	struct UBA_BPT_step *next;
	struct {
		uint32_t step_start;
		uint32_t step_completed;
		uint32_t step_max_time;
	} timing;
} UBA_BPT_step;

#define UBA_BPT_STEP_DEFUALT	\
{								\
	0,							\
	UBA_BPT_STEP_TYPE_CHARGE,	\
	NULL,						\
	{0,0,UBA_BPT_STMP_MAX_TIME}	\
}

typedef enum UBA_BPT_TYPE {
	UBA_BPT_TYPE_SINGLE_CHANNEL,
	UBA_BPT_TYPE_DUAL_CHANNEL,
	UBA_BPT_TYPE_MAX,
	UBA_BPT_TYPE_INVALED

} UBA_BPT_TYPE;

typedef enum UBA_BPT_EWI {
	UBA_BPT_EWI_OK = 0x0000,
	UBA_BPT_EWI_LEVEL_INFO = UBA_INFO,
	UBA_BPT_EWI_COMPLETED = UBA_BPT_EWI_LEVEL_INFO|0x0001,
	UBA_BPT_EWI_LEVEL_WARNING = UBA_WARNING,
	UBA_BPT_EWI_LEVEL_ERROR = UBA_ERROR,
	UBA_BPT_EWI_LEVEL_CRITICAL = UBA_CRITICAL | UBA_BPT_EWI_LEVEL_ERROR,
	UBA_BPT_EWI_USER_ABORT = UBA_ERROR | 0x0002,
	UBA_BPT_EWI_TEST_FAIL = UBA_CRITICAL | 0x0004,
	UBA_BPT_EWI_TEST_PAUSED = UBA_WARNING| 0x0008,



} UBA_BPT_EWI;

typedef enum UBA_BPT_STATE {
	UBA_BPT_STATE_INIT,
	UBA_BPT_STATE_STANDBY,
	UBA_BPT_STATE_PAUSE,
	UBA_BPT_STATE_RUN_STEP,
	UBA_BPT_STATE_STEP_COMPLEATE,
	UBA_BPT_STATE_TEST_FAILED,
	UBA_BPT_STATE_TEST_COMPLEATE,
	UBA_BPT_STATE_MAX,
	UBA_BPT_STATE_INVALID,

} UBA_BPT_STATE;

typedef struct UBA_BPT {
	UBA_BPT_TYPE type;
	UBA_BPT_EWI ewi;
	struct {
		UBA_BPT_STATE pre;
		UBA_BPT_STATE current;
		UBA_BPT_STATE next;
	} state;
	char name[UBA_BPT_NAME_MAX_SIZE];
	uint32_t start_time;
	UBA_BPT_step *head_step;
	UBA_BPT_step *current_step;
	uint8_t last_step_index;
	UBA_channel *ch;
	struct{
		RTC_DateTypeDef date;
		RTC_TimeTypeDef time;
	}start_date_time;
} UBA_BPT;

bool UBA_BPT_isRunning(UBA_BPT *bpt);
bool UBA_BPT_isPause(UBA_BPT *bpt);
bool UBA_BPT_start(UBA_BPT *bpt);
bool UBA_BPT_stop(UBA_BPT *bpt);
bool UBA_BPT_load(UBA_BPT *bpt);
bool UBA_BPT_begin(UBA_BPT *bpt);
bool UBA_BPT_pause_test(UBA_BPT *bpt);
bool UBA_BPT_isUnpacked(UBA_BPT *bpt);
void UBA_BPT_init_list();

void UBA_BPT_run(UBA_BPT *bpt);
UBA_BPT_EWI UBA_BPT_pair(UBA_BPT *bpt, UBA_channel *ch, int list_index);

#endif /* UBA_BATTERY_PERFORMANCE_TEST_H_ */

