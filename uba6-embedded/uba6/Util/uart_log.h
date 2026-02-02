/*
 * uart_log.h
 *
 *  Created on: Aug 12, 2024
 *      Author: ORA
 */

#ifndef UART_LOG_H_
#define UART_LOG_H_
#include "stdint.h"
#include "stdbool.h"

#define UART_LOG_LEVEL_ALL    	(0x00)
#define UART_LOG_LEVEL_DEBUG    (0x01)
#define UART_LOG_LEVEL_INFO		(0x02)
#define UART_LOG_LEVEL_WARNNING	(0x04)
#define UART_LOG_LEVEL_ERROR	(0x08)
#define UART_LOG_LEVEL_CRITICAL (0x10)
#define UART_LOG_LEVEL_COMM 	(0x80)


#define UART_LOG_LEVEL (UART_LOG_LEVEL_DEBUG)

#define UART_LOG_COMP_STR "|%-15s|"
#define UART_LOG_NL "\r\n"

bool uart_log_level(uint8_t level);

uint8_t uart_printf(const char *format, ...);
void print_time(void);
void test_print() ;

//fo
#define UBA_6_log(format, ...) 				\
  do {                                  	\
	  print_time();       					\
      uart_printf(format,##__VA_ARGS__);   	\
      uart_printf("\r\n"); 				  	\
  } while (0)
#endif /* UART_LOG_H_ */


#define UART_LOG_COMPONENT(level,UBA_COMP,format,...)	\
do{														\
	if(uart_log_level(level)){							\
		print_time();       							\
		uart_printf(UART_LOG_COMP_STR,UBA_COMP);		\
		uart_printf(format,##__VA_ARGS__);   			\
		uart_printf("\r\n"); 				  			\
	}													\
}while(0)


#define UART_LOG_DEBUG(COMP,format,...) 	UART_LOG_COMPONENT(UART_LOG_LEVEL_DEBUG,	COMP,format,##__VA_ARGS__)
#define UART_LOG_INFO(COMP,format,...) 		UART_LOG_COMPONENT(UART_LOG_LEVEL_INFO,		COMP,format,##__VA_ARGS__)
#define UART_LOG_WARNNING(COMP,format,...) 	UART_LOG_COMPONENT(UART_LOG_LEVEL_WARNNING,	COMP,format,##__VA_ARGS__)
#define UART_LOG_ERROR(COMP,format,...) 	UART_LOG_COMPONENT(UART_LOG_LEVEL_ERROR,	COMP,format,##__VA_ARGS__)
#define UART_LOG_CRITICAL(COMP,format,...) 	UART_LOG_COMPONENT(UART_LOG_LEVEL_CRITICAL,	COMP,format,##__VA_ARGS__)
#define UART_LOG_COMM(COMP,format,...) 		UART_LOG_COMPONENT(UART_LOG_LEVEL_COMM,		COMP,format,##__VA_ARGS__)
