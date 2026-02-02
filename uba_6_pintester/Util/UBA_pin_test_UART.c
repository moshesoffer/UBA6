/*
 * UBA_pin_test_UART.c
 *
 *  Created on: Dec 3, 2024
 *      Author: ORA
 */

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart_log.h"
#include "usart.h"
#include "UBA_pin_test_UART.h"

#define RX_BUFFER_SIZE 128
char rxBuffer[RX_BUFFER_SIZE];   // Buffer for received data
volatile uint8_t rxData;         // Variable for single-byte reception
volatile uint8_t rxIndex = 0;

#define UBA_COMP  "PIN_TEST"

#define PIN_OUTPUT_CMD "PIN_OUTPUT"
#define PIN_INPUT_CMD "PIN_INPUT"
#define PORT_A "A"
#define PORT_B "B"
#define PORT_C "C"
#define PORT_D "D"
#define PORT_E "E"
#define PORT_F "F"
#define PORT_G "G"

#define PIN_INPUT "INPUT"
#define PIN_OUTPUT "OUTPUT"

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		if (rxData == '\n') {  // End of command
			rxBuffer[rxIndex] = '\0'; // Null-terminate the string
			rxIndex = 0;
			processCommand(rxBuffer);
		} else {
			if (rxIndex < RX_BUFFER_SIZE - 1) {
				rxBuffer[rxIndex++] = rxData; // Add received byte to buffer
			}
		}
		// Continue receiving next byte
		HAL_UART_Receive_IT(&huart2, &rxData, 1);
	}
}

void UBA_pin_test_init(void) {
	HAL_UART_Receive_IT(&huart2, &rxData, 1);
}

void pars_pin_output_command(char param1[32], char param2[32], char param3[32]) {
	GPIO_TypeDef *port;
	uint16_t pin = 0;
	GPIO_PinState pinState = 0;
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	UART_LOG_DEBUG(UBA_COMP, "test pin command received: %s, %s ,%s", param1, param2, param3);
	if (strcmp(param1, PORT_A) == 0) {
		port = GPIOA;
	} else if (strcmp(param1, PORT_B) == 0) {
		port = GPIOB;
	} else if (strcmp(param1, PORT_C) == 0) {
		port = GPIOC;
	} else if (strcmp(param1, PORT_D) == 0) {
		port = GPIOD;
	} else if (strcmp(param1, PORT_E) == 0) {
		port = GPIOE;
	} else if (strcmp(param1, PORT_F) == 0) {
		port = GPIOF;
	} else if (strcmp(param1, PORT_G) == 0) {
		port = GPIOG;
	} else {
		UART_LOG_ERROR(UBA_COMP, "Invalid port");
		return;
	}
	int pin_number = atoi(param2);
	pin = 1 << pin_number;
	pin_number = atoi(param3);
	if (pin_number) {
		pinState = GPIO_PIN_SET;
	} else {
		pinState = GPIO_PIN_RESET;
	}
	GPIO_InitStruct.Pin = pin;
	UART_LOG_DEBUG(UBA_COMP, "Config PORT: 0x%X Pin:0x%X Mode: 0x%x", port, GPIO_InitStruct.Pin, GPIO_InitStruct.Mode);
	UART_LOG_DEBUG(UBA_COMP, "Write PORT: 0x%X Pin:0x%X Value: 0x%x", port, pin, pinState);
	HAL_GPIO_Init(port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(port, pin, pinState);
}

void pars_pin_input_command(char param1[32], char param2[32]) {
	GPIO_TypeDef *port;
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	UART_LOG_DEBUG(UBA_COMP, "test pin command received: %s, %s", param1, param2);
	if (strcmp(param1, PORT_A) == 0) {
		port = GPIOA;
	} else if (strcmp(param1, PORT_B) == 0) {
		port = GPIOB;
	} else if (strcmp(param1, PORT_C) == 0) {
		port = GPIOC;
	} else if (strcmp(param1, PORT_D) == 0) {
		port = GPIOD;
	} else if (strcmp(param1, PORT_E) == 0) {
		port = GPIOE;
	} else if (strcmp(param1, PORT_F) == 0) {
		port = GPIOF;
	} else if (strcmp(param1, PORT_G) == 0) {
		port = GPIOG;
	} else {
		UART_LOG_ERROR(UBA_COMP, "Invalid port");
		return;
	}
	int pin_number = atoi(param2);
	GPIO_InitStruct.Pin = (1 << pin_number);
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	UART_LOG_DEBUG(UBA_COMP, "Config PORT: 0x%X Pin:0x%X Mode: 0x%x", port, GPIO_InitStruct.Pin, GPIO_InitStruct.Mode);
	HAL_GPIO_Init(port, &GPIO_InitStruct);

}

void processCommand(const char *line) {
	char cmdName[32];
	char param1[32], param2[32], param3[32];

	// Parse command
	int parsed = sscanf(line, "%[^,],%[^,],%[^,],%s\n", cmdName, param1, param2, param3);

	UART_LOG_WARNNING(UBA_COMP, "Start process Command");
	if (parsed == 4) {
		UART_LOG_DEBUG(UBA_COMP, "Command: %s", cmdName);
		UART_LOG_DEBUG(UBA_COMP, "param1: %s", param1);
		UART_LOG_DEBUG(UBA_COMP, "param2: %s", param2);
		UART_LOG_DEBUG(UBA_COMP, "param3: %s", param3);

		// Command successfully parsed
		if (strcmp(cmdName, PIN_OUTPUT_CMD) == 0) {
			// Example: Command is "SET", handle parameters
			pars_pin_output_command(param1, param2, param3);
		} else if (strcmp(cmdName, PIN_INPUT_CMD) == 0) {
			// Example: Command is "GET"
			pars_pin_input_command(param1, param2);
		} else {
			UART_LOG_ERROR(UBA_COMP, "Unknown command: %s\n", cmdName);
		}
	} else {
		// Error in parsing
		UART_LOG_ERROR(UBA_COMP, "Invalid command format");
	}
}

void UBA_test_pin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState pinState) {
	HAL_GPIO_WritePin(port, pin, pinState);
}

