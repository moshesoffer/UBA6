/*
 * UBA_util.c
 *
 *  Created on: Nov 18, 2024
 *      Author: ORA
 */

#include "UBA_util.h"
#include "rtc.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "stm32g474xx.h"

#define MAX_LENGTH_DATETIME_STR (64)

#define COMP "Util"

static char date_time_str[MAX_LENGTH_DATETIME_STR] = { 0 };

void PID_init(PID_controller *pid, float Kp, float Ki, float Kd, float setpoint) {
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	pid->prev_error = 0.0f;
	pid->integral = 0.0f;
	pid->setpoint = setpoint;
}

float PID_compute(PID_controller *pid, float measuredValue, float dt) {
	// Calculate the error
	float error = pid->setpoint - measuredValue;

	// Calculate the integral term
	pid->integral += error * dt;

	// Calculate the derivative term
	float derivative = (error - pid->prev_error) / dt;

	// Calculate the output
	float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

	// Update the previous error
	pid->prev_error = error;
	UART_LOG_INFO("PID", "output:%f", output);
	return output;
}

void UBA_util_print_buffer(uint8_t *buffer, uint32_t size) {
	UART_LOG_INFO(COMP, "Buffer[%lu]:", size);
	if (size) {
		uart_printf("%02X", buffer[0]);
	}
	for (uint32_t i = 1; i < size; i++) {
		uart_printf("-%02X", buffer[i]);
	}
	uart_printf("\r\n");
}

char* get_rtc_date_time_str(char *str_buf, uint32_t str_max_len, RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime, char *format) {
	memset(str_buf, 0, str_max_len);
	/* Print in a readable format */
	int n = snprintf(str_buf, str_max_len, format,
			2000 + sDate->Year, // HAL returns years since 2000
			sDate->Month,
			sDate->Date,
			sTime->Hours,
			sTime->Minutes,
			sTime->Seconds);
	if (n >= (int) str_max_len) {
		UART_LOG_ERROR(COMP, "Buffer length is to short(%lu,%d)", str_max_len, n);
	}

	return str_buf;
}

char* get_RTC_date_time_str(void) {
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	return get_rtc_date_time_str(date_time_str, MAX_LENGTH_DATETIME_STR, &sDate, &sTime, UBA_RTC_STR_FORMAT_LOG);
}

uint32_t RTC_datetime2unix_timestamp(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) {
	struct tm t;

	// STM32 RTC years are offset from 2000, Unix starts at 1900
	t.tm_year = sDate->Year + 100;   // Year since 1900
	t.tm_mon = sDate->Month - 1;    // Month 0–11
	t.tm_mday = sDate->Date;         // Day 1–31

	t.tm_hour = sTime->Hours;
	t.tm_min = sTime->Minutes;
	t.tm_sec = sTime->Seconds;

	t.tm_isdst = -1; // Not using daylight saving

	return mktime(&t);
}

void unix_timestamp2RTC_datetime(uint32_t timestamp, RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime) {
	struct tm *timeinfo;
	time_t rawtime = (time_t) timestamp;
	// Convert timestamp to broken-down time (UTC)
	timeinfo = gmtime(&rawtime);

	// Fill date struct (year is offset from 2000)
	sDate->Year = timeinfo->tm_year - 100;  // tm_year is years since 1900
	sDate->Month = timeinfo->tm_mon + 1;    // tm_mon: 0-11
	sDate->Date = timeinfo->tm_mday;
	sDate->WeekDay = ((timeinfo->tm_wday == 0) ? 7 : timeinfo->tm_wday); // RTC: 1=Mon ... 7=Sun

	// Fill time struct
	sTime->Hours = timeinfo->tm_hour;
	sTime->Minutes = timeinfo->tm_min;
	sTime->Seconds = timeinfo->tm_sec;
	sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime->StoreOperation = RTC_STOREOPERATION_RESET;
	UART_LOG_DEBUG("unix_timestamp2RTC_datetime", "%lu == %s", timestamp,
			get_rtc_date_time_str(date_time_str,MAX_LENGTH_DATETIME_STR,sDate, sTime,UBA_RTC_STR_FORMAT_LOG));
}

uint32_t get_RTC_unix_timestamp(void) {
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	return RTC_datetime2unix_timestamp(&sDate, &sTime);
}





void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}


void DWT_Delay_Cycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < cycles);
}




