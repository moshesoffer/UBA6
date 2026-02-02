/*
 * UBA_line.c
 *
 *  Created on: Oct 8, 2024
 *      Author: ORA
 */

#include "UBA_line.h"
#include <UBA_line_ADC.h>
#include "uart_log.h"
#include "string.h"
#include "UBA_util.h"
#include "dac.h"
#include "stm32g4xx_ll_adc.h"
#include "stdlib.h"
#include "UBA_UART_comm.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include "UBA_PROTO_CALIBRATION.pb.h"
#include "file_logger.h"
UBA_line UBA_LINE_A = UBA_INTRENAL_LINE_DEFUALT;
UBA_line UBA_LINE_B = UBA_INTRENAL_LINE_DEFUALT;

/**
 * @brief bit flag for delta values
 *
 */
typedef enum UBA_LINE_CHARGE_DELTA {

	UBA_LINE_CHARGE_DELTA_INVALID = 0x00, /**< UBA_LINE_CHARGE_DELTA_INVALID */
	UBA_LINE_CHARGE_DELTA_ON_GAP = 0x01, /*the voltage is in the charge gap (VGen > VBat + 0.5) *//**< UBA_LINE_CHARGE_DELTA_ON_GAP */
	UBA_LINE_CHARGE_DELTA_ON_TARGET = 0x02 | UBA_LINE_CHARGE_DELTA_ON_GAP, /**< UBA_LINE_CHARGE_DELTA_ON_TARGET */
	UBA_LINE_CHARGE_DELTA_CV = 0x04 | UBA_LINE_CHARGE_DELTA_ON_TARGET | UBA_LINE_CHARGE_DELTA_ON_GAP, /*in small gap to lower current in CV state*/
	UBA_LINE_CHARGE_DELTA_HIGH = 0x08, /**< UBA_LINE_CHARGE_DELTA_HIGH */
	UBA_LINE_CHARGE_DELTA_LOW = 0x10, /**< UBA_LINE_CHARGE_DELTA_LOW */
} UBA_LINE_CHARGE_DELTA;

#define UBA_COMP 		"LINE"
#define PRINT_A2D 		true

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

#define UBA_LINE_DAC_MV2ADC(value) ((value*0xfff)/5000)
#define UBA_VERF_MV (3300)

#define UBA_LINE_VOLTAGE_MAX (60000)
#define UBA_LINE_VOLTAGE_BAT_MIN (600)
#define UBA_LINE_VOLTAGE_BAT_DISCONNECTED (0)
#define UBA_LINE_VOLTAGE_BAT_MAX (0xffff) /*the max value to store into the bat*/

#define UBA_LINE_DSCH_CURRNT_SHORT (200) /*200mA Discharge current > 0.2A while discharge is disabled */
#define UBA_LINE_CHARGE_CURRNT_SHORT (200) /*200mA Discharge current > 0.2A while discharge is disabled */
#define UBA_LINE_OVERCURRENT_DELTA (500) /*500mA current is overCurrent */

#define UBA_LINE_CURRENT_MAX (5000) /*5A max current*/
#define UBA_LIME_TEMP_MAX	(500.0f)/*MAX Temperature */

#define UBA_LINE_MAX_BAT_TEMP (70.0f)
#define UBA_LINE_MAX_AMB_TEMP (80.0f)
#define UBA_LINE_CLEAR_MAX_BAT_TEMP_ERROR (40.0f) /*Below 40 c clear the error*/

#define UBA_LINE_FULL_SACLE_DISC_VALUE (6000) /*full scale DAC Value TODO: mesure this value*/
#define UBA_LINE_DSCH_CURR_GAIN (52)

#define UBA_LINE_DSCH_DAC_RES (0.633f) /*0.633* mA == DAC value (1 DAC bit is 1.579mA)   new_dac = (discharge_current * 51 * 4096) / (100 * 3300);*/
#define UBA_LINE_DSCH_CURRENT_HYST_MA (2) /*1 bits in DAC (1/0.633 == 1.57 --->2 ) the DAC בשמ Flicker a single byte == 2 mA noise*/
#define UBA_LINE_DSCH_DAC_MA2DAC(current) (current * UBA_LINE_DSCH_DAC_RES)
#define UBA_LINE_DSCH_MAX_CURRENT (UBA_LINE_CURRENT_MAX) // line max Current is 5A

#define UBA_LINE_CHARGE_DAC_RES (0.620f) /*0.620* mA == DAC value (1 DAC bit is 1.611mA)   new_dac = (charge_current * 50 * 4096) / (100 * 3300);*/
#define UBA_LINE_CHARGE_CURRENT_HYST_MA (2) /*1 bits in DAC (1/0.620 == 1.611 --->2 ) the DAC בשמ Flicker a single byte == 2 mA noise*/
#define UBA_LINE_CHARGE_CURRENT_HYST_FACTOR (25)
#define UBA_LINE_CHARGE_CURRENT_HYST_DIV (10)
#define UBA_LINE_CHARGE_CURRENT_HYST_MA_FACTOR (UBA_LINE_CHARGE_CURRENT_HYST_MA * UBA_LINE_CHARGE_CURRENT_HYST_FACTOR)
#define UBA_LINE_CHARGE_CURRENT_HYST_RAW ((int) (UBA_LINE_CHARGE_CURRENT_HYST_MA_FACTOR / UBA_LINE_CHARGE_DAC_RES ))
#define UBA_LINE_CHARGE_DAC_MA2DAC(current) (current * UBA_LINE_CHARGE_DAC_RES)
#define UBA_LINE_CHARGE_DAC_MAX_VALUE (0xfff)
#define UBA_LINE_CHARGE_MAX_POWER (150*1000*1000) /*Max power Allow in mV*mA*/

#define UBA_LINE_DATA_READ_TIME_MS (10)
#define UBA_LINE_PRINT_DATA_READ_TIME_MS (5000)
#define UBA_LINE_PRE_CHARGE_DATA_READ_TIME_MS (10)

#define UBA_LINE_DSCH_DAC_MIN_VALUE (0)  /*was 50 change to 0 for debug TODO: set after Tomer mesure */
#define UBA_LINE_DSCH_DAC_MAX_VALUE (UBA_LINE_DSCH_DAC_MA2DAC(UBA_LINE_DSCH_MAX_CURRENT))

#define UBA_LINE_CHARGE_DAC_MIN_VALUE (0)  /*was 50 change to 0 for debug TODO: set after Tomer mesure */
#define UBA_LINE_CHARGE_DAC_MAX_VALUE (0xfff)

#define UBA_LINE_CHARGE_GAP_MIN (500) /*0.5V charge gap*/
#define UBA_LINE_CHARGE_GAP_MAX (UBA_LINE_CHARGE_GAP_MIN*2) /* 2* charge gap*/
#define UBA_LINE_CHARGE_HYSTERESIS (50) /*0.5V charge gap*/

#define UBA_LINE_DEAD_WAKEUP_TIME (600000) /*10 min*/

#define UBA_LINE_EX_DAC_DELTA_RES (2048) /*2^11*/

#define UBA_LINE_MIN_INPUT_VOLTAGE (9000) /*9V*/
#define UBA_LINE_MAX_INPUT_VOLTAGE (48000) /*48V*/

#define UBA_LINE_COPY_LINEAR(le, lec) \
    do { \
        (le)->slop = (lec)->slop; \
        (le)->y_intercept = (lec)->y_intercept; \
    } while(0)

#define COPY_CALIB_PARAM(param) UBA_LINE_COPY_LINEAR(&(line->calibration.param), &(msg->param))
#define UBA_LINE_DISCHARGE_STEP_VALUE (1)

#define CHARGE_ENABLE (true)
#define CHARGE_DISABLE (false)
#define DISCHARGE_ENABLE (true)
#define DISCHARGE_DISABLE (false)
#define ENABLE (true)
#define DISABLE (false)

//#define VIRTUAL_BAT 1
//#define AUTO_QUERY 1
//==========================================================state machine private functions==============================

void UBA_line_init_enter(UBA_line *line);
void UBA_line_init(UBA_line *line);
void UBA_line_init_exit(UBA_line *line);

void UBA_line_idle_enter(UBA_line *line);
void UBA_line_idle(UBA_line *line);
void UBA_line_idle_exit(UBA_line *line);

void UBA_line_pre_charging_enter(UBA_line *line);
void UBA_line_pre_charging(UBA_line *line);
void UBA_line_pre_charging_exit(UBA_line *line);

void UBA_line_charging_cc_enter(UBA_line *line);
void UBA_line_charging_cc(UBA_line *line);
void UBA_line_charging_cc_exit(UBA_line *line);

void UBA_line_charging_cv_enter(UBA_line *line);
void UBA_line_charging_cv(UBA_line *line);
void UBA_line_charging_cv_exit(UBA_line *line);

void UBA_line_discharging_enter(UBA_line *line);
void UBA_line_discharging(UBA_line *line);
void UBA_line_discharging_exit(UBA_line *line);

void UBA_line_dead_enter(UBA_line *line);
void UBA_line_dead(UBA_line *line);
void UBA_line_dead_exit(UBA_line *line);

typedef void (*step_cb_t)(UBA_line *line);

/***
 * UBA Single Channel State Machine Assigner Rule
 */
struct UBASCSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
	char *name;
};

/*UBA Single Channel State Machine Assigner */
#define UBASCSMA(step, cbe, cbr, cbx)[UBA_LINE_STATE_##step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx, .name = #step}

// @formatter:off
static const struct UBASCSMA_rule rule_g[UBA_LINE_STATE_MAX] ={
		UBASCSMA(INIT,			UBA_line_init_enter,			UBA_line_init,			UBA_line_init_exit),
		UBASCSMA(IDLE,			UBA_line_idle_enter,			UBA_line_idle,			UBA_line_idle_exit),
		UBASCSMA(PRE_CHARGING,	UBA_line_pre_charging_enter,	UBA_line_pre_charging,	UBA_line_pre_charging_exit),
		UBASCSMA(CHARGING_CC,	UBA_line_charging_cc_enter,		UBA_line_charging_cc,	UBA_line_charging_cc_exit),
		UBASCSMA(CHARGING_CV,	UBA_line_charging_cv_enter,		UBA_line_charging_cv,	UBA_line_charging_cv_exit),
		UBASCSMA(DISCHARGING,	UBA_line_discharging_enter,		UBA_line_discharging,	UBA_line_discharging_exit),
		UBASCSMA(DEAD,			UBA_line_dead_enter,			UBA_line_dead,			UBA_line_dead_exit),

};
// @formatter:on

//=================================================private functions========================================================//
void UBA_line_enable_charge(UBA_line *line, bool isEnable);
void UBA_line_enable_discharge(UBA_line *line, bool isEnable);
void UBA_line_set_voltage(UBA_line *line, int32_t voltage);
bool UBA_line_isBattery_connected(UBA_line *line);
void UBA_line_print_reading(UBA_line *line);

bool UBA_line_BIST(UBA_line *line) {
	bool ret = false;
	UBA_line_enable_charge(line, CHARGE_DISABLE);
	UBA_line_get_line_data(line);
	if (UBA_line_isBattery_connected(line)) {
		UART_LOG_LINE_ERROR("Cannot Perform BIST Battery is connected");
		return ret;
	}
	for (int i = 0; i < 20; i++) {
		UBA_line_get_line_data(line);
		UBA_line_print_reading(line);
		UBA_line_increase_charge_voltage(line, 1);
		UBA_DCDC_run(&line->buck_boost);
	}
	UBA_line_enable_charge(line, CHARGE_ENABLE);
	HAL_Delay(20);
	UBA_line_get_line_data(line);
	UBA_line_print_reading(line);
	if (UBA_IN_RANGE_HYST(line->data.gen_voltage,line->data.voltage ,500) == false) {
		UART_LOG_LINE_ERROR("Vgen:%04d is not on Bat Line:%04d", line->data.gen_voltage, line->data.voltage);
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_BIST_FALIED)) {
			UART_LOG_LINE_CRITICAL("BIST Failed: Vgen:%04d is not on Bat Line:%04d", line->data.gen_voltage, line->data.voltage);
		}
	} else {
		UART_LOG_LINE_INFO("Vgen:%04d on Bat Line:%04d", line->data.gen_voltage, line->data.voltage);
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BIST_FALIED);
		ret = true;
	}
	UBA_line_enable_charge(line, CHARGE_DISABLE);
	return ret;
}

void UBA_line_enable_charge(UBA_line *line, bool isEnable) {
	bool pin_state = HAL_GPIO_ReadPin(line->charge_enable.GPIOx, line->charge_enable.GPIO_Pin) == GPIO_PIN_SET ? CHARGE_ENABLE : CHARGE_DISABLE;
	if (pin_state ^ isEnable) {
		UART_LOG_LINE_WARN("==================================CHRAGE %s!!!======================================", isEnable ? "ENABLE" : "DISABLE");
		HAL_GPIO_WritePin(line->charge_enable.GPIOx, line->charge_enable.GPIO_Pin, isEnable ? GPIO_PIN_SET : GPIO_PIN_RESET);
	}
}
void UBA_line_enable_discharge(UBA_line *line, bool isEnable) {
	bool pin_state =
			HAL_GPIO_ReadPin(line->discharge_enable.GPIOx, line->discharge_enable.GPIO_Pin) == GPIO_PIN_SET ? DISCHARGE_ENABLE : DISCHARGE_DISABLE;
	if (pin_state ^ isEnable) {
		UART_LOG_LINE_WARN("=================================DISCHRAGE %s!!!====================================", isEnable ? "ENABLE" : "DISABLE");
		HAL_GPIO_WritePin(line->discharge_enable.GPIOx, line->discharge_enable.GPIO_Pin, isEnable ? GPIO_PIN_SET : GPIO_PIN_RESET);/*disable the charge in pre charge*/
	}
}

bool UBA_line_isBattery_connected(UBA_line *line) {
	if (UBA_line_isCharging(line)) {
		if ((line->data.charge_current > 10) && (line->data.voltage > UBA_LINE_VOLTAGE_BAT_MIN)) {
			return true;
		} else {
			// charge current is to low must disconnected charge and test again
			UBA_line_enable_charge(line, CHARGE_DISABLE);
			HAL_Delay(20);
			UBA_line_set_voltage(line, UBA_line_ADC_get_bat_voltage(line, UBA_LINE_ADC_RANGE60V)); //get the voltage and set it to the channel use DMA
			UBA_line_enable_charge(line, CHARGE_ENABLE);
		}
	}
	return line->data.voltage > UBA_LINE_VOLTAGE_BAT_MIN;
}

void UBA_line_update_state(UBA_line *line) {
	if (line->state.current < UBA_LINE_STATE_MAX && line->state.next < UBA_LINE_STATE_MAX) {
		UART_LOG_INFO(line->name, "update state %s ---> %s", rule_g[line->state.current].name, rule_g[line->state.next].name);
	} else {
		UART_LOG_INFO(line->name, "update state %u ---> %u", line->state.current, line->state.next);
	}
	line->state.pre = line->state.current;
	line->state.current = line->state.next;
	line->state.next = UBA_LINE_STATE_INVALID;
	line->state.tick = HAL_GetTick();
}

void UBA_line_print_reading(UBA_line *line) {
	UART_LOG_LINE_INFO("VIN:%05d mV VBat:%05dmV VGen:%05dmV CC:%04dmA DC:%04dmA BAT Temp:%+07.2fC ABN Temp:%+07.2fC",
			line->data.vps,
			line->data.voltage,
			line->data.gen_voltage,
			line->data.charge_current,
			line->data.discharge_current,
			line->data.bat_temperature,
			line->data.amb_temperature
			);
	#ifdef PRINT_A2D
	UART_LOG_LINE_DEBUG("VIN:%04d     VBat:%04u    VGen:%04u  CC:%04u    DC:%04u    BAT Temp:%04u    ABN Temp:%04u",
			line->ADC_raw_data[ADC_CHNNEL_VPS], line->ADC_raw_data[ADC_CHNNEL_VBAT], line->ADC_raw_data[ADC_CHNNEL_VGEN], line->EX_ADC_raw_data,
			line->ADC_raw_data[ADC_CHNNEL_DSCH_CURR], line->ADC_raw_data[ADC_CHNNEL_NTC_BAT], line->ADC_raw_data[ADC_CHNNEL_AMB_TEMP]
	);
#endif
}

void UBA_line_read_line_values_DMA_start(UBA_line *line) {
	HAL_ADC_Start_DMA(line->adc_handle, (uint32_t*) &line->ADC_raw_data[0], line->ADC_DMA_SIZE);
}

void UBA_line_set_voltage(UBA_line *line, int32_t voltage) {
#ifdef VIRTUAL_BAT
	line->data.voltage = line->target.voltage;
	line->isBattery_connected = true;
	return;
#endif
	if (HAL_GPIO_ReadPin(line->bat_negtive.GPIOx, line->bat_negtive.GPIO_Pin) == GPIO_PIN_SET) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_REVERSE_POLARITY)) {
			UART_LOG_LINE_ERROR("Battery is Inverse");
		}
		line->data.voltage = 0; // value of the voltage is N/A
		return;
	} else if (line->error & UBA_PROTO_UBA6_ERROR_LINE_REVERSE_POLARITY) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_REVERSE_POLARITY);
	}
	if (voltage > UBA_LINE_VOLTAGE_MAX) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE)) {
			UART_LOG_LINE_ERROR("Battery High Voltage");
		}
		line->isBattery_connected = true;
	} else if (voltage < UBA_LINE_VOLTAGE_BAT_MIN) {
		if (line->data.voltage > UBA_LINE_VOLTAGE_BAT_MIN) {
			UART_LOG_LINE_INFO("Battery disconnected : %d --> %d", line->data.voltage, voltage);
			line->isBattery_connected = false;
		}
		if (line->error & UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE) {
			UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE);
		}
	} else {
		if (line->data.voltage < UBA_LINE_VOLTAGE_BAT_MIN) {
			line->isBattery_connected = true;
			UART_LOG_LINE_INFO("Battery connected : %d --> %d", line->data.voltage, voltage);
		}
		if (line->error & UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE) {
			UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE);
		}
	}
	/*if (HAL_GPIO_ReadPin(line->charge_enable.GPIOx, line->charge_enable.GPIO_Pin) == GPIO_PIN_SET) {
	 if ((voltage > line->target.voltage) && (line->target.voltage > 0)) {
	 if (UBA_line_post_error(line, UBA_LINE_ERROR_OVER_VOLTAGE)) {
	 UART_LOG_LINE_ERROR("Over Voltage Vbat=%05d , target=%05d, charging is disabled", voltage, line->target.voltage);
	 }
	 } else if (line->error & UBA_LINE_ERROR_OVER_VOLTAGE) {
	 UBA_line_clear_error(line, UBA_LINE_ERROR_OVER_VOLTAGE);
	 }
	 }*/

	if (voltage < UBA_LINE_VOLTAGE_BAT_MIN) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED)) {
			UART_LOG_LINE_ERROR("Battery not connected (%04d)", voltage);
		}
	} else {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED);
	}

	line->data.voltage = voltage;

}

void UBA_line_set_gen_voltage(UBA_line *line, int32_t voltage) {
	if (voltage > UBA_LINE_VOLTAGE_MAX) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_GEN_HIGH_VOLTAGE)) {
			UART_LOG_LINE_ERROR("Vgen Max Voltage", voltage);
		}
	}
	line->data.gen_voltage = voltage;
}

void UBA_line_set_discharge_current(UBA_line *line, int32_t current) {
	if (current >= UBA_LINE_CURRENT_MAX) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT)) {
			UART_LOG_LINE_ERROR("Discharge Max Current");
		}
	} else {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT);
	}
	if ((HAL_GPIO_ReadPin(line->discharge_enable.GPIOx, line->discharge_enable.GPIO_Pin) == GPIO_PIN_RESET)
			&& (current > UBA_LINE_DSCH_CURRNT_SHORT)) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_DISCHARGE_MOSFET_SHORT)) {
			UART_LOG_LINE_ERROR("FET Short fail disconnect BAT immediately");
		}
	} else if (line->error & UBA_PROTO_UBA6_ERROR_LINE_DISCHARGE_MOSFET_SHORT) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_DISCHARGE_MOSFET_SHORT);
	}

	if (current > line->target.current + UBA_LINE_OVERCURRENT_DELTA) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT)) {
			UART_LOG_LINE_ERROR("Over current fail discharge");
		}
	} else if (line->error & UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT);
	}

	line->data.discharge_current = current;
}

void UBA_line_set_charge_current(UBA_line *line, int32_t current) {
	if (current >= UBA_LINE_CURRENT_MAX) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT)) {
			UART_LOG_LINE_ERROR("Charge Max Current");
		}
	} else {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT);
	}
	if ((HAL_GPIO_ReadPin(line->charge_enable.GPIOx, line->charge_enable.GPIO_Pin) == GPIO_PIN_RESET)
			&& (current > UBA_LINE_CHARGE_CURRNT_SHORT)) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_CHARGE_MOSFET_SHORT)) {
			UART_LOG_LINE_ERROR("FET Short fail disconnect BAT immediately");
		}
	} else if (line->error & UBA_PROTO_UBA6_ERROR_LINE_CHARGE_MOSFET_SHORT) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_CHARGE_MOSFET_SHORT);
	}

	if (current > line->target.current + UBA_LINE_OVERCURRENT_DELTA) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT)) {
			UART_LOG_LINE_ERROR("Over current fail charge");
		}
	} else if (line->error & UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT);
	}

	line->data.charge_current = current;
}

void UBA_line_set_bat_temp(UBA_line *line, float temp) {
	/*if (temp > UBA_LINE_MAX_BAT_TEMP) {
	 if(UBA_line_post_error(line, UBA_LINE_ERROR_BAT_HIGH_TEMP)){
	 UART_LOG_LINE_ERROR("Battery Critical over temperature (%05.2f)", temp);
	 }
	 } else if ((line->error & UBA_LINE_ERROR_BAT_HIGH_TEMP) == UBA_LINE_ERROR_BAT_HIGH_TEMP) {
	 if (line->data.bat_temperature <= UBA_LINE_CLEAR_MAX_BAT_TEMP_ERROR) {
	 UART_LOG_LINE_INFO("Cooled Down Clear Error ", temp);
	 } else {
	 if(UBA_line_clear_error(line, UBA_LINE_ERROR_BAT_HIGH_TEMP)){
	 UART_LOG_LINE_ERROR("Battery Critical over temperature (%05.2f) - Cooling", line->data.bat_temperature);
	 }
	 }
	 }*/
	line->data.bat_temperature = temp;
}

void UBA_line_set_amb_temp(UBA_line *line, float temp) {

	if (temp > UBA_LINE_MAX_AMB_TEMP) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_AMB_HIGH_TEMP)) {
			UART_LOG_LINE_WARN("Warning: High internal temperature (%05.2f)", temp);
		}
	} else if ((line->error & UBA_PROTO_UBA6_ERROR_LINE_AMB_HIGH_TEMP) == UBA_PROTO_UBA6_ERROR_LINE_AMB_HIGH_TEMP) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_AMB_HIGH_TEMP);
	}

	line->data.amb_temperature = temp;
}

void UBA_line_set_input_voltage(UBA_line *line, int32_t voltage) {
	if (voltage < UBA_LINE_MIN_INPUT_VOLTAGE) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE)) {
			UART_LOG_LINE_ERROR("Input fail Low Voltage (%04d)", voltage);
		}
	} else if (voltage > UBA_LINE_MAX_INPUT_VOLTAGE) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_HIGH_INPUT_VOLTAGE)) {
			UART_LOG_LINE_ERROR("Input fail High Voltage (%04d)", voltage);
		}
	} else if ((line->error & UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE) == UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE);
	} else if ((line->error & UBA_PROTO_UBA6_ERROR_LINE_HIGH_INPUT_VOLTAGE) == UBA_PROTO_UBA6_ERROR_LINE_HIGH_INPUT_VOLTAGE) {
		UBA_line_clear_error(line, UBA_PROTO_UBA6_ERROR_LINE_HIGH_INPUT_VOLTAGE);
	}
	line->data.vps = voltage;
}

void UBA_line_get_line_data(UBA_line *line) {
	uint32_t sum = 0;
	HAL_ADC_Start_DMA(line->adc_handle, (uint32_t*) &line->ADC_raw_data[0], line->ADC_DMA_SIZE);
	for (uint8_t i = 0; i < 16; i++) {
		if (MCP3221_read(&line->EX_ADC, &line->EX_ADC_raw_data) != MCP3221_STATUS_OK) {
			UART_LOG_LINE_CRITICAL("MCP3221 read Failed ");
		}
		sum += line->EX_ADC_raw_data;
	}
	line->EX_ADC_raw_data = sum / 16;
	HAL_Delay(10);
	UBA_line_ADC_print_reading(line);
	UBA_line_set_voltage(line, UBA_line_ADC_get_bat_voltage(line, UBA_LINE_ADC_RANGE60V)); //get the voltage and set it to the channel
	UBA_line_set_gen_voltage(line, UBA_line_ADC_get_gen_voltage(line)); //get the voltage and set it to the channel
	UBA_line_set_discharge_current(line, UBA_line_ADC_get_discharge_current(line));
	UBA_line_set_charge_current(line, UBA_line_ADC_get_bat_charge_current(line));
	UBA_line_set_bat_temp(line, UBA_line_ADC_get_battery_temperature(line));
	UBA_line_set_amb_temp(line, UBA_line_ADC_get_ambient_temperature(line));
	UBA_line_set_input_voltage(line, UBA_line_ADC_get_V_IN(line));

	if (HAL_GetTick() - (line->data_log_print_tick) > UBA_LINE_PRINT_DATA_READ_TIME_MS) {
		line->data_log_print_tick = HAL_GetTick();
		UBA_line_print_reading(line);
	}
	line->data.isPending = true;

}
void UBA_line_discharge_current_control(UBA_line *line) {
	if (UBA_IN_RANGE_HYST(line->data.discharge_current,line->target.current ,UBA_LINE_DSCH_CURRENT_HYST_MA) == false) {
		UART_LOG_LINE_INFO("Current(%d) not on Target(%d)", line->data.discharge_current, line->target.current);
		int32_t diff = line->target.current - line->data.discharge_current;
		uint16_t dac = UBA_LINE_DSCH_DAC_MA2DAC(abs(diff));
		if (diff < 0) {
			UBA_line_decrease_discharge_current(line, dac);
		} else if (diff > 0) {
			UBA_line_increase_discharge_current(line, dac);
		}
		HAL_DAC_SetValue(line->dac_handle, line->dac_channel, DAC_ALIGN_12B_R, line->data_write.dac_cc);
	}
}

void UBA_line_external_chrage_control(UBA_line *line, bool currnt_control) {
	MCP47X6_StatusTypeDef status_dac;
	int32_t diff = 0;
	int32_t targate_current = currnt_control ? line->target.current : 0;
	uint16_t dac = 0;
	if ((targate_current * line->data.voltage) > UBA_LINE_CHARGE_MAX_POWER) {
		int32_t targate_current_new = (line->data.voltage > 0) ? (UBA_LINE_CHARGE_MAX_POWER / line->data.voltage) : 0;
		UART_LOG_WARNNING(line->name, "Current %d mA with voltage %d mV (power %d) exceeds allowed power %lu, reducing to %d mA", targate_current,
				line->data.voltage, (targate_current * line->data.voltage), UBA_LINE_CHARGE_MAX_POWER, targate_current_new);
		targate_current = targate_current_new;
	}
	UART_LOG_INFO(line->name, "External Charge - current:%04d/%04d DAC:%04d", line->data.charge_current, targate_current, line->data_write.dac_ex);
	if (currnt_control) {

		if (UBA_IN_RANGE_HYST(line->data.charge_current,targate_current ,UBA_LINE_CHARGE_CURRENT_HYST_MA_FACTOR) == false) {
			diff = targate_current - line->data.charge_current;
			dac = UBA_LINE_CHARGE_DAC_MA2DAC(abs(diff)); // get the difference in dac value
			UART_LOG_LINE_INFO("Current(%d mA) not on Target(%d mA) Difference:%d mA ,DAC diff : %d", line->data.charge_current, targate_current,
					diff,
					dac);
			if (UBA_IN_RANGE_HYST((line->EX_ADC_raw_data & 0xfff),line->data_write.dac_ex ,UBA_LINE_CHARGE_CURRENT_HYST_RAW) == false) {
				UART_LOG_ERROR(line->name, "ADC-DAC Not Stable External ADC:%u External DAC:%u mismatch", (line->EX_ADC_raw_data & 0xfff),
						line->data_write.dac_ex);
			} else {
				if (diff < 0) {
					UBA_line_decrease_charge_current(line, (dac / UBA_LINE_CHARGE_CURRENT_HYST_DIV) - 1);
				} else if (diff > 0) {
					UBA_line_increase_charge_current(line, (dac / UBA_LINE_CHARGE_CURRENT_HYST_DIV) + 1);
				} else {
					return;
				}
			}
		}
	}
	status_dac = MCP47X6_write_DAC_reg_value(&line->EX_DAC, line->data_write.dac_ex);
	if (status_dac != MCP47X6_STATUS_OK) {
		UART_LOG_CRITICAL(line->name, "External DAC Failed %u", status_dac);
	}

}

/**
 * @brief control the v_gen Voltage according to target voltage the charge gap
 *
 * @param line
 * @return
 */
UBA_LINE_CHARGE_DELTA UBA_line_control_v_gen(UBA_line *line) {
	UBA_LINE_CHARGE_DELTA ret = UBA_LINE_CHARGE_DELTA_INVALID;
	int delta = 0, target_delta = 0;
	delta = (line->data.gen_voltage - line->data.voltage);
	if (delta <= 0) {
		UBA_line_increase_charge_voltage(line, delta); // to low
		ret = UBA_LINE_CHARGE_DELTA_LOW;
	} else if (delta < UBA_LINE_CHARGE_GAP_MIN) {
		UBA_line_increase_charge_voltage(line, delta);
		ret = UBA_LINE_CHARGE_DELTA_LOW;
	} else if (delta > UBA_LINE_CHARGE_GAP_MAX) {
		UBA_line_decrease_charge_voltage(line);
		ret = UBA_LINE_CHARGE_DELTA_HIGH;
	} else { //  500<delta <1000
		/* 0<Vbat < Vgen< vbat+1  */
		target_delta = (line->target.voltage - line->data.voltage); // get bat taragte voltage gap
		UART_LOG_LINE_INFO("Vgen:%04d (delta:%d) is in Charge gap Target delta: %d", line->data.gen_voltage, delta, target_delta);
		if (target_delta < -20) {
			// bat id higher then target ==> lower current
			ret = UBA_LINE_CHARGE_DELTA_CV;
		} else {
			// vbat didnt reatch the target voltage
			ret = UBA_LINE_CHARGE_DELTA_ON_TARGET;
			UART_LOG_LINE_WARN("Delta on Target: %05d/%05d", line->data.voltage, line->target.voltage);
		}
	}
	UBA_DCDC_run(&line->buck_boost);
	UART_LOG_LINE_DEBUG("CHARGE_DELTA :%02X", ret);
	return ret;
}

bool UBA_line_load_cal_from_file(UBA_line *line) {
	uint8_t buffer_stream[256];
	int bytesRead = flie_logger_read((uint8_t*) line->calibration.file_name, buffer_stream, (uint32_t) sizeof(buffer_stream));
	if (bytesRead > 0) {
		pb_istream_t stream = pb_istream_from_buffer(buffer_stream, bytesRead);
		UBA_PROTO_CALIBRATION_line_calibration_message message = UBA_PROTO_CALIBRATION_line_calibration_message_init_zero;
		if (pb_decode(&stream, UBA_PROTO_CALIBRATION_line_calibration_message_fields, &message)) {
			UART_LOG_LINE_INFO("Cal File Load Successfully decoded! ");
			UBA_line_load_calibration(line, &message);
			return true;
			// Successfully decoded!
		} else {
			UART_LOG_ERROR(line->name, "File %s -  Decoding failed: %s", line->calibration.file_name, PB_GET_ERROR(&stream));
			UBA_util_print_buffer(buffer_stream, bytesRead);
			return false;
			// Error
		}
	} else {
		UART_LOG_ERROR(line->name, "Failed To Read File :  %s", line->calibration.file_name);
		return false;
	}
}

//=================================================state machine functions========================================================//
void UBA_line_init_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("Enter init");
	line->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	if (tpl010_init(&line->digital_potentiometer, line->i2c_handle)
			!= TPL0102_STATUS_OK) {
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_I2C_PERIPHERAL)) {
			UART_LOG_LINE_CRITICAL("tpl010 Failed to INIT");
		}
	} else {
		tpl010_set_potentiometer_b(&line->digital_potentiometer,
				UBA_LINE_ADC_RANGE60V);
	}
	if (MCP3221_init(&line->EX_ADC, line->i2c_handle) != MCP3221_STATUS_OK) {
		UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_I2C_PERIPHERAL);
		UART_LOG_LINE_CRITICAL("MCP3221 Failed to INIT");
	}
	if (MCP47X6_init(&line->EX_DAC, line->i2c_handle) != MCP47X6_STATUS_OK) {
		UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_I2C_PERIPHERAL);
		UART_LOG_LINE_CRITICAL("MCP47X6 Failed to INIT");
	}
	HAL_Delay(10);
	if (UBA_line_load_cal_from_file(line)) {
		line->calibration.isCalibrated = true;
	} else {
		line->calibration.isCalibrated = false;
		if (UBA_line_post_error(line, UBA_PROTO_UBA6_ERROR_LINE_NO_CALIBRATION)) {
			UART_LOG_LINE_CRITICAL("Failed To load Calibration file Form SD Card");
		}
	}
	line->data_refresh_tick = HAL_GetTick();
	line->buck_boost.p_vps = &line->data.vps;
}

void UBA_line_init(UBA_line *line) {
	if (line->error == UBA_PROTO_UBA6_ERROR_NO_ERROR) {
		UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
	} else if ((line->error & UBA_LINE_ERROR_DEAD) && (line->init_retry--)) {
		UBA_line_set_next_state(line, UBA_LINE_STATE_INIT);
		UART_LOG_LINE_WARN("Retry init %u", line->init_retry);
	} else if (line->error & UBA_LINE_ERROR_IDLE_ONLY) {
		UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
	} else {
		UBA_line_set_next_state(line, UBA_LINE_STATE_DEAD);
	}
}

void UBA_line_init_exit(UBA_line *line) {
}

void UBA_line_idle_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter Idle");
	line->init_retry = UBA_LINE_MAX_RETRY;
	UBA_line_enable_charge(line, CHARGE_DISABLE); /*disable the charge When enter Idle*/
	HAL_Delay(10);
	UBA_line_enable_discharge(line, DISCHARGE_DISABLE); /*disable the Discharge When enter Idle*/
	HAL_DAC_Start(line->dac_handle, line->dac_channel);

}
void UBA_line_idle(UBA_line *line) {
	if (HAL_GetTick() - (line->data_refresh_tick) > UBA_LINE_DATA_READ_TIME_MS) {
		line->data_refresh_tick = HAL_GetTick();
		UBA_line_get_line_data(line);
	}

}
void UBA_line_idle_exit(UBA_line *line) {
}

/**
 * @brief set the VGen voltage to the bat pre charge voltage
 *
 * @param line
 */
void UBA_line_pre_charging_enter(UBA_line *line) {
	UART_LOG_LINE_DEBUG("enter pre-Charge");
	UBA_line_update_state(line);
	UBA_line_enable_discharge(line, DISCHARGE_DISABLE);/*Safety*/
	HAL_Delay(10);
	UBA_line_enable_charge(line, CHARGE_DISABLE);/*disable the charge in pre charge*/
	line->data_write.dac_ex = 50;
	if (MCP47X6_write_DAC_reg_value(&line->EX_DAC, line->data_write.dac_ex) != MCP47X6_STATUS_OK) {
		UART_LOG_ERROR(line->name, "External DAC  Failed");
	}
	line->data.capacity = 0; //reset the capacity for the start of the charge
	UBA_line_get_line_data(line);
}
/**
 * @brief set the generator Voltage to the desire value , without enabling the charge
 *
 * @param line
 */
void UBA_line_pre_charging(UBA_line *line) {
	if (HAL_GetTick() - (line->data_refresh_tick) > UBA_LINE_PRE_CHARGE_DATA_READ_TIME_MS) {
		line->data_refresh_tick = HAL_GetTick();
		UBA_line_get_line_data(line);
		if ((line->error & UBA_LINE_ERROR_DEAD)) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_DEAD);
		} else if (line->error & UBA_LINE_ERROR_IDLE_ONLY) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
		}

		if ((UBA_line_control_v_gen(line) & UBA_LINE_CHARGE_DELTA_ON_GAP) == UBA_LINE_CHARGE_DELTA_ON_GAP) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_CHARGING_CC);
		}
	}
}

void UBA_line_pre_charging_exit(UBA_line *line) {

}

void UBA_line_charging_cc_enter(UBA_line *line) {
	UART_LOG_LINE_INFO("=========================================Enter Charge Constant Current=========================================");
	UBA_line_update_state(line);
	UBA_line_enable_charge(line, CHARGE_ENABLE);
}

void UBA_line_charging_cc(UBA_line *line) {
	uint32_t d_time = HAL_GetTick() - line->data_refresh_tick;
	if (d_time > UBA_LINE_DATA_READ_TIME_MS) {
		line->data.capacity += line->data.charge_current
				* d_time; // accumulate the last current nA*mSec
		line->data_refresh_tick = HAL_GetTick();
		UBA_line_get_line_data(line);
		if ((line->error & UBA_LINE_ERROR_DEAD)) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_DEAD);
		} else if (line->error & UBA_LINE_ERROR_IDLE_ONLY) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
		} else if ((UBA_line_control_v_gen(line) & UBA_LINE_CHARGE_DELTA_CV) == UBA_LINE_CHARGE_DELTA_CV) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_CHARGING_CV);
		}
		UBA_line_external_chrage_control(line, true); //Stabilize the charge current

	}
}

void UBA_line_charging_cc_exit(UBA_line *line) {
}

void UBA_line_charging_cv_enter(UBA_line *line) {
	UART_LOG_LINE_DEBUG("Enter Constant Voltage");
	UBA_line_update_state(line);
}

void UBA_line_charging_cv(UBA_line *line) {
	UBA_LINE_CHARGE_DELTA delta = UBA_LINE_CHARGE_DELTA_INVALID;
	uint32_t d_time = HAL_GetTick() - line->data_refresh_tick;
	if (d_time > UBA_LINE_DATA_READ_TIME_MS) {
		line->data_refresh_tick = HAL_GetTick();
		UBA_line_get_line_data(line);
		if ((line->error & UBA_LINE_ERROR_DEAD)) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_DEAD);
		} else if (line->error & UBA_LINE_ERROR_IDLE_ONLY) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
		}
		delta = UBA_line_control_v_gen(line);
		if ((delta & UBA_LINE_CHARGE_DELTA_CV) == UBA_LINE_CHARGE_DELTA_CV) {
			if (line->data_write.dac_ex > UBA_LINE_DSCH_DAC_MIN_VALUE) {
				line->data_write.dac_ex--;
			}
		} else if ((delta & UBA_LINE_CHARGE_DELTA_LOW)
				== UBA_LINE_CHARGE_DELTA_LOW) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_CHARGING_CC);
		}
		line->data.capacity += (line->data.charge_current * d_time);
	}
	UBA_line_external_chrage_control(line, false); //Stabilize the charge current
}

void UBA_line_charging_cv_exit(UBA_line *line) {

}

void UBA_line_discharging_enter(UBA_line *line) {
	uint32_t new_dac_cc = 0;
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter discharge");
	UBA_line_enable_charge(line, CHARGE_DISABLE); // Safety disable charge line when discharging
	HAL_Delay(10);
	UBA_line_enable_discharge(line, DISCHARGE_ENABLE); /*Enable line for discharging*/
	new_dac_cc = UBA_LINE_DSCH_DAC_MA2DAC(line->target.current);
	UART_LOG_LINE_WARN("Set Discharge DAC: %u [enter]", new_dac_cc);
	line->data_write.dac_cc = new_dac_cc > UBA_LINE_DSCH_DAC_MAX_VALUE ? UBA_LINE_DSCH_DAC_MAX_VALUE : new_dac_cc;
	HAL_DAC_Start(line->dac_handle, line->dac_channel);
	HAL_Delay(10);
	HAL_DAC_SetValue(line->dac_handle, line->dac_channel, DAC_ALIGN_12B_R, line->data_write.dac_cc);
	UBA_line_get_line_data(line);
	HAL_Delay(10);
	UART_LOG_LINE_DEBUG("DAC Set Value:%d mV", UBA_LINE_DAC_MV2ADC(line->data_write.dac_cc));
	line->data.capacity = 0; //reset the capacity for the start discarge
	if (line->isBattery_connected == false) {

	}

}

void UBA_line_discharging(UBA_line *line) {
	uint32_t d_time = HAL_GetTick() - line->data_refresh_tick;
	if (d_time > UBA_LINE_DATA_READ_TIME_MS) {
		line->data_refresh_tick = HAL_GetTick();
		UBA_line_get_line_data(line);
		if ((line->error & UBA_LINE_ERROR_DEAD)) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_DEAD);
		} else if (line->error & UBA_LINE_ERROR_IDLE_ONLY) {
			UBA_line_set_next_state(line, UBA_LINE_STATE_IDLE);
		}

		if (UBA_IN_RANGE_HYST((line->ADC_raw_data[ADC_CHNNEL_DSCH_CURR] & 0xfff),line->data_write.dac_cc ,UBA_LINE_CHARGE_CURRENT_HYST_MA) == false) {
			UART_LOG_ERROR(line->name, "ADC:%u DAC:%u mismatch", (line->ADC_raw_data[ADC_CHNNEL_DSCH_CURR] & 0xfff), line->data_write.dac_cc);
		}else{
			UBA_line_discharge_current_control(line);
		}
		line->data.capacity -= (line->data.discharge_current * d_time); // mA*mSec
	}
}

void UBA_line_discharging_exit(UBA_line *line) {
	line->data_write.dac_cc = 0;
		HAL_DAC_SetValue(line->dac_handle, line->dac_channel, DAC_ALIGN_12B_R, line->data_write.dac_cc);
		HAL_Delay(1);
	HAL_DAC_Stop(line->dac_handle, line->dac_channel);
}

void UBA_line_dead_enter(UBA_line *line) {
	UBA_line_update_state(line);
	UART_LOG_LINE_DEBUG("enter dead");
	UART_LOG_LINE_CRITICAL("Enter Dead State With Error:0x%X", line->error);
	line->init_retry = UBA_LINE_MAX_RETRY;
	UBA_line_enable_charge(line, CHARGE_DISABLE);
	UBA_line_enable_discharge(line, DISCHARGE_DISABLE);
}

void UBA_line_dead(UBA_line *line) {
	if (HAL_GetTick() - line->state.tick > UBA_LINE_DEAD_WAKEUP_TIME) {
		UBA_line_set_next_state(line, UBA_LINE_STATE_INIT);
	}
	UBA_line_get_line_data(line);
	//UBA_line_ADC_print_reading(line);
}

void UBA_line_dead_exit(UBA_line *line) {
}

/*=========================================public functions=======================*/

UBA_PROTO_UBA6_ERROR UBA_line_set_chrage_param(UBA_line *line, int32_t chrage_current,
		int32_t charge_voltage) {
	if (chrage_current > UBA_LINE_CURRENT_MAX) {
		UART_LOG_LINE_CRITICAL("Current :%d mV is to high for this line",
				chrage_current);
		line->data_write.dac_ex = 0;
		return UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT;
	}
	if (charge_voltage > UBA_LINE_VOLTAGE_BAT_MAX) {
		UART_LOG_LINE_CRITICAL("Voltage :%d mV is to high for this line",
				charge_voltage);
		line->data_write.dac_ex = 0;
		return UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE;
	}
	line->data_write.dac_ex = UBA_LINE_CHARGE_DAC_MA2DAC(chrage_current);

	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}
//========================================================DISCHAREGE=================================================
UBA_PROTO_UBA6_ERROR UBA_line_increase_discharge_current(UBA_line *line, uint32_t diff_dac) {
	UART_LOG_LINE_WARN("increase discharge current using DAC by %u", diff_dac);
	uint32_t new_dac = line->data_write.dac_cc + diff_dac;
	if (new_dac > UBA_LINE_DSCH_DAC_MAX_VALUE) {
		UART_LOG_LINE_WARN("new DAC is higher  then MAX Value, resetting to Maximum value");
		new_dac = UBA_LINE_DSCH_DAC_MAX_VALUE;
	}
	line->data_write.dac_cc = new_dac;

	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

UBA_PROTO_UBA6_ERROR UBA_line_decrease_discharge_current(UBA_line *line, uint32_t diff_dac) {
	UART_LOG_LINE_WARN("decrease discharge current by :%u", diff_dac);
	uint32_t new_dac = UBA_LINE_DSCH_DAC_MIN_VALUE;
	if (diff_dac > line->data_write.dac_cc) {
		UART_LOG_LINE_WARN("diff DAC is higher then current value");
		new_dac = UBA_LINE_DSCH_DAC_MIN_VALUE;
	} else {
		new_dac = line->data_write.dac_cc - diff_dac;
	}
	if (new_dac < UBA_LINE_DSCH_DAC_MIN_VALUE) {
		UART_LOG_LINE_WARN("new DAC is lower then Minimum Value, resetting to Minimum value");
		new_dac = UBA_LINE_DSCH_DAC_MIN_VALUE;
	}
	line->data_write.dac_cc = new_dac;

	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}
//======================================================== CHAREGE=================================================
UBA_PROTO_UBA6_ERROR UBA_line_increase_charge_current(UBA_line *line, uint32_t diff_dac) {
	UART_LOG_LINE_WARN("increase charge current using External DAC by %u", diff_dac);
	uint32_t new_dac = line->data_write.dac_ex + diff_dac;
	if (new_dac > UBA_LINE_CHARGE_DAC_MAX_VALUE) {
		UART_LOG_LINE_WARN("new External DAC is higher then MAX Value, resetting to minimum value");
		new_dac = UBA_LINE_CHARGE_DAC_MAX_VALUE;
	}
	line->data_write.dac_ex = new_dac;
	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

UBA_PROTO_UBA6_ERROR UBA_line_decrease_charge_current(UBA_line *line, uint32_t diff_dac) {
	UART_LOG_LINE_WARN("decrease charge current by :%u", diff_dac);
	uint32_t new_dac = UBA_LINE_CHARGE_DAC_MIN_VALUE;
	if (diff_dac > line->data_write.dac_ex) {
		UART_LOG_LINE_WARN("diff External DAC is higher then current value");
		new_dac = UBA_LINE_CHARGE_DAC_MIN_VALUE;
	} else {
		new_dac = line->data_write.dac_ex - diff_dac;
	}
	if (new_dac < UBA_LINE_CHARGE_DAC_MIN_VALUE) {
		UART_LOG_LINE_WARN("new External DAC is lower then Minimum Value, resetting to minimum value");
		new_dac = UBA_LINE_CHARGE_DAC_MIN_VALUE;
	}
	line->data_write.dac_ex = new_dac;

	return UBA_PROTO_UBA6_ERROR_NO_ERROR;

}

UBA_PROTO_UBA6_ERROR UBA_line_increase_charge_voltage(UBA_line *line, int delta) {
	uint8_t step_size = 1;
	if (delta < 500) {
		step_size = 10;
	}
	UBA_PROTO_UBA6_ERROR ret = UBA_DCDC_bock_boost_up(&line->buck_boost, step_size);
	if (ret != UBA_PROTO_UBA6_ERROR_NO_ERROR) {
		UBA_line_post_error(line, ret);
	} else if ((line->error & ret) == ret) {
		UBA_line_clear_error(line, ret);
	}
	return ret;
}

UBA_PROTO_UBA6_ERROR UBA_line_decrease_charge_voltage(UBA_line *line) {
	UBA_PROTO_UBA6_ERROR ret = UBA_DCDC_bock_boost_down(&line->buck_boost);
	if (ret != UBA_PROTO_UBA6_ERROR_NO_ERROR) {
		UBA_line_post_error(line, ret);
	} else if ((line->error & ret) == ret) {
		UBA_line_clear_error(line, ret);
	}
	return ret;
}

bool UBA_line_post_error(UBA_line *line, UBA_PROTO_UBA6_ERROR new_error) {
	if ((line->error & new_error) == 0) {
		line->error |= new_error;
		UART_LOG_LINE_ERROR("post new Error: 0x%08X line Errors:0x%08X", new_error, line->error);
		return true; // the error has being posted
	}
	return false; // the error already exist

}

bool UBA_line_clear_error(UBA_line *line, UBA_PROTO_UBA6_ERROR error2celar) {
	if (line->state.current == UBA_LINE_STATE_DEAD) {
		return false;
	}
	if ((line->error & error2celar) > 0) {
		line->error = (line->error & (~error2celar));
		UART_LOG_LINE_INFO("Clear Error: 0x%08X line Errors:0x%08X", error2celar, line->error);
		return true;
	}
	return false;
}

void UBA_line_run(UBA_line *line) {
	if (line->state.next == UBA_LINE_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[line->state.current].run) {
			rule_g[line->state.current].run(line); // run the main function of the state
		}
	} else {
		if (line->state.current < UBA_LINE_STATE_MAX) {
			if (rule_g[line->state.current].exit) {
				rule_g[line->state.current].exit(line); // run the status exit function
			}
		}
		if (rule_g[line->state.next].enter) {
			rule_g[line->state.next].enter(line); // run the next state enter function
		}
	}
}

/**
 * @brief set the next state of the line if possible
 * This function determines whether the line's state can transition to the specified next state.
 * If the line is dead, changing its state is not possible.
 * If the line is in the middle of charging or discharging, it can only transition to an idle state.
 * Transitioning to an idle state is always permitted.
 * @param line -  - Pointer to an internal line structure.
 * @param next_state - The desired next state for the line.
 * @return UBA_LINE_ERROR_NO_ERROR if it possible , UBA_LINE_ERROR_BUSY if it mid charging or discharging and UBA_LINE_ERROR_NOT_AVAILABLE if the line is dead
 */
UBA_PROTO_UBA6_ERROR UBA_line_set_next_state(UBA_line *line, UBA_LINE_STATE next_state) {
	UBA_PROTO_UBA6_ERROR ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	if ((next_state == UBA_LINE_STATE_DEAD) || (next_state == UBA_LINE_STATE_INIT)) {
		//can always go to dead or init
		line->state.next = next_state;
		ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	} else if ((line->state.current == UBA_LINE_STATE_INIT) && (next_state != UBA_LINE_STATE_IDLE)) {
		//from init can go only to idle
		ret = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	} else if ((line->state.current != UBA_LINE_STATE_DEAD) && (next_state == UBA_LINE_STATE_IDLE)) {
		line->state.next = next_state;
		ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	} else if ((line->state.current == UBA_LINE_STATE_DEAD) || (line->state.current == UBA_LINE_STATE_INIT)) {
		ret = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	} else if ((next_state == UBA_LINE_STATE_IDLE) || (line->state.current == UBA_LINE_STATE_IDLE)) {
		line->state.next = next_state;
		ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	} else if (((line->state.current >= UBA_LINE_STATE_PRE_CHARGING)
			&& (line->state.current <= UBA_LINE_STATE_CHARGING_CV))
			&& ((next_state >= UBA_LINE_STATE_PRE_CHARGING)
					&& (next_state <= UBA_LINE_STATE_CHARGING_CV))) {
		line->state.next = next_state;
		ret = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	} else {
		ret = UBA_PROTO_UBA6_ERROR_LINE_BUSY;
	}
	UART_LOG_LINE_WARN("Set line next state :%s - 0x%02x", rule_g[next_state].name, ret);
	return ret;
}

bool UBA_line_isCharging(UBA_line *line) {
	bool pin_state = HAL_GPIO_ReadPin(line->charge_enable.GPIOx, line->charge_enable.GPIO_Pin) == GPIO_PIN_SET ? CHARGE_ENABLE : CHARGE_DISABLE;
	return (((line->state.current == UBA_LINE_STATE_CHARGING_CC) || (line->state.current == UBA_LINE_STATE_CHARGING_CV))
			&& (pin_state == CHARGE_ENABLE));
}

bool UBA_line_isDischarging(UBA_line *line) {
	bool pin_state =
			HAL_GPIO_ReadPin(line->discharge_enable.GPIOx, line->discharge_enable.GPIO_Pin) == GPIO_PIN_SET ? DISCHARGE_ENABLE : DISCHARGE_DISABLE;
	return ((line->state.current == UBA_LINE_STATE_DISCHARGING) && (pin_state == CHARGE_ENABLE));
}

void UBA_line_init_local_lines(void) {
	UBA_LINE_A.i2c_handle = &hi2c3;
	UBA_LINE_A.adc_handle = &hadc1;
	UBA_LINE_A.tim_handle = &htim1;
	UBA_LINE_A.buck_boost.SW1_2_BUCK.handle = &htim1;
	UBA_LINE_A.buck_boost.SW1_2_BUCK.channel = TIM_CHANNEL_1;
	UBA_LINE_A.buck_boost.SW3_4_BOOST.handle = &htim1;
	UBA_LINE_A.buck_boost.SW3_4_BOOST.channel = TIM_CHANNEL_2;
	UBA_LINE_A.dac_handle = &hdac1;
	UBA_LINE_A.bat_negtive.GPIO_Pin = Negative_CH1_Pin;
	UBA_LINE_A.bat_negtive.GPIOx = Negative_CH1_GPIO_Port;
	UBA_LINE_A.charge_enable.GPIO_Pin = CHRG_EN_CH1_Pin;
	UBA_LINE_A.charge_enable.GPIOx = CHRG_EN_CH1_GPIO_Port;
	UBA_LINE_A.discharge_enable.GPIO_Pin = DISCH_EN_CH1_Pin;
	UBA_LINE_A.discharge_enable.GPIOx = DISCH_EN_CH1_GPIO_Port;
	UBA_LINE_A.dac_channel = DAC_CHANNEL_1;
	UBA_LINE_A.adc_resistance[UBA_LINE_ADC_RANGE10V] = 21800; //21800
	UBA_LINE_A.adc_step_value[UBA_LINE_ADC_RANGE10V] = 0xC9;
	UBA_LINE_A.adc_resistance[UBA_LINE_ADC_RANGE30V] = 4300; //4K
	UBA_LINE_A.adc_step_value[UBA_LINE_ADC_RANGE30V] = 0xF5;
	UBA_LINE_A.adc_resistance[UBA_LINE_ADC_RANGE60V] = 780; //740
	UBA_LINE_A.adc_step_value[UBA_LINE_ADC_RANGE60V] = 0xFE;
	UBA_LINE_A.adc_resistance[UBA_LINE_ADC_RANGE_SAFETY] = 300;
	UBA_LINE_A.adc_step_value[UBA_LINE_ADC_RANGE_SAFETY] = 0xff;
	UBA_LINE_A.R23_R55 = 56000;
	UBA_LINE_A.R28_R60 = 2210;
	UBA_LINE_A.ADC_DMA_SIZE = ADC_CHNNEL_MAX;

	memcpy(UBA_LINE_A.name, "Line A", 7);
	memcpy(UBA_LINE_A.calibration.file_name, "LINE_A.cal", 11);
	UBA_LINE_B.i2c_handle = &hi2c4;
	UBA_LINE_B.adc_handle = &hadc2;
	UBA_LINE_B.tim_handle = &htim8;
	UBA_LINE_B.buck_boost.SW1_2_BUCK.handle = &htim8;
	UBA_LINE_B.buck_boost.SW1_2_BUCK.channel = TIM_CHANNEL_1;
	UBA_LINE_B.buck_boost.SW3_4_BOOST.handle = &htim8;
	UBA_LINE_B.buck_boost.SW3_4_BOOST.channel = TIM_CHANNEL_2;

	UBA_LINE_B.dac_handle = &hdac1;
	UBA_LINE_B.bat_negtive.GPIO_Pin = Negative_CH2_Pin;
	UBA_LINE_B.bat_negtive.GPIOx = Negative_CH2_GPIO_Port;
	UBA_LINE_B.charge_enable.GPIO_Pin = CHRG_EN_CH2_Pin;
	UBA_LINE_B.charge_enable.GPIOx = CHRG_EN_CH2_GPIO_Port;
	UBA_LINE_B.discharge_enable.GPIO_Pin = DISCH_EN_CH2_Pin;
	UBA_LINE_B.discharge_enable.GPIOx = DISCH_EN_CH2_GPIO_Port;
	UBA_LINE_B.dac_channel = DAC_CHANNEL_2;
	UBA_LINE_B.adc_resistance[UBA_LINE_ADC_RANGE10V] = 21800; //21800
	UBA_LINE_B.adc_step_value[UBA_LINE_ADC_RANGE10V] = 0xC9;
	UBA_LINE_B.adc_resistance[UBA_LINE_ADC_RANGE30V] = 4300; //4K
	UBA_LINE_B.adc_step_value[UBA_LINE_ADC_RANGE30V] = 0xF5;
	UBA_LINE_B.adc_resistance[UBA_LINE_ADC_RANGE60V] = 780; //740
	UBA_LINE_B.adc_step_value[UBA_LINE_ADC_RANGE60V] = 0xFE;
	UBA_LINE_B.adc_resistance[UBA_LINE_ADC_RANGE_SAFETY] = 300;
	UBA_LINE_B.adc_step_value[UBA_LINE_ADC_RANGE_SAFETY] = 0xff;
	UBA_LINE_B.R23_R55 = 56000;
	UBA_LINE_B.R28_R60 = 2210;
	UBA_LINE_B.ADC_DMA_SIZE = ADC_CHNNEL_MAX;

	memcpy(UBA_LINE_B.name, "Line B", 7);
	memcpy(UBA_LINE_B.calibration.file_name, "LINE_B.cal", 11);
}

void UBA_line_update_message(UBA_line *line, UBA_PROTO_LINE_status *msg) {
	msg->state = line->state.current;
	msg->error = line->error;
	msg->id = (line == &UBA_LINE_A) ? UBA_PROTO_LINE_ID_A : UBA_PROTO_LINE_ID_B;

	msg->data.voltage = line->data.voltage;
	msg->data.gen_voltage = line->data.gen_voltage;
	msg->data.bat_temperature = line->data.bat_temperature;
	msg->data.amb_temperature = line->data.amb_temperature;
	msg->data.discharge_current = line->data.discharge_current;
	msg->data.charge_current = line->data.charge_current;
	msg->data.capacity = line->data.capacity;
	msg->data.vps = line->data.vps;

	msg->data.isBattery_connected = line->isBattery_connected;
	if (msg->has_adc_data) {
		msg->adc_data.vbat = line->ADC_raw_data[ADC_CHNNEL_VBAT];
		msg->adc_data.vgen = line->ADC_raw_data[ADC_CHNNEL_VGEN];
		msg->adc_data.amb_temp = line->ADC_raw_data[ADC_CHNNEL_AMB_TEMP];
		msg->adc_data.ntc_temp = line->ADC_raw_data[ADC_CHNNEL_NTC_BAT];
		msg->adc_data.discharge_curr = line->ADC_raw_data[ADC_CHNNEL_DSCH_CURR];
		msg->adc_data.charge_curr = line->EX_ADC_raw_data;
	}
}

void UBA_line_load_calibration(UBA_line *line, UBA_PROTO_CALIBRATION_line_calibration_message *msg) {
	UART_LOG_LINE_INFO("Load Calibration into line");
	UART_LOG_LINE_DEBUG("Vbat(%04.4f,%04.4f)", msg->vbat[0].slop, msg->vbat[0].y_intercept);
	COPY_CALIB_PARAM(vbat[0]);
	COPY_CALIB_PARAM(vbat[1]);
	COPY_CALIB_PARAM(vbat[2]);
	COPY_CALIB_PARAM(vgen);
	COPY_CALIB_PARAM(vps);
	COPY_CALIB_PARAM(charge_current);
	COPY_CALIB_PARAM(discharge_current);
	COPY_CALIB_PARAM(amb_temp);
	COPY_CALIB_PARAM(ntc_temp);
	line->calibration.isCalibrated = true;
}

void UBA_line_command_execute(UBA_line *line, UBA_PROTO_LINE_command *cmd) {
	switch (cmd->id) {
		case UBA_PROTO_LINE_CMD_ID_TEST:
			UART_LOG_LINE_INFO("Test Command");
			break;
		case UBA_PROTO_LINE_CMD_ID_CALIBRATION:
			line->calibration.isCalibrated = (cmd->state > 0);
			UART_LOG_LINE_INFO("Is isCalibrated :%u", line->calibration.isCalibrated);
			break;
		case UBA_PROTO_LINE_CMD_ID_CHARGE_ENABLE:
			UBA_line_enable_charge(line, (cmd->state > 0));
			break;
		case UBA_PROTO_LINE_CMD_ID_DISCHARGE_ENABLE:
			UBA_line_enable_discharge(line, (cmd->state > 0));
			break;
		case UBA_PROTO_LINE_CMD_ID_TARGET:
			line->target.current = cmd->current;
			line->target.voltage = cmd->voltage;
			UBA_line_set_next_state(line, cmd->state);
			break;
		case UBA_PROTO_LINE_CMD_ID_BIST:
			UBA_line_BIST(line);
		default:

	}
}
