/*
 * IRQ_handle.c
 *
 *  Created on: Aug 15, 2024
 *      Author: ORA
 */
#include "IRQ_handle.h"
#include "..\Util\uart_log.h"
#include "gpio.h"

#include "UBA_button.h"
#include "UBA_buzzer.h"
#include "PLI74HC166.h"
#include "uart_log.h"
#include "LCD.h"

#define PLI74HC166_LOAD (GPIO_PIN_RESET)
#define PLI74HC166_SHIFT (GPIO_PIN_SET)

#define UBA_COMP "IRQ handle"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == BUTTON_INT_Pin) {
		GPIO_PinState pinState = HAL_GPIO_ReadPin(BUTTON_INT_GPIO_Port, GPIO_Pin);
		if (pinState == GPIO_PIN_SET) {
			// Rising edge detected
			uint8_t who_press = PLI74HC166_read(&buttons_driver_g) & allb;
			UART_LOG_DEBUG(UBA_COMP, "who Press: %lx", who_press);
			if (who_press & UBA_BUTTON_A_UP_CH1) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_A_UP_CH1");
				UBA_BTN_CH_1_UP.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}
			if (who_press & UBA_BUTTON_B_SET_CH1) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_B_SET_CH1");
				UBA_BTN_CH_1_SELECT.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}
			if (who_press & UBA_BUTTON_C_DOWN_CH1) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_C_DOWN_CH1");
				UBA_BTN_CH_1_DOWN.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}
			if (who_press & UBA_BUTTON_E_UP_CH2) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_E_UP_CH2");
				UBA_BTN_CH_2_UP.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}
			if (who_press & UBA_BUTTON_F_SET_CH2) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_F_SET_CH2");
				UBA_BTN_CH_2_SELECT.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}
			if (who_press & UBA_BUTTON_G_DOWN_CH2) {
				UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_G_DOWN_CH2");
				UBA_BTN_CH_2_DOWN.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
			}

//					LCD_refresh(&UBA_LCD_g);
		} else {
			// Falling edge detected
			HAL_GPIO_WritePin(buttons_driver_g.load_pin.GPIOx, buttons_driver_g.load_pin.GPIO_Pin, PLI74HC166_LOAD);
		}

	}

	if ((UBA_BTN_CH_1_SELECT.state == UBA_BUTTON_STATE_PRESSED) && (UBA_BTN_CH_2_SELECT.state == UBA_BUTTON_STATE_PRESSED)) {
		UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_DOOM);
	}

}

