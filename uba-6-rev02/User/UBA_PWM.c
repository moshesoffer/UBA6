/*
 * UBA_PWM.c
 *
 *  Created on: Nov 19, 2024
 *      Author: ORA
 */

#include "UBA_PWM.h"
#include "UBA_util.h"

#define UBA_COMP "PWM"

void UBA_PWM_set_value(UBA_PWM *pwm, uint32_t new_value) {
	if (new_value > pwm->max_value) {
		UART_LOG_ERROR(UBA_COMP, "new_value:%04lu > %04lu ", new_value, pwm->max_value);
		return;
	}
	if (new_value < pwm->min_value) {
		UART_LOG_ERROR(UBA_COMP, "new_value:%04lu < %04lu ", new_value, pwm->min_value);
		return;
	}
	if(new_value > pwm->value +1){
		UART_LOG_WARNNING(UBA_COMP, "new_value:%04lu >> current Value %04lu ", new_value, pwm->value);
	}
	if (new_value + 1 < pwm->value ) {
		UART_LOG_WARNNING(UBA_COMP, "new_value:%04lu << current Value %04lu ", new_value, pwm->value);
	}
	for (int i = 0; i < UBA_PWM_BUFFER_SIZE; i++) {
		pwm->acc_dma_buff[i] = new_value;
	}
	pwm->value = new_value;
}

void UBA_PWM_init(UBA_PWM *pwm) {
	if (pwm == NULL) {
		UART_LOG_CRITICAL(UBA_COMP, "pwm is null");
		return;
	}
	UBA_PWM_set_value(pwm, pwm->min_value);
	UBA_PWM_stop(pwm);
}

void UBA_PWM_start(UBA_PWM *pwm) {
	HAL_StatusTypeDef ret = HAL_OK;
	if (pwm == NULL) {
		UART_LOG_CRITICAL(UBA_COMP, "pwm is null");
		return;
	}
	ret = HAL_TIM_PWM_Start_DMA(pwm->handle, pwm->channel, pwm->acc_dma_buff, UBA_PWM_BUFFER_SIZE);
	if (ret != HAL_OK) {
		UART_LOG_CRITICAL(UBA_COMP, "Failed to Start DMA: 0x%lx", ret);
	}
	ret = HAL_TIMEx_PWMN_Start(pwm->handle, pwm->channel);
	if (ret != HAL_OK) {
		UART_LOG_CRITICAL(UBA_COMP, "Failed to Start PWM: 0x%lx", ret);
	}
	UART_LOG_INFO(UBA_COMP,"Start PWM Channel:%lu" ,pwm->channel);
}

void UBA_PWM_stop(UBA_PWM *pwm) {
	HAL_StatusTypeDef ret = HAL_OK;
	if (pwm == NULL) {
		UART_LOG_CRITICAL(UBA_COMP, "pwm is null");
		return;
	}
	ret = HAL_TIM_PWM_Stop_DMA(pwm->handle, pwm->channel);
	if (ret != HAL_OK) {
		UART_LOG_CRITICAL(UBA_COMP, "Failed to STOP DMA: 0x%lx", ret);
	}
	ret = HAL_TIMEx_PWMN_Stop(pwm->handle, pwm->channel);
	if (ret != HAL_OK) {
		UART_LOG_CRITICAL(UBA_COMP, "Failed to STOP PWM: 0x%lx", ret);
	}
}

UBA_STATUS_CODE UBA_PWM_update(UBA_PWM *pwm, uint32_t new_value) {
	UART_LOG_DEBUG(UBA_COMP,"update new value:%04lu",new_value);
	if (pwm == NULL) {
		UART_LOG_CRITICAL(UBA_COMP, "pwm is null");
		return UBA_STATUS_NULL;
	}
	UBA_PWM_set_value(pwm, new_value);

	return UBA_STATUS_CODE_OK;
}

