/*
 * UBA_util.h
 *
 *  Created on: Sep 23, 2024
 *      Author: ORA
 */

#ifndef UBA_UTIL_H_
#define UBA_UTIL_H_
#include "uart_log.h"

#define UBA_LCD_MIN(a,b) (((a)<(b))?(a):(b))
#define UBA_LCD_MAX(a,b) (((a)>(b))?(a):(b))

#define UBA_LOG_LEVEL_GLOBAL  	UART_LOG_LEVEL_INFO
#define UBA_LOG_LEVEL_CHANNEL  	UART_LOG_LEVEL_WARNNING
#define UBA_LOG_LEVEL_TPL0102  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_LINE  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_BPT  		UBA_LOG_LEVEL_GLOBAL


#endif /* UBA_UTIL_H_ */
