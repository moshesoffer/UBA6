/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.h
 * @brief   This file contains all the function prototypes for
 *          the adc.c file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

extern ADC_HandleTypeDef hadc2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_ADC1_Init(void);
void MX_ADC2_Init(void);

/* USER CODE BEGIN Prototypes */
typedef enum ADC_CHNNEL {
	ADC_CHNNEL_VBAT = 0x00, /*ADC1_IN1,	ADC2_IN14	*/
	ADC_CHNNEL_VGEN = 0x01, /*ADC1_IN2,	ADC2_IN3 	*/
	ADC_CHNNEL_AMB_TEMP = 0x02, /*ADC1_IN8,	ADC2_IN15 	*/
	ADC_CHNNEL_NTC_BAT = 0x03, /*ADC1_IN7,	ADC2_IN4 	*/
	ADC_CHNNEL_DSCH_CURR = 0x04, /*ADC1_IN6,	ADC2_IN11	*/
	ADC_CHNNEL_VPS	= 0x05, /*ADC1_IN12 ADC2_IN12, */
	ADC_CHNNEL_MAX = 0x06,
} ADC_CHNNEL;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

