/*
 * PLI74HC166.c
 *
 *  Created on: Nov 2, 2025
 *      Author: ORA
 */
#include "PLI74HC166.h"
#include "spi.h"
#include "main.h"
#include "UBA_util.h"
#define SPI_CLOCK hspi2
#define PLI74HC166_LOAD (GPIO_PIN_RESET)
#define PLI74HC166_SHIFT (GPIO_PIN_SET)

#define COMP "74HC166"

PLI74HC166 buttons_driver_g = { 0 };


void PLI74HC166_init(PLI74HC166 *driver) {
	driver->load_pin.GPIOx = BUTTON_LD_GPIO_Port;
	driver->load_pin.GPIO_Pin = BUTTON_LD_Pin;
	driver->serial_pin.GPIOx = BUTTON_SER_GPIO_Port;
	driver->serial_pin.GPIO_Pin = BUTTON_SER_Pin;
	driver->clock_pin.GPIOx = BCLK_GPIO_Port;
	driver->clock_pin.GPIO_Pin = BCLK_Pin;

}

uint8_t PLI74HC166_read(PLI74HC166 *driver) {
	uint8_t ret = 0;
	// Wait until SPI not busy
	HAL_GPIO_WritePin(driver->load_pin.GPIOx, driver->load_pin.GPIO_Pin, PLI74HC166_LOAD);
	HAL_GPIO_WritePin(driver->clock_pin.GPIOx, driver->clock_pin.GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(driver->clock_pin.GPIOx, driver->clock_pin.GPIO_Pin, GPIO_PIN_RESET);
	HAL_Delay(2);
	HAL_GPIO_WritePin(driver->load_pin.GPIOx, driver->load_pin.GPIO_Pin, PLI74HC166_SHIFT);
	for (int i = 0; i < 8; i++) {
		HAL_Delay(1);
		HAL_GPIO_WritePin(driver->clock_pin.GPIOx, driver->clock_pin.GPIO_Pin, GPIO_PIN_SET);
		HAL_Delay(1);
		GPIO_PinState pin_state = HAL_GPIO_ReadPin(driver->serial_pin.GPIOx, driver->serial_pin.GPIO_Pin);
		if (pin_state == GPIO_PIN_RESET) {
			ret |= (1u << i);
			UART_LOG_DEBUG(COMP, "at index :%d", i);
		}
		HAL_GPIO_WritePin(driver->clock_pin.GPIOx, driver->clock_pin.GPIO_Pin, GPIO_PIN_RESET);
	}
	return ret;

}
