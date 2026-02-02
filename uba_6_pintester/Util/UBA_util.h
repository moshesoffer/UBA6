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

#define UBA_LOG_LEVEL_CHANNEL  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_TPL0102  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_MCP3221  	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_MCP47X6 	UBA_LOG_LEVEL_GLOBAL
#define UBA_LOG_LEVEL_LINE  	UART_LOG_LEVEL_DEBUG
#define UBA_LOG_LEVEL_BPT  		UBA_LOG_LEVEL_GLOBAL


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
#endif /* UBA_UTIL_H_ */
