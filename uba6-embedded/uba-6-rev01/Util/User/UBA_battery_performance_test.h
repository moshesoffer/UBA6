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
#include  "UBA_PROTO_BPT.pb.h"
#include "UBA_PROTO_UBA6.pb.h"

#define UBA_BPT_STMP_MAX_TIME (0xffffffff)
#define UBA_BPT_NAME_MAX_SIZE (32)
#define UBA_BPT_FILENAME_MAX_SIZE (64)

#define UBA_BPT_CRITICAL (0)
#define UBA_BPT_ERROR	(0)
#define UBA_BPT_WARNNING (0)


typedef enum UBA_BPT_STEP_TYPE {
	UBA_BPT_STEP_TYPE_CHARGE =UBA_PROTO_BPT_STEP_TYPE_CHARGE,
	UBA_BPT_STEP_TYPE_DISCHARGE =UBA_PROTO_BPT_STEP_TYPE_DISCHARGE,
	UBA_BPT_STEP_TYPE_DELAY =UBA_PROTO_BPT_STEP_TYPE_DELAY,
	UBA_BPT_STEP_TYPE_MAX =UBA_PROTO_BPT_STEP_TYPE_MAX,
	UBA_BPT_STEP_TYPE_INVALID = UBA_PROTO_BPT_STEP_TYPE_INVALID

} UBA_BPT_STEP_TYPE;


#define UBA_BPT_DISCHARGE_CURRENT_DEMO 	\
{										\
	UBA_PROTO_BPT_DISCHARGE_CURRENT_TYPE_ABSOLUTE,		\
	500									\
}


typedef struct UBA_BPT_charge {
	UBA_PROTO_BPT_SOURCE source;
	int32_t current;
	int32_t voltage;
	float min_temperature;
	struct {
		float max_emperature; /*the max temp in c*/
		uint32_t max_time; /*the max time in ms*/
		int32_t cut_off_current; /*the current of charge that below it the step will end*/
		int32_t limit_capacity; /*the capacity in mAh that above it the step will end*/
	} stop_condition;
} UBA_BPT_charge;

#define UBA_BPT_CHARGE_DEMO_LOW 	\
{								\
	UBA_PROTO_BPT_SOURCE_INTERNAL,	\
	300,						\
	4200,/*4.2 V*/						\
	-273.0f,					\
	{							\
		500.0f,					\
		500000,/*500 Sec */		\
		0, /* 0mAh (never stops) */			\
		800, /*800 mAh*/			\
	}							\
}


#define UBA_BPT_CHARGE_DEMO_HIGH 	\
{								\
	UBA_PROTO_BPT_SOURCE_INTERNAL,	\
	300,						\
	55000,/*55.0 V*/						\
	-273.0f,					\
	{							\
		500.0f,					\
		500000,/*500 Sec */		\
		0, /* 0mAh (never stops) */			\
		800, /*800 mAh*/			\
	}							\
}


typedef struct UBA_BPT_discharge {
	UBA_PROTO_BPT_SOURCE source;
	UBA_PROTO_BPT_discharge_current current;
	float min_temperature;
	struct {
		float max_emperature;
		uint32_t max_time;
		int32_t cut_off_voltage;
		int32_t limit_capacity; /*the capacity in mAh that above it the step will end*/
	} stop_condition;
} UBA_BPT_discharge;

#define UBA_BPT_DISCHARGE_DEMO 	\
{								\
	UBA_PROTO_BPT_SOURCE_INTERNAL,	\
	UBA_BPT_DISCHARGE_CURRENT_DEMO,		\
	-273.0f,					\
	{							\
		500.0f,					\
		5000,/*50 Sec */		\
		3400,					\
		5000, /*5Ah*/			\
	},							\
}



typedef struct UBA_BPT_delay {
	uint32_t delay_time;
	float cool_down_emperature;
} UBA_BPT_delay;

typedef struct UBA_BPT_step {
	uint8_t step_index;
	uint8_t plan_index;
	UBA_BPT_STEP_TYPE type_id;
	struct UBA_BPT_step *next;
	struct {
		uint32_t step_start;// the time that the step state started
		uint32_t step_completed;
		uint32_t step_action_start;//the actual time the step logic started
		uint32_t step_max_time;
	} timing;
	union {
		UBA_BPT_charge charge;
		UBA_BPT_discharge discharge;
		UBA_BPT_delay delay;
	} type;
} UBA_BPT_step;

#define UBA_BPT_STEP_DEFUALT	\
{								\
	0,							\
	0,							\
	UBA_BPT_STEP_TYPE_CHARGE,	\
	NULL,						\
	{0,0,0,UBA_BPT_STMP_MAX_TIME}	\
	.type = {.charge ={0}}	\
}

typedef enum UBA_BPT_TYPE {
	UBA_BPT_TYPE_SINGLE_CHANNEL,
	UBA_BPT_TYPE_DUAL_CHANNEL,
	UBA_BPT_TYPE_MAX,
	UBA_BPT_TYPE_INVALED
} UBA_BPT_MODE;

typedef enum UBA_BPT_STATE {
	UBA_BPT_STATE_INIT = UBA_PROTO_BPT_STATE_INIT,
	UBA_BPT_STATE_STANDBY = UBA_PROTO_BPT_STATE_STANDBY,
	UBA_BPT_STATE_PAUSE = UBA_PROTO_BPT_STATE_PAUSE,
	UBA_BPT_STATE_RUN_STEP = UBA_PROTO_BPT_STATE_RUN_STEP,
	UBA_BPT_STATE_STEP_COMPLEATE = UBA_PROTO_BPT_STATE_STEP_COMPLEATE,
	UBA_BPT_STATE_TEST_FAILED = UBA_PROTO_BPT_STATE_TEST_FAILED,
	UBA_BPT_STATE_TEST_COMPLEATE = UBA_PROTO_BPT_STATE_TEST_COMPLEATE,
	UBA_BPT_STATE_MAX,
	UBA_BPT_STATE_INVALID,
} UBA_BPT_STATE;

typedef struct UBA_BPT {
	UBA_BPT_MODE type;
	UBA_PROTO_UBA6_ERROR error;
	struct {
		UBA_BPT_STATE pre;
		UBA_BPT_STATE current;
		UBA_BPT_STATE next;
	} state;
	char name[UBA_BPT_NAME_MAX_SIZE];
	uint32_t log_tick_ms;
	uint16_t log_intreval;
	UBA_BPT_step *head_step;
	UBA_BPT_step *current_step;
	uint8_t last_step_index;
	UBA_channel *ch;
	struct {
		RTC_DateTypeDef date;
		RTC_TimeTypeDef time;
	} start_date_time;
	uint8_t filename[UBA_BPT_FILENAME_MAX_SIZE]; /*the file name that the data will be store at*/
	uint8_t TR_selected_index;
} UBA_BPT;




bool UBA_BPT_isRunning(UBA_BPT *bpt);
bool UBA_BPT_isPause(UBA_BPT *bpt);
bool UBA_BPT_start(UBA_BPT *bpt);
bool UBA_BPT_stop(UBA_BPT *bpt);
bool UBA_BPT_load(UBA_BPT *bpt);
UBA_STATUS_CODE UBA_BPT_begin(UBA_BPT *bpt,uint8_t index);
bool UBA_BPT_pause_test(UBA_BPT *bpt);
bool UBA_BPT_isUnpacked(UBA_BPT *bpt);
void UBA_BPT_init_list();

void UBA_BPT_run(UBA_BPT *bpt);

UBA_PROTO_UBA6_ERROR UBA_BPT_pair(UBA_BPT *bpt, UBA_channel *ch, int list_index);

void UBA_BPT_command_execute(UBA_BPT *bpt,UBA_PROTO_BPT_command *cmd);
void UBA_BPT_update_message(UBA_BPT *bpt,UBA_PROTO_BPT_status_message *msg);

#endif /* UBA_BATTERY_PERFORMANCE_TEST_H_ */

