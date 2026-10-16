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
#include "UBA_PROTO_TR.pb.h"
#define UBA_TR_MAX_LENGTH (10)
#define UBA_TR_NAME_MAX_SIZE (32)



typedef enum TEST_ROUTINE_STEP_TYPE {
	TEST_ROUTINE_STEP_TYPE_CHARGE = UBA_BPT_STEP_TYPE_CHARGE,
	TEST_ROUTINE_STEP_TYPE_DISCHARGE = UBA_BPT_STEP_TYPE_DISCHARGE,
	TEST_ROUTINE_STEP_TYPE_DELAY = UBA_BPT_STEP_TYPE_DELAY,
	TEST_ROUTINE_STEP_TYPE_LOOP,
	TEST_ROUTINE_STEP_TYPE_MAX,
	TEST_ROUTINE_STEP_TYPE_INVALID,
} TEST_ROUTINE_STEP_TYPE;

#define UBA_TR_CURRENT_DEFUALTE  	\
{									\
	UBA_TR_CURRENT_TYPE_ABSOLUTE,	\
	100								\
}
typedef struct UBA_TR_loop {
	uint8_t loop_to_step;
	uint16_t loop_size;
	uint16_t loop_counter;
} UBA_TR_loop;

typedef struct UBA_TR_config_step {
	TEST_ROUTINE_STEP_TYPE type_id;
	union {
		UBA_BPT_charge charge;
		UBA_BPT_discharge discharge;
		UBA_BPT_delay delay;
		UBA_TR_loop loop;
	} type;
} UBA_TR_config_step;



#define UBA_TR_CONFIG_STEP_DEMO_CHARGE		\
{											\
	TEST_ROUTINE_STEP_TYPE_CHARGE,			\
	.type = {.charge =UBA_BPT_CHARGE_DEMO_HIGH}	\
}

#define UBA_TR_CONFIG_STEP_DEMO_CHARGE_LOW		\
{											\
	TEST_ROUTINE_STEP_TYPE_CHARGE,			\
	.type = {.charge =UBA_BPT_CHARGE_DEMO_LOW}	\
}

#define UBA_TR_CONFIG_STEP_DEMO_DISCHARGE	\
{											\
	TEST_ROUTINE_STEP_TYPE_DISCHARGE,		\
	.type = {.discharge =UBA_BPT_DISCHARGE_DEMO}				\
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

#define UBA_TR_DEMO 						\
{											\
	UBA_BPT_TYPE_SINGLE_CHANNEL,			\
	"DEMO Test\0",							\
	5,										\
	UBA_BAT_DEMO,							\
	.config ={								\
		UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_CHARGE_LOW, 	\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DELAY, 		\
		UBA_TR_CONFIG_STEP_DEMO_LOOP 		\
	}										\
}

#define UBA_TR_DEMO_DISCHRAGE 				\
{											\
	UBA_BPT_TYPE_SINGLE_CHANNEL,			\
	"Discharge\0",							\
	2,										\
	UBA_BAT_DEMO,							\
	.config ={								\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
		UBA_TR_CONFIG_STEP_DEMO_DISCHARGE, 	\
	}										\
}

#define UBA_TR_DEMO_CHRAGE 					\
{											\
	UBA_BPT_TYPE_SINGLE_CHANNEL,			\
	"Charge H\0",								\
	2,										\
	UBA_BAT_DEMO,							\
	.config ={								\
			UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
			UBA_TR_CONFIG_STEP_DEMO_CHARGE, 	\
	}										\
}

#define UBA_TR_DEMO_CHRAGE_L 					\
{											\
	UBA_BPT_TYPE_SINGLE_CHANNEL,			\
	"Charge L\0",								\
	2,										\
	UBA_BAT_DEMO,							\
	.config ={								\
			UBA_TR_CONFIG_STEP_DEMO_CHARGE_LOW,\
			UBA_TR_CONFIG_STEP_DEMO_CHARGE_LOW,\
	}										\
}

#define UBA_TR_DUAL_DEMO 						\
{											\
	UBA_BPT_TYPE_DUAL_CHANNEL,				\
	"DEMO Dual Test\0",							\
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
	"Self Test\0",							\
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

int32_t UBA_TR_unpack(TR_Test_Routine *tr, UBA_BPT*); // unpack the test ro the bpt
void UBA_TR_print(TR_Test_Routine *tr);
#define UBA_TR_LIST_SIZE (10)
extern  TR_Test_Routine_File TR_file;
int UBA_TR_demo_init(void);
#endif /* UBA_TEST_ROUTINE_H_ */
