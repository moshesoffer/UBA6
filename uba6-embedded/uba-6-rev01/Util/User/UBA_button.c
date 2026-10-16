/*
 * UBA_button.c
 *
 *  Created on: Sep 11, 2024
 *      Author: ORA
 */

#include "UBA_button.h"
#include "uart_log.h"
#include "stddef.h"
#include "stm32g4xx_hal.h"

#define UBA_COMP  "Button"
#define UBA_PRESS_TIME (50) /*200 one shot press time*/

UBA_button UBA_BTN_CH_1_UP;
UBA_button UBA_BTN_CH_2_UP;
UBA_button UBA_BTN_CH_1_DOWN;
UBA_button UBA_BTN_CH_2_DOWN;
UBA_button UBA_BTN_CH_1_SELECT;
UBA_button UBA_BTN_CH_2_SELECT;

int UBA_button_clear_panding(UBA_button *bnt) {
	if (bnt != NULL) {
		bnt->state = UBA_BUTTON_STATE_RELEASE;
	}
	return 0;
}

int UBA_button_IRQ(UBA_button *bnt, bool is_release) {
	switch (bnt->state) {
		case UBA_BUTTON_STATE_RELEASE:
			if (is_release) {
				// do noting
			} else {
				bnt->state = UBA_BUTTON_STATE_PRESSED;
				bnt->press_time = HAL_GetTick();
			}
			break;
		case UBA_BUTTON_STATE_PRESSED:
			if (is_release) {
				if ((HAL_GetTick() - bnt->press_time) > UBA_PRESS_TIME) {
					bnt->state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
				} else {
					bnt->state = UBA_BUTTON_STATE_RELEASE;
				}
			} else {
				// do noting
			}
			break;
		case UBA_BUTTON_STATE_SHORT_PRESS_PENDING:
			break;
		case UBA_BUTTON_STATE_LONG_PRESS_PENDING:
			break;
		case UBA_BUTTON_STATE_DOUBLE_PRESS_PENDING:
			break;
		default:
			}
	UART_LOG_DEBUG(UBA_COMP, "BUTTON_DOWN_CH1_Pin STATE:%u", bnt->state);
	return 0;
}

bool UBA_button_is_pending(UBA_button *bnt) {
	if (bnt != NULL) {
		return ((bnt->state & UBA_BUTTON_STATE_PENDING) > 0);
	} else {
		return false;
	}
}
