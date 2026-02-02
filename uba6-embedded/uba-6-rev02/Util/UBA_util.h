/*
 * UBA_util.h
 *
 *  Created on: Sep 23, 2024
 *      Author: ORA
 */

#ifndef UBA_UTIL_H_
#define UBA_UTIL_H_

#include "uart_log.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_rtc.h"


#define UBA_LCD_MIN(a,b) (((a)<(b))?(a):(b))
#define UBA_LCD_MAX(a,b) (((a)>(b))?(a):(b))
#define UBA_IN_RANGE_HYST(value, target, hyst) \
    ( ((value) >= ((target) - (hyst))) && \
      ((value) <= ((target) + (hyst))) )

#define UBA_LOG_LEVEL_GLOBAL  	UART_LOG_LEVEL_INFO

#define UBA_LOG_LEVEL_CHANNEL  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_TPL0102  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_MCP3221  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_MCP47X6 	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_LINE  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_BPT  		UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_CMD  		UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_COMM 		UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_FILE 		UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_FILE_MAN 	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_PROTO_H 	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_TR		UBA_LOG_LEVEL_GLOBAL

#define UBA_RTC_STR_FORMAT_LOG ("%04d-%02d-%02d %02d:%02d:%02d")
#define UBA_RTC_STR_FORMAT_FILE ("%04d%02d%02d%02d%02d%02d")

typedef struct{
	float slop;
	float y_intercept;
}liner_equation;

#define LINER_EQUATION_DEFUALT	\
	{							\
	1.0f,						\
	0.0f						\
}


typedef struct {
    float Kp;           // Proportional gain
    float Ki;           // Integral gain
    float Kd;           // Derivative gain
    float prev_error;    // Previous error value
    float integral;     // Integral accumulator
    float setpoint;     // Desired setpoint
} PID_controller;

void PID_init(PID_controller *pid, float Kp, float Ki, float Kd, float setpoint) ;
float PID_compute(PID_controller *pid, float measuredValue, float dt);
void UBA_util_print_buffer(uint8_t *buffer, uint32_t size);
char* get_rtc_date_time_str(char * str_buf,uint32_t str_max_len, RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime, char * format);
char * get_RTC_date_time_str(void);
uint32_t RTC_datetime2unix_timestamp(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime);
void unix_timestamp2RTC_datetime(uint32_t timestamp, RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime);
uint32_t get_RTC_unix_timestamp(void);
void DWT_Delay_Init(void);
void DWT_Delay_Cycles(uint32_t cycles);


#endif /* UBA_UTIL_H_ */
