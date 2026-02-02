/*
 * IRQ_handle.c
 *
 *  Created on: Aug 15, 2024
 *      Author: ORA
 */
#include "IRQ_handle.h"
#include "uart_log.h"
#include "gpio.h"

#include "UBA_button.h"

#define UBA_COMP "IRQ handle"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (BUTTON_DOWN_CH1_Pin & GPIO_Pin) {
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_DOWN_CH1_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_1_DOWN, HAL_GPIO_ReadPin(BUTTON_DOWN_CH1_GPIO_Port, BUTTON_DOWN_CH1_Pin));
	}
	if (BUTTON_DOWN_CH2_Pin & GPIO_Pin) {
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_DOWN_CH2_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_2_DOWN, HAL_GPIO_ReadPin(BUTTON_DOWN_CH2_GPIO_Port, BUTTON_DOWN_CH2_Pin));
	}
	if (BUTTON_SET_CH1_Pin & GPIO_Pin) {
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_SET_CH1_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_1_SELECT, HAL_GPIO_ReadPin(BUTTON_SET_CH1_GPIO_Port, BUTTON_SET_CH1_Pin));
	}
	if (BUTTON_SET_CH2_Pin & GPIO_Pin) {
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_SET_CH2_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_2_SELECT, HAL_GPIO_ReadPin(BUTTON_SET_CH2_GPIO_Port, BUTTON_SET_CH2_Pin));
	}
	if (BUTTON_UP_CH1_Pin & GPIO_Pin){
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_UP_CH1_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_1_UP,HAL_GPIO_ReadPin(BUTTON_UP_CH1_GPIO_Port, BUTTON_UP_CH1_Pin));
	}
	if (BUTTON_UP_CH2_Pin & GPIO_Pin){
		UART_LOG_INFO(UBA_COMP, "IRQ Detected on pin BUTTON_UP_CH2_Pin");
		UBA_button_IRQ(&UBA_BTN_CH_2_UP,HAL_GPIO_ReadPin(BUTTON_UP_CH2_GPIO_Port, BUTTON_UP_CH2_Pin));
	}

}

