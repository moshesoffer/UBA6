/*
 * uart_log.c
 *
 *  Created on: Aug 12, 2024
 *      Author: ORA
 */

#include "uart_log.h"

#include "usart.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>
#include "stdbool.h"
#include "rtc.h"

#define ANSI_COLOR_BLACK   (0)
#define ANSI_COLOR_RED     (1)
#define ANSI_COLOR_GREEN   (2)
#define ANSI_COLOR_YELLOW  (3)
#define ANSI_COLOR_BLUE    (4)
#define ANSI_COLOR_MAGENTA (5)
#define ANSI_COLOR_CYAN    (6)
#define ANSI_COLOR_WHITE   (7)

#define ANSI_FG_COLOR_BASE			(30)
#define ANSI_FG_COLOR_BLACK			(ANSI_FG_COLOR_BASE+ANSI_COLOR_BLACK)
#define ANSI_FG_COLOR_RED         	(ANSI_FG_COLOR_BASE+ANSI_COLOR_RED)
#define ANSI_FG_COLOR_GREEN       	(ANSI_FG_COLOR_BASE+ANSI_COLOR_GREEN)
#define ANSI_FG_COLOR_YELLOW      	(ANSI_FG_COLOR_BASE+ANSI_COLOR_YELLOW)
#define ANSI_FG_COLOR_BLUE        	(ANSI_FG_COLOR_BASE+ANSI_COLOR_BLUE)
#define ANSI_FG_COLOR_MAGENTA     	(ANSI_FG_COLOR_BASE+ANSI_COLOR_MAGENTA)
#define ANSI_FG_COLOR_CYAN        	(ANSI_FG_COLOR_BASE+ANSI_COLOR_CYAN)
#define ANSI_FG_COLOR_WHITE       	(ANSI_FG_COLOR_BASE+ANSI_COLOR_WHITE)
#define ANSI_FG_COLOR_DEFAULT       (ANSI_FG_COLOR_BASE+9)

#define ANSI_FG_COLOR_BRIGTH_BASE			(90)
#define ANSI_FG_COLOR_BRIGTH_BLACK			(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_BLACK)
#define ANSI_FG_COLOR_BRIGTH_RED         	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_RED)
#define ANSI_FG_COLOR_BRIGTH_GREEN       	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_GREEN)
#define ANSI_FG_COLOR_BRIGTH_YELLOW      	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_YELLOW)
#define ANSI_FG_COLOR_BRIGTH_BLUE        	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_BLUE)
#define ANSI_FG_COLOR_BRIGTH_MAGENTA     	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_MAGENTA)
#define ANSI_FG_COLOR_BRIGTH_CYAN        	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_CYAN)
#define ANSI_FG_COLOR_BRIGTH_WHITE       	(ANSI_FG_COLOR_BRIGTH_BASE+ANSI_COLOR_WHITE)

#define ANSI_BG_COLOR_BASE			(40)
#define ANSI_BG_COLOR_BLACK			(ANSI_BG_COLOR_BASE+ANSI_COLOR_BLACK)
#define ANSI_BG_COLOR_RED         	(ANSI_BG_COLOR_BASE+ANSI_COLOR_RED)
#define ANSI_BG_COLOR_GREEN       	(ANSI_BG_COLOR_BASE+ANSI_COLOR_GREEN)
#define ANSI_BG_COLOR_YELLOW      	(ANSI_BG_COLOR_BASE+ANSI_COLOR_YELLOW)
#define ANSI_BG_COLOR_BLUE        	(ANSI_BG_COLOR_BASE+ANSI_COLOR_BLUE)
#define ANSI_BG_COLOR_MAGENTA     	(ANSI_BG_COLOR_BASE+ANSI_COLOR_MAGENTA)
#define ANSI_BG_COLOR_CYAN        	(ANSI_BG_COLOR_BASE+ANSI_COLOR_CYAN)
#define ANSI_BG_COLOR_WHITE       	(ANSI_BG_COLOR_BASE+ANSI_COLOR_WHITE)
#define ANSI_BG_COLOR_DEFAULT       (ANSI_BG_COLOR_BASE+9)

#define ANSI_BG_COLOR_BRIGTH_BASE			(100)
#define ANSI_BG_COLOR_BRIGTH_BLACK			(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_BLACK)
#define ANSI_BG_COLOR_BRIGTH_RED         	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_RED)
#define ANSI_BG_COLOR_BRIGTH_GREEN       	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_GREEN)
#define ANSI_BG_COLOR_BRIGTH_YELLOW      	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_YELLOW)
#define ANSI_BG_COLOR_BRIGTH_BLUE        	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_BLUE)
#define ANSI_BG_COLOR_BRIGTH_MAGENTA     	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_MAGENTA)
#define ANSI_BG_COLOR_BRIGTH_CYAN        	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_CYAN)
#define ANSI_BG_COLOR_BRIGTH_WHITE       	(ANSI_BG_COLOR_BRIGTH_BASE+ANSI_COLOR_WHITE)

#define ANSI_COLOR_SET 		"\x1B[%u;%um"

#define UART_LOG_DEBUG_FOREGROUND ANSI_FG_COLOR_GREEN
#define UART_LOG_DEBUG_BACKGROUND ANSI_BG_COLOR_BLACK

#define UART_LOG_INFO_FOREGROUND ANSI_FG_COLOR_WHITE
#define UART_LOG_INFO_BACKGROUND ANSI_BG_COLOR_BLACK

#define UART_LOG_WARNNIGN_FOREGROUND ANSI_FG_COLOR_YELLOW
#define UART_LOG_WARNNING_BACKGROUND ANSI_BG_COLOR_BLACK

#define UART_LOG_ERROR_FOREGROUND ANSI_FG_COLOR_RED
#define UART_LOG_ERROR_BACKGROUND ANSI_BG_COLOR_BLACK

#define UART_LOG_CRITICAL_FOREGROUND ANSI_FG_COLOR_YELLOW
#define UART_LOG_CRITICAL_BACKGROUND (ANSI_BG_COLOR_RED)

#define UART_LOG_COMMUNICATION_FOREGROUND ANSI_FG_COLOR_BLUE
#define UART_LOG_COMMUNICATION_BACKGROUND ANSI_BG_COLOR_BLACK

#define BUFFER_SIZE (254)

#define COMP "UART log"

#define LOG_CH huart2

static char buffer[BUFFER_SIZE] = { 0 };
static  RTC_TimeTypeDef sTime;
static RTC_DateTypeDef sDate;
void test_print();



uint8_t uart_printf(const char *format, ...) {
	va_list args;

	memset(buffer, 0, sizeof(buffer));
	va_start(args, format);
	vsprintf(buffer, format, args);
	perror(buffer);
	va_end(args);
	HAL_UART_Transmit(&LOG_CH, (uint8_t*) buffer, strlen(buffer) < sizeof(buffer) - 1 ? (uint8_t) strlen(buffer) : (uint8_t) (sizeof(buffer) - 1),
	HAL_MAX_DELAY);
	return (uint8_t) strlen(buffer);
}

void print_backg_and_foreg(uint8_t b, uint8_t f) {
	test_print();
}

void print_time(void) {
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	uart_printf("%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
}

void print_debug() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_DEBUG_BACKGROUND, UART_LOG_DEBUG_FOREGROUND);
}
void print_info() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_INFO_BACKGROUND, UART_LOG_INFO_FOREGROUND);
}

void print_warnning() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_WARNNING_BACKGROUND, UART_LOG_WARNNIGN_FOREGROUND);
}

void print_error() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_ERROR_BACKGROUND, UART_LOG_ERROR_FOREGROUND);
}

void print_critical() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_CRITICAL_BACKGROUND, UART_LOG_CRITICAL_FOREGROUND);
}

void print_comm() {
	uart_printf(ANSI_COLOR_SET, UART_LOG_COMMUNICATION_BACKGROUND, UART_LOG_COMMUNICATION_FOREGROUND);
}

void test_print() {
	UART_LOG_DEBUG(COMP,"Debug level");
	UART_LOG_INFO(COMP,"info level");
	UART_LOG_WARNNING(COMP,"warnning level");
	UART_LOG_ERROR(COMP,"error level");
	UART_LOG_CRITICAL(COMP,"critical level");
	UART_LOG_COMM(COMP,"comm level");
}

bool uart_log_level(uint8_t level) {
	if (level < UART_LOG_LEVEL) {
		return false;
	} else {
		switch (level) {
			case UART_LOG_LEVEL_DEBUG:
				print_debug();
				break;
			case UART_LOG_LEVEL_INFO:
				print_info();
				break;
			case UART_LOG_LEVEL_WARNNING:
				print_warnning();
				break;
			case UART_LOG_LEVEL_ERROR:
				print_error();
				break;
			case UART_LOG_LEVEL_CRITICAL:
				print_critical();
				break;
			case UART_LOG_LEVEL_COMM:
				print_comm();
				break;
			default:
				return false;
		}
	}
	return true;
}

