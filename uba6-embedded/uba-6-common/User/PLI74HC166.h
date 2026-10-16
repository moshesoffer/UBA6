/*
 * PLI74HC166.h
 *
 *  Created on: Nov 2, 2025
 *      Author: ORA
 */

#ifndef PLI74HC166_H_
#define PLI74HC166_H_
#include "gpio.h"

typedef struct _PLI74HC166 {
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} serial_pin;
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} load_pin;
	struct {
			GPIO_TypeDef *GPIOx;
			uint16_t GPIO_Pin;
		} clock_pin;

} PLI74HC166;
#endif /* PLI74HC166_H_ */

void  PLI74HC166_init(PLI74HC166 *driver);
uint8_t PLI74HC166_read(PLI74HC166 *driver);


extern PLI74HC166 buttons_driver_g;
