/*
 * UBA_util.c
 *
 *  Created on: Nov 18, 2024
 *      Author: ORA
 */

#include "UBA_util.h"


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
    UART_LOG_INFO("PID","output:%f",output);
    return output;
}
