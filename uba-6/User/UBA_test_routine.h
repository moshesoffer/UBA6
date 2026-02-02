/*
 * test_routine.h
 *
 *  Created on: Sep 25, 2024
 *      Author: ORA
 */

#ifndef UBA_TEST_ROUTINE_H_
#define UBA_TEST_ROUTINE_H_

#include "UBA_battery_performance_test.h"
#include "UBA_battary.h"
#include "inttypes.h"
#define UBA_TR_MAX_LENGTH (10)
#define UBA_TR_NAME_MAX_SIZE (32)

typedef enum TEST_ROUTINE_STEP_TYPE {
	TEST_ROUTINE_STEP_TYPE_CHARGE,
	TEST_ROUTINE_STEP_TYPE_DISCHARGE,
	TEST_ROUTINE_STEP_TYPE_DELAY,
	TEST_ROUTINE_STEP_TYPE_LOOP,
	TEST_ROUTINE_STEP_TYPE_MAX,
	TEST_ROUTINE_STEP_TYPE_INVALID,

} TEST_ROUTINE_STEP_TYPE;

typedef struct UBA_TR_charge {
	uint32_t current_tg;
} UBA_TR_charge;

typedef struct UBA_TR_discharge {
	uint32_t current_tg;
} UBA_TR_discharge;

typedef struct UBA_TR_delay {
	uint32_t delay_time;
} UBA_TR_delay;
typedef struct UBA_TR_loop {
	uint8_t loop_to_step;
	uint16_t loop_size;
	uint16_t loop_counter;

} UBA_TR_loop;

typedef struct UBA_TR_config_step {
	TEST_ROUTINE_STEP_TYPE type_id;
	union {
		UBA_TR_charge charge;
		UBA_TR_discharge discharge;
		UBA_TR_delay delay;
		UBA_TR_loop loop;
	} type;
} UBA_TR_config_step;

#define UBA_TR_CONFIG_STEP_DEMO_CHARGE 		\
{											\
	TEST_ROUTINE_STEP_TYPE_CHARGE,			\
	.type = {.charge ={0}}					\
}

#define UBA_TR_CONFIG_STEP_DEMO_DISCHARGE	\
{											\
	TEST_ROUTINE_STEP_TYPE_DISCHARGE,		\
	.type = {.discharge={0}}				\
}
#define UBA_TR_CONFIG_STEP_DEMO_DELAY		\
{											\
	TEST_ROUTINE_STEP_TYPE_DELAY,			\
	.type = {.delay={100000}}				\
}

#define UBA_TR_CONFIG_STEP_DEMO_LOOP		\
{											\
	TEST_ROUTINE_STEP_TYPE_LOOP,		 	\
	.type = {.loop = {0,5,0}}				\
}

typedef struct UBA_TR {
	UBA_BPT_TYPE type;
	char name[UBA_TR_NAME_MAX_SIZE];
	uint8_t length;
	UBA_battery battery;
	UBA_TR_config_step config[UBA_TR_MAX_LENGTH];
} UBA_TR;

#define UBA_TR_DEMO 						\
{											\
	UBA_BPT_TYPE_SINGLE_CHANNEL,			\
	"DEMO Test",							\
	5,										\
	UBA_BAT_DEMO,							\
	.config ={								\
		UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DELAY, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
	}										\
}
#define UBA_TR_DUAL_DEMO 						\
{											\
	UBA_BPT_TYPE_DUAL_CHANNEL,				\
	"DEMO Dual Test",							\
	4,										\
	UBA_BAT_DEMO,							\
	.config ={								\
		UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DELAY, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
	}										\
}
#define UBA_TR_DEMO_SELF_TEST				\
{											\
	UBA_BPT_TYPE_DUAL_CHANNEL,				\
	"Self Test",							\
	3,										\
	UBA_BAT_DEMO,							\
	.config ={								\
		UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DELAY, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP, 		\
	}										\
}

int32_t UBA_TR_unpack(UBA_TR * tr,UBA_BPT *); // unpack the
#define UBA_TR_LIST_SIZE (10)
extern UBA_TR TR_list[UBA_TR_LIST_SIZE];
int UBA_TR_demo_init(void);
#endif /* UBA_TEST_ROUTINE_H_ */
