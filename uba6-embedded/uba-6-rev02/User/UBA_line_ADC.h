/*
 * UBA_bat_voltage.h
 *
 *  Created on: Nov 6, 2024
 *      Author: ORA
 */

#ifndef UBA_LINE_ADC_H_
#define UBA_LINE_ADC_H_
#include "UBA_line.h"

#define UBA_LINE_ADC_MAX_VOLTAGE (60000)
#define UBA_LINE_ADC_VOLTAGE_WARNING_LEVEL (60000)
#define UBA_LINE_ADC_VOLTAGE_ERROR_LEVEL (60)
#define UBA_LINE_ADC_VOLTAGE_BAT_DETECTED_ERROR_LEVEL (60)


int32_t UBA_line_ADC_get_bat_voltage(UBA_line *line, UBA_LINE_ADC_RANGE adc_range);
int32_t UBA_line_ADC_get_gen_voltage(UBA_line *line);
int32_t UBA_line_ADC_get_discharge_current(UBA_line *line);
int32_t UBA_line_ADC_get_bat_charge_current(UBA_line *line);
float UBA_line_ADC_get_ambient_temperature(UBA_line *line);
float UBA_line_ADC_get_battery_temperature(UBA_line *line);

int32_t UBA_line_ADC_get_V_IN(UBA_line *line);

void UBA_line_ADC_print_reading(UBA_line *line);

#endif /* UBA_LINE_ADC_H_ */

