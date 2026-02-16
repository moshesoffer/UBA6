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

//refresh LCD (presss button)
#define LCD_REFRESH

#ifdef LCD_REFRESH
#define LCD_REFRESH_PRESS_DELAY  300 /*ms*/
uint32_t lcd_last_press_time = 0;
uint32_t lcd_refresh_consecutive_press = 0;
#endif/*LCD_REFRESH*/

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
#ifdef LCD_REFRESH
				//refresh LCD - upon 3 consecutive button press			
				uint32_t press_time = HAL_GetTick();
				if ((press_time - lcd_last_press_time) < LCD_REFRESH_PRESS_DELAY)
				{
					lcd_refresh_consecutive_press++;
					if (lcd_refresh_consecutive_press >= 2)//actually 3 consecutive button press
					{
						LCD_refresh(&UBA_LCD_g);
						lcd_refresh_consecutive_press = 0;
					}
				}
				else
				{
					lcd_refresh_consecutive_press = 0;
#endif/*LCD_REFRESH*/

					//set button state
					UART_LOG_DEBUG(UBA_COMP, "who Press: UBA_BUTTON_G_DOWN_CH2");
					UBA_BTN_CH_2_DOWN.state = UBA_BUTTON_STATE_SHORT_PRESS_PENDING;
#ifdef LCD_REFRESH
				}
				//refresh LCD - update last press time;
				lcd_last_press_time = press_time;
#endif/*LCD_REFRESH*/
			}				
		} else {
			// Falling edge detected
			HAL_GPIO_WritePin(buttons_driver_g.load_pin.GPIOx, buttons_driver_g.load_pin.GPIO_Pin, PLI74HC166_LOAD);
		}

	}

	if ((UBA_BTN_CH_1_SELECT.state == UBA_BUTTON_STATE_PRESSED) && (UBA_BTN_CH_2_SELECT.state == UBA_BUTTON_STATE_PRESSED)) {
		UBA_buzzer_play_melody(&buzzer_g, UBA_BUZZER_BUZZ_DOOM);
	}

}

