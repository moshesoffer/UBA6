/*
 * UBA_bat_voltage.c
 *
 *  Created on: Nov 6, 2024
 *      Author: ORA
 */

#include <UBA_line_ADC.h>
#include "uart_log.h"
#include "string.h"
#include "UBA_util.h"
#include "dac.h"
#include "adc.h"
#include "stm32g4xx_ll_adc.h"
#include "UBA_line.h"
#include "TPL0102.h"
#include "math.h"

#define UBA_COMP 		"UBA ADC"

#if (UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_INFO)
#define UART_LOG_LINE_INFO(...) UART_LOG_INFO(line->name,##__VA_ARGS__)
#else
	#define UART_LOG_LINE_INFO(...)
#endif

#if UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_DEBUG
#define UART_LOG_LINE_DEBUG(...)  UART_LOG_DEBUG(line->name,##__VA_ARGS__)
#else
#define UART_LOG_LINE_DEBUG(...)
#endif

#if UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_WARNNING
#define UART_LOG_LINE_WARN(...)  UART_LOG_WARNNING(line->name,##__VA_ARGS__)
#else
#define UART_LOG_LINE_WARN(...)
#endif

#if UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_ERROR
#define UART_LOG_LINE_ERROR(...)  UART_LOG_ERROR(line->name,##__VA_ARGS__)
#else
#define UART_LOG_LINE_ERROR(...)
#endif

#if UBA_LOG_LEVEL_LINE <= UART_LOG_LEVEL_CRITICAL
#define UART_LOG_LINE_CRITICAL(...)  UART_LOG_CRITICAL(line->name,##__VA_ARGS__)
#else
#define UART_LOG_LINE_CRITICAL(...)
#endif
#define UBA_LINE_ONE_RANGE 0xFA /*10.16K*/
#define UBA_LINE_ONE_RANGE_RES (2340)

#define UBA_LINE_RANG10V (0xC8) /*22.88 KΩ*/
#define UBA_LINE_RANG30V (0xF5) /*4.3 KΩ*/
#define UBA_LINE_RANG60V (0xFE) /*0.78 KΩ*/
#define UBA_LINE_RANGE_SAFE (0xFF) /*0.3 KΩ*/

#define UBA_LINE_ADC_VBAT_RANGE_10 (10000) /*10000 mV*/
#define UBA_LINE_ADC_VBAT_RANGE_30 (30000) /*30000 mV*/
#define UBA_LINE_ADC_VBAT_RANGE_60 (60000) /*60000 mV*/

/* R1 resistance */
#define NTC_UP_R 10000.0f

/* constants of Steinhart-Hart equation Amb NTC*/
#define A 		0.00335401643468053f
#define B 		0.00025652355089612f
#define C 		0.00000260597012072f
#define D 		0.00000006329261264f

/* constants of Steinhart-Hart equation Bat NTC*/
#define A_BAT 	0.00335401643468053f
#define B_BAT 	0.00030013082511566f
#define C_BAT 	0.00000508516494379f
#define D_BAT 	0.00000021876504925f

#define R88_R101 (10) /*10 mΩ*/
#define UBA_LINE_DISCHARGE_CURRENT_GAIN (52)

#define ADC_LINE_ADC_VBAT2VBAT(adc_vbat,R28_R60,R23_R55,resistance) ((adc_vbat * (resistance + R28_R60 + R23_R55)) / (resistance + R28_R60))
#define UBA_LINE_DAC_MV2ADC(value) ((value*0xfff)/5000)
#define UBA_VERF_MV (3300)
#define UBA_LINE_ADC_RES (0x1000)
#define UBA_LINE_ADC_ADC2MV(value) ((value*UBA_VERF_MV)/UBA_LINE_ADC_RES)

void UBA_line_ADC_print_reading(UBA_line *line) {
	uint8_t i = ADC_CHNNEL_VBAT;
	UART_LOG_LINE_DEBUG("ADC[VBAT]    :%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	i = ADC_CHNNEL_VGEN;
	UART_LOG_LINE_DEBUG("ADC[VGEN]    :%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	i = ADC_CHNNEL_AMB_TEMP;
	UART_LOG_LINE_DEBUG("ADC[AMB_TEMP]:%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	i = ADC_CHNNEL_NTC_BAT;
	UART_LOG_LINE_DEBUG("ADC[NTC_BAT] :%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	i = ADC_CHNNEL_DSCH_CURR;
	UART_LOG_LINE_DEBUG("ADC[DSCH]    :%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	if (line->ADC_DMA_SIZE > ADC_CHNNEL_VPS) {
		i = ADC_CHNNEL_VPS;
		UART_LOG_LINE_DEBUG("ADC[VPS]     :%05u voltage:%05u mV", line->ADC_raw_data[i], UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[i]));
	}
}

float UBA_line_calc_calibrate(UBA_line *line, uint16_t adc_voltage, liner_equation *le) {
	float ret = 0.0f;

	if (line->calibration.isCalibrated) {
		ret = (le->slop * adc_voltage + le->y_intercept);
	} else {
		ret = (int32_t) adc_voltage;
	}
	UART_LOG_LINE_DEBUG("Is Calibrated :%s - Calculate Line with liner Eq:% lu[mV] (%f,%f):%lu[mV]",line->calibration.isCalibrated ? "True" : "False",adc_voltage, le->slop,le->y_intercept,(uint32_t)ret);
	return ret;
}

float UBA_line_calc_calibrate_float(UBA_line *line, float value, liner_equation *le) {
	float ret = 0.0f;
	if (line->calibration.isCalibrated) {
		ret = (le->slop * value + le->y_intercept);
	} else {
		ret = value;
	}
	UART_LOG_LINE_DEBUG("Is Calibrated :%s - Calculate Line with liner Eq:% lu[mV] (%f,%f):%lf ",line->calibration.isCalibrated ? "True" : "False",value, le->slop,le->y_intercept,ret);
	return ret;
}
/*set the rage and measure the VBAT at that range */
/**
 *
 * @param line - the line pointer
 * @param adc_range - the range tp mesure
 * @return a
 */
int32_t UBA_line_ADC_get_bat_voltage(UBA_line *line, UBA_LINE_ADC_RANGE adc_range) {
	UBA_LINE_ADC_RANGE new_range;
	uint32_t adc_voltage = 0;
	uint32_t vbat = 0;
	HAL_StatusTypeDef hal_ret;

	if (line->digital_potentiometer.WRB != line->adc_step_value[adc_range]) {
		tpl010_set_potentiometer_b(&line->digital_potentiometer, line->adc_step_value[adc_range]);
		HAL_Delay(TPL0102_T_WRT);
		HAL_Delay(10); // stable the line voltage
		hal_ret = HAL_ADC_Start_DMA(line->adc_handle, (uint32_t*) &line->ADC_raw_data[0], line->ADC_DMA_SIZE);
		HAL_Delay(1);
		if (hal_ret != HAL_OK) {
			UART_LOG_LINE_CRITICAL("ADC Query Failed %x", hal_ret);
		}
	}

	adc_voltage = UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[ADC_CHNNEL_VBAT]);
	UART_LOG_LINE_DEBUG("ADC resistance(pb) is %u adc_voltage : %d", line->adc_resistance[adc_range], adc_voltage);
	vbat = (uint32_t) UBA_line_calc_calibrate(line,
			ADC_LINE_ADC_VBAT2VBAT(adc_voltage, line->R28_R60, line->R23_R55, line->adc_resistance[adc_range]), &line->calibration.vbat[adc_range]);
	if (vbat <= UBA_LINE_ADC_VBAT_RANGE_10) /*vbat 0-10*/{
		new_range = UBA_LINE_ADC_RANGE10V;
	} else if (vbat <= UBA_LINE_ADC_VBAT_RANGE_30) /*vbat 10-30*/{
		new_range = UBA_LINE_ADC_RANGE30V;
	} else if (vbat <= UBA_LINE_ADC_VBAT_RANGE_60) /*vbat 30-60*/{
		new_range = UBA_LINE_ADC_RANGE60V;
	} else {
		UART_LOG_LINE_CRITICAL("Battery Voltage is OOB");
		tpl010_set_potentiometer_b(&line->digital_potentiometer, line->adc_step_value[UBA_LINE_ADC_RANGE_SAFETY]);
		return UBA_LINE_ADC_MAX_VOLTAGE;
	}
	if (new_range >= adc_range) {
		/*if the range is the same store the vbat*/
		tpl010_set_potentiometer_b(&line->digital_potentiometer, line->adc_step_value[UBA_LINE_ADC_RANGE60V]);
		return vbat;
	} else {
		return UBA_line_ADC_get_bat_voltage(line, new_range);
	}
}

int32_t UBA_line_ADC_get_gen_voltage(UBA_line *line) {
	uint32_t adc_voltage = 0;
	uint32_t v_gen = 0;
	HAL_StatusTypeDef hal_ret;
	adc_voltage = UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[ADC_CHNNEL_VGEN]);
	v_gen = (uint32_t) UBA_line_calc_calibrate(line, (adc_voltage * 59) / 3,
			&line->calibration.vgen);

	tpl010_set_potentiometer_a(&line->digital_potentiometer, line->adc_step_value[UBA_LINE_ADC_RANGE60V]);

	return v_gen;
}

int32_t UBA_line_ADC_get_discharge_current(UBA_line *line) {
	int32_t ADC_DC_voltage = UBA_LINE_ADC_ADC2MV(line->ADC_raw_data[ADC_CHNNEL_DSCH_CURR]);
	return UBA_line_calc_calibrate(line, ((ADC_DC_voltage * 100) / UBA_LINE_DISCHARGE_CURRENT_GAIN), &line->calibration.discharge_current);
}

int32_t UBA_line_ADC_get_bat_charge_current(UBA_line *line) {
	return (int32_t) (UBA_line_calc_calibrate(line, (((int32_t) line->EX_ADC_raw_data) * 10000 / 0x1000), &line->calibration.charge_current));
}

float UBA_line_ADC_get_ambient_temperature(UBA_line *line) {
	float Ntc_Ln = 0;
	float Ntc_Tmp;
	uint16_t Ntc_R;
	if (line->ADC_raw_data[ADC_CHNNEL_AMB_TEMP] > 4000) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_INTRENAL_TEMP_SENSOR_NC)) {
			UART_LOG_LINE_ERROR("ADC Value: %d --> internal Temp sensor Disconnected", line->ADC_raw_data[ADC_CHNNEL_AMB_TEMP]);
		}
		return -273.15;
	} else {
		/* calc. ntc resistance */
		Ntc_R = ((NTC_UP_R) / ((4095.0 / line->ADC_raw_data[ADC_CHNNEL_AMB_TEMP]) - 1));
		/* temp */
		Ntc_Ln = log(Ntc_R / NTC_UP_R);
		/* calc. temperature */
		Ntc_Tmp = (1.0 / (A + B * Ntc_Ln + C * Ntc_Ln * Ntc_Ln + D * Ntc_Ln * Ntc_Ln * Ntc_Ln)) - 273.15;
		return UBA_line_calc_calibrate_float(line, Ntc_Tmp, &line->calibration.amb_temp);
	}
}

float UBA_line_ADC_get_battery_temperature(UBA_line *line) {
	float Ntc_Ln = 0;
	float Ntc_Tmp;
	uint16_t Ntc_R;
	if (line->ADC_raw_data[ADC_CHNNEL_NTC_BAT] > 4000) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_TEMP_SENSOR_NC)) {
			UART_LOG_LINE_ERROR("ADC Value: %d --> Bat Temp sensor Disconnected", line->ADC_raw_data[ADC_CHNNEL_NTC_BAT]);
		}
		return -273.15;
	} else {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_TEMP_SENSOR_NC);
		/* calc. ntc resistance */
		Ntc_R = ((NTC_UP_R) / ((4095.0 / line->ADC_raw_data[ADC_CHNNEL_NTC_BAT]) - 1));

		/* temp */
		Ntc_Ln = log(Ntc_R / NTC_UP_R);
		/* calc. temperature */
		Ntc_Tmp = (1.0 / (A_BAT + B_BAT * Ntc_Ln + C_BAT * Ntc_Ln * Ntc_Ln + D_BAT * Ntc_Ln * Ntc_Ln * Ntc_Ln)) - 273.15;
		return UBA_line_calc_calibrate_float(line, Ntc_Tmp, &line->calibration.ntc_temp);
	}
}

int32_t UBA_line_ADC_get_V_IN(UBA_line *line) {
	uint16_t vin = 0, vin_ADC = 0;
	int32_t in_voltage = -1;
	if (line->ADC_DMA_SIZE > ADC_CHNNEL_VPS) {
		vin = line->ADC_raw_data[ADC_CHNNEL_VPS];
		vin_ADC = UBA_LINE_ADC_ADC2MV(vin);
		in_voltage = UBA_line_calc_calibrate(line, ADC_LINE_ADC_VBAT2VBAT(vin_ADC, 3000, 56000, 0), &line->calibration.vps);
	}
	return in_voltage;
}

