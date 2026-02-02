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

#define UBA_FIRMWARE_MAJOR 	(0)
#define UBA_FIRMWARE_MINOR 	(0)
#define UBA_FIRMWARE_PATCH 	(1)
#define UBA_FIRMWARE_BUILD 	(0)




typedef enum UBA_6_STATE{
	UBA_6_STATE_INIT,
	UBA_6_STATE_SINGLE_CHANNELS,
	UBA_6_STATE_DUAL_CHANNEL,
	UBA_6_STATE_MAX,
	UBA_6_STATE_INVALID,
}UBA_6_STATE;

typedef enum UBA_6_ERROR{
	UBA_6_ERROR_NO_ERROR= 0x0000,
}UBA_6_ERROR;


typedef struct  UBA_firmware {
	uint16_t major; /**< Major release version */
	uint16_t minor; /**< Minor release version */
	uint16_t patch; /**< Patch release number */
	uint16_t build; /**< Build number */
} UBA_firmware;

#define  UBA_FIRMWARE_DEFUALT 	\
{								\
	UBA_FIRMWARE_MAJOR,			\
	UBA_FIRMWARE_MINOR,			\
	UBA_FIRMWARE_PATCH,			\
	UBA_FIRMWARE_BUILD,			\
}


typedef struct UBA_6{
	UBA_6_ERROR error;
	struct {
		UBA_6_STATE pre;
		UBA_6_STATE current;
		UBA_6_STATE next;
	}state;
	UBA_firmware firmware;
	UBA_BPT  BPT_A;//channel A BPT
	UBA_BPT  BPT_B;//channel B BPT
	UBA_BPT  BPT_AB;//channel AB BPT
}UBA_6;

#define UBA_6_DEFUALT 		\
{							\
	UBA_6_ERROR_NO_ERROR,	\
	{						\
		UBA_6_STATE_INIT,	\
		UBA_6_STATE_INIT,	\
		UBA_6_STATE_INIT	\
	},						\
	UBA_FIRMWARE_DEFUALT,	\
	{0},					\
	{0},					\
	{0}						\
}

#endif /* UBA_6_H_ */

extern UBA_6 UBA_6_device_g;

void UBA_6_run(UBA_6 * uba);

UBA_6_ERROR UBA_6_set_next_state(UBA_6 * uba,UBA_6_STATE next_state);


