/*
 * UBA_6.h
 *
 *  Created on: Sep 24, 2024
 *      Author: ORA
 */

#ifndef UBA_6_H_
#define UBA_6_H_

#include "stddef.h"
#include "stdint.h"
#include "UBA_battery_performance_test.h"
#include "UBA_PROTO_UBA6.pb.h"

#define UBA_FIRMWARE_MAJOR 	(0)
#define UBA_FIRMWARE_MINOR 	(0)
#define UBA_FIRMWARE_PATCH 	(2)
#define UBA_FIRMWARE_BUILD 	(0)
#define UBA_MAX_NAME_SIZE (32)  /*sizeof _UBA_PROTO_UBA6_settings.name*/
#define UBA_DEFUALT_ADRESS (0x80000000)

typedef enum UBA_6_STATE{
	UBA_6_STATE_INIT = UBA_PROTO_UBA6_STATE_INIT,
	UBA_6_STATE_SINGLE_CHANNELS =UBA_PROTO_UBA6_STATE_SINGLE_CHANNELS,
	UBA_6_STATE_DUAL_CHANNEL =UBA_PROTO_UBA6_STATE_DUAL_CHANNEL,
	UBA_6_STATE_MAX,
	UBA_6_STATE_INVALID,
}UBA_6_STATE;

typedef enum UBA_6_ERROR{
	UBA_6_ERROR_NO_ERROR= UBA_PROTO_UBA6_ERROR_NO_ERROR,
	UBA_6_ERROR_PARAM= 0x0001,
}UBA_6_ERROR;

typedef enum UBA_6_HV_CONSUMER{
	UBA_6_HV_CONSUMER_NONE = 0x00,
	UBA_6_HV_CONSUMER_LINE_A = 0x01,
	UBA_6_HV_CONSUMER_LINE_B = 0x02,
	UBA_6_HV_CONSUMER_FAN = 0x04,
	UBA_6_HV_CONSUMER_MAX = 0x08,
}UBA_6_HV_CONSUMER;

typedef struct UBA_6{
	UBA_6_ERROR error;
	struct {
		UBA_6_STATE pre;
		UBA_6_STATE current;
		UBA_6_STATE next;
		uint32_t tick;/*the time the line enter a new state*/
	}state;
	UBA_BPT  BPT_A;//channel A BPT
	UBA_BPT  BPT_B;//channel B BPT
	UBA_BPT  BPT_AB;//channel AB BPT
	uint8_t high_voltage_consumers;
	bool isFan_on;
	UBA_PROTO_UBA6_settings settings;
	UBA_PROTO_UBA6_info info;
}UBA_6;

#define UBA_6_DEFUALT 		\
{							\
	UBA_6_ERROR_NO_ERROR,	\
	{						\
		UBA_6_STATE_INIT,	\
		UBA_6_STATE_INIT,	\
		UBA_6_STATE_INIT,	\
		0					\
	},						\
	{0},					\
	{0},					\
	{0},					\
	0,						\
	false,					\
	UBA_PROTO_UBA6_settings_init_zero,		\
	UBA_PROTO_UBA6_info_init_zero,		\
}

#endif /* UBA_6_H_ */
#define UBA_VPS_MIN_VALUE (9000)

extern UBA_6 UBA_6_device_g;

void UBA_6_run(UBA_6 * uba);

UBA_6_ERROR UBA_6_set_next_state(UBA_6 * uba,UBA_6_STATE next_state);

UBA_6_ERROR  UBA_6_high_voltage_enable(UBA_6 * uba,UBA_6_HV_CONSUMER c);
UBA_6_ERROR  UBA_6_high_voltage_disable(UBA_6 * uba,UBA_6_HV_CONSUMER c);
void UBA_6_update_message(UBA_6 * uba,UBA_PROTO_UBA6_status * msg);
void UBA_6_command_execute(UBA_6 *uba, UBA_PROTO_UBA6_command *cmd);


