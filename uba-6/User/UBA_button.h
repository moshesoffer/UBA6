/*
 * UBA_button.h
 *
 *  Created on: Sep 11, 2024
 *      Author: ORA
 */

#ifndef UBA_BUTTON_H_
#define UBA_BUTTON_H_

#include "stdint.h"
#include "stdbool.h"

typedef enum UBA_BUTTON_STATE{
	UBA_BUTTON_STATE_RELEASE = 0x00,
	UBA_BUTTON_STATE_PRESSED = 0x01,
	UBA_BUTTON_STATE_PENDING = 0x02,
	UBA_BUTTON_STATE_SHORT_PRESS_PENDING = 0x04|UBA_BUTTON_STATE_PENDING ,
	UBA_BUTTON_STATE_LONG_PRESS_PENDING = 0x08|UBA_BUTTON_STATE_PENDING ,
	UBA_BUTTON_STATE_DOUBLE_PRESS_PENDING = 0x10|UBA_BUTTON_STATE_PENDING ,
}UBA_BUTTON_STATE;

typedef struct UBA_button{
	UBA_BUTTON_STATE state;
	uint32_t press_time;
}UBA_button;


extern UBA_button UBA_BTN_CH_1_UP;
extern UBA_button UBA_BTN_CH_2_UP;
extern UBA_button UBA_BTN_CH_1_DOWN;
extern UBA_button UBA_BTN_CH_2_DOWN;
extern UBA_button UBA_BTN_CH_1_SELECT;
extern UBA_button UBA_BTN_CH_2_SELECT;

int UBA_button_clear_panding(UBA_button * bnt);
int UBA_button_IRQ(UBA_button *bnt, bool is_release);
bool UBA_button_is_pending(UBA_button *bnt);

#endif /* UBA_BUTTON_H_ */
