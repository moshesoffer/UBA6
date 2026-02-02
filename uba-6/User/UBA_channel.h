/*
 * UBA_channel.h
 *
 *  Created on: Aug 29, 2024
 *      Author: ORA
 */

#ifndef UBA_CHANNEL_H_
#define UBA_CHANNEL_H_
#include "gpio.h"
#include "i2c.h"
#include "stdbool.h"
#include "stddef.h"
#include "MCP47X6.h"
#include "TPL0102.h"
#include "MCP3221.h"
#include "UBA_line.h"

#define UBA_CHANNEL_MAX_LINES (2)

typedef enum UBA_CHANNLE_ID {
	UBA_CHANNLE_ID_NONE = 0x00,
	UBA_CHANNLE_ID_A = 0x01,
	UBA_CHANNLE_ID_B = 0x02,
	UBA_CHANNLE_ID_AB = UBA_CHANNLE_ID_A | UBA_CHANNLE_ID_B,
	UBA_CHANNLE_ID_MAX,
	UBA_CHANNLE_ID_INVALID = 0xff
} UBA_CHANNLE_ID;

typedef enum UBA_CHANNLE_ERROR{
	UBA_CHANNLE_ERROR_NO_ERROR = 0x00,
	UBA_CHANNLE_ERROR_EMPTY_LINE = 0x00,

}UBA_CHANNLE_ERROR;

typedef enum UBA_CHANNEL_STATE {
	UBA_CHANNEL_STATE_INIT,// configure the channel
	UBA_CHANNEL_STATE_STANDBY, // wait for external state change , make sure that the channel is in idle mode all the time
	UBA_CHANNEL_STATE_DELAY,
	UBA_CHANNEL_STATE_CHARGE,
	UBA_CHANNEL_STATE_DISCHARGE,
	UBA_CHANNEL_STATE_OFF,
	UBA_CHANNEL_STATE_MAX,
	UBA_CHANNEL_STATE_INVALID,
} UBA_CHANNEL_STATE;

typedef struct UBA_channle {
	UBA_CHANNLE_ID id;
	UBA_CHANNLE_ERROR error;
	uint8_t name[11];
	struct {
		UBA_CHANNEL_STATE pre;
		UBA_CHANNEL_STATE current;
		UBA_CHANNEL_STATE next;
	} state;
	UBA_line *lines_p[UBA_CHANNEL_MAX_LINES];
	uint8_t line_size;
} UBA_channel;

uint32_t UBA_channel_get_voltage(UBA_channel *ch);
uint32_t UBA_channel_get_temperature(UBA_channel *ch);
uint32_t UBA_channel_get_current(UBA_channel *ch);
uint32_t UBA_channel_get_capacity(UBA_channel *ch);
uint32_t UBA_channel_set_next_state(UBA_channel *ch, UBA_CHANNEL_STATE next_state);
void UBA_channel_run(UBA_channel *ch);

bool UBA_channel_isInCriticalError(UBA_channel *ch);

extern UBA_channel UBA_CH_A;
extern UBA_channel UBA_CH_B;
extern UBA_channel UBA_CH_AB;

void UBA_channel_init_g(void);
#endif /* UBA_CHANNEL_H_ */

