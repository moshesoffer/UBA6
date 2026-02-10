/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWML1_CH1_Pin GPIO_PIN_13
#define PWML1_CH1_GPIO_Port GPIOC
#define F_CS_Pin GPIO_PIN_0
#define F_CS_GPIO_Port GPIOF
#define CHRG_EN_CH1_Pin GPIO_PIN_1
#define CHRG_EN_CH1_GPIO_Port GPIOF
#define ADC1_DSCH_CURR_CH1_Pin GPIO_PIN_0
#define ADC1_DSCH_CURR_CH1_GPIO_Port GPIOC
#define ADC1_NTC_BAT_CH1_Pin GPIO_PIN_1
#define ADC1_NTC_BAT_CH1_GPIO_Port GPIOC
#define ADC1_AMB_TEMP_CH1_Pin GPIO_PIN_2
#define ADC1_AMB_TEMP_CH1_GPIO_Port GPIOC
#define BUTTON_INT_Pin GPIO_PIN_3
#define BUTTON_INT_GPIO_Port GPIOC
#define BUTTON_INT_EXTI_IRQn EXTI3_IRQn
#define ADC1_VBAT_CH1_Pin GPIO_PIN_0
#define ADC1_VBAT_CH1_GPIO_Port GPIOA
#define ADC1_VGEN_CH1_Pin GPIO_PIN_1
#define ADC1_VGEN_CH1_GPIO_Port GPIOA
#define DAC_DSCHG_CURR_CH1_Pin GPIO_PIN_4
#define DAC_DSCHG_CURR_CH1_GPIO_Port GPIOA
#define DAC_DSCHG_CURR_CH2_Pin GPIO_PIN_5
#define DAC_DSCHG_CURR_CH2_GPIO_Port GPIOA
#define ADC2_VGEN_CH2_Pin GPIO_PIN_6
#define ADC2_VGEN_CH2_GPIO_Port GPIOA
#define ADC2_NTC_BAT_CH2_Pin GPIO_PIN_7
#define ADC2_NTC_BAT_CH2_GPIO_Port GPIOA
#define ADC2_DSCH_CURR_CH2_Pin GPIO_PIN_5
#define ADC2_DSCH_CURR_CH2_GPIO_Port GPIOC
#define PWMH2_CH2_Pin GPIO_PIN_0
#define PWMH2_CH2_GPIO_Port GPIOB
#define BUTTON_SER_Pin GPIO_PIN_7
#define BUTTON_SER_GPIO_Port GPIOE
#define CHRG_EN_CH2_Pin GPIO_PIN_8
#define CHRG_EN_CH2_GPIO_Port GPIOE
#define PWMH1_CH1_Pin GPIO_PIN_9
#define PWMH1_CH1_GPIO_Port GPIOE
#define PWMH2_CH1_Pin GPIO_PIN_10
#define PWMH2_CH1_GPIO_Port GPIOE
#define uSD_CS_Pin GPIO_PIN_11
#define uSD_CS_GPIO_Port GPIOE
#define uSD_SCK_Pin GPIO_PIN_12
#define uSD_SCK_GPIO_Port GPIOE
#define uSD_MISO_Pin GPIO_PIN_13
#define uSD_MISO_GPIO_Port GPIOE
#define uSD_MOSI_Pin GPIO_PIN_14
#define uSD_MOSI_GPIO_Port GPIOE
#define LCD_Reset_Pin GPIO_PIN_15
#define LCD_Reset_GPIO_Port GPIOE
#define Negative_CH1_Pin GPIO_PIN_10
#define Negative_CH1_GPIO_Port GPIOB
#define ADC2_VBAT_CH2_Pin GPIO_PIN_11
#define ADC2_VBAT_CH2_GPIO_Port GPIOB
#define DISP_CS_Pin GPIO_PIN_12
#define DISP_CS_GPIO_Port GPIOB
#define LCD_SCK_Pin GPIO_PIN_13
#define LCD_SCK_GPIO_Port GPIOB
#define LCD_MISO_Pin GPIO_PIN_14
#define LCD_MISO_GPIO_Port GPIOB
#define ADC2_AMB_TEMP_CH2_Pin GPIO_PIN_15
#define ADC2_AMB_TEMP_CH2_GPIO_Port GPIOB
#define FAN_Pin GPIO_PIN_8
#define FAN_GPIO_Port GPIOD
#define DISCH_EN_CH1_Pin GPIO_PIN_9
#define DISCH_EN_CH1_GPIO_Port GPIOD
#define DISCH_EN_CH2_Pin GPIO_PIN_10
#define DISCH_EN_CH2_GPIO_Port GPIOD
#define I2C_SCL_CH2_Pin GPIO_PIN_6
#define I2C_SCL_CH2_GPIO_Port GPIOC
#define PWML2_CH2_Pin GPIO_PIN_7
#define PWML2_CH2_GPIO_Port GPIOC
#define BUTTON_LD_Pin GPIO_PIN_8
#define BUTTON_LD_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOC
#define I2C_SCL_CH1_Pin GPIO_PIN_8
#define I2C_SCL_CH1_GPIO_Port GPIOA
#define PWML2_CH1_Pin GPIO_PIN_9
#define PWML2_CH1_GPIO_Port GPIOA
#define LCD_MOSI_Pin GPIO_PIN_11
#define LCD_MOSI_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define JTDI_Pin GPIO_PIN_15
#define JTDI_GPIO_Port GPIOA
#define PWML1_CH2_Pin GPIO_PIN_10
#define PWML1_CH2_GPIO_Port GPIOC
#define Negative_CH2_Pin GPIO_PIN_11
#define Negative_CH2_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_12
#define BUZZER_GPIO_Port GPIOC
#define virtual_DE_Pin GPIO_PIN_0
#define virtual_DE_GPIO_Port GPIOD
#define LCD_DC_Pin GPIO_PIN_2
#define LCD_DC_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define BCLK_Pin GPIO_PIN_4
#define BCLK_GPIO_Port GPIOB
#define I2C_SDA_CH1_Pin GPIO_PIN_5
#define I2C_SDA_CH1_GPIO_Port GPIOB
#define PWMH1_CH2_Pin GPIO_PIN_6
#define PWMH1_CH2_GPIO_Port GPIOB
#define I2C_SDA_CH2_Pin GPIO_PIN_7
#define I2C_SDA_CH2_GPIO_Port GPIOB
#define uSD_detect_Pin GPIO_PIN_9
#define uSD_detect_GPIO_Port GPIOB
#define uSD_detect_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
#define SD_SPI_HANDLE hspi4

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
