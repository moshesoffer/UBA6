/*
 * UBA_line.h
 *
 *  Created on: Oct 8, 2024
 *      Author: ORA
 */

#ifndef UBA_LINE_H_
#define UBA_LINE_H_

#include "gpio.h"
#include "i2c.h"
#include "stdbool.h"
#include "stddef.h"
#include "MCP47X6.h"
#include "TPL0102.h"
#include "MCP3221.h"
#include "tim.h"
#include "adc.h"
#include "UBA_util.h"
#include "UBA_common_def.h"
#include "UBA_PWM.h"
#include "UBA_DC_DC_converter.h"
#include "UBA_PROTO_LINE.pb.h"
#include "UBA_PROTO_CALIBRATION.pb.h"

typedef enum UBA_LINE_ADC_RANGE {
	UBA_LINE_ADC_RANGE10V,
	UBA_LINE_ADC_RANGE30V,
	UBA_LINE_ADC_RANGE60V,
	UBA_LINE_ADC_RANGE_SAFETY,
	UBA_LINE_ADC_RANGE_MAX
} UBA_LINE_ADC_RANGE;

typedef enum UBA_LINE_STATE {
	UBA_LINE_STATE_INIT,
	UBA_LINE_STATE_IDLE,
	UBA_LINE_STATE_PRE_CHARGING,
	UBA_LINE_STATE_CHARGING_CC,
	UBA_LINE_STATE_CHARGING_CV,
	UBA_LINE_STATE_DISCHARGING,
	UBA_LINE_STATE_DEAD,
	UBA_LINE_STATE_MAX,
	UBA_LINE_STATE_INVALID = 0xff,

} UBA_LINE_STATE;



#define	UBA_LINE_ERROR_DEAD ( UBA_PROTO_UBA6_ERROR_LINE_I2C_PERIPHERAL | UBA_PROTO_UBA6_ERROR_LINE_BIST_FALIED | UBA_PROTO_UBA6_ERROR_LINE_NO_CALIBRATION \
			| UBA_PROTO_UBA6_ERROR_LINE_DISCHARGE_MOSFET_SHORT | UBA_PROTO_UBA6_ERROR_LINE_CHARGE_MOSFET_SHORT | UBA_PROTO_UBA6_ERROR_LINE_LOW_INPUT_VOLTAGE \
			| UBA_PROTO_UBA6_ERROR_LINE_HIGH_INPUT_VOLTAGE)
#define UBA_LINE_ERROR_IDLE_ONLY ( UBA_PROTO_UBA6_ERROR_LINE_OVERCURRENT | UBA_PROTO_UBA6_ERROR_LINE_VGEN_LIMITES | UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_VOLTAGE \
			| UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_CURRENT | UBA_PROTO_UBA6_ERROR_LINE_BAT_HIGH_TEMP | UBA_PROTO_UBA6_ERROR_LINE_AMB_HIGH_TEMP | UBA_PROTO_UBA6_ERROR_LINE_REVERSE_POLARITY\
			| UBA_PROTO_UBA6_ERROR_LINE_OVER_VOLTAGE | UBA_PROTO_UBA6_ERROR_LINE_VGEN_EXPECTED_MAX_VOLTAGE | UBA_PROTO_UBA6_ERROR_LINE_VGEN_FAILED|UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED)



#define UBA_LINE_PWM_CH_BUCK TIM_CHANNEL_1
#define UBA_LINE_PWM_CH_BOOST TIM_CHANNEL_2

#define UBA_LINE_MAX_RETRY (0)

typedef struct UBA_line {
	UBA_PROTO_UBA6_ERROR error; /*the error of line */
	uint8_t name[10]; /*the name of the line*/
	uint8_t init_retry; /*retry counter for failed init*/
	uint32_t data_refresh_tick; /*the tick time that a new data was acquired*/
	uint32_t data_log_print_tick; /*the tick time that a new data was acquired*/
	struct {
		UBA_LINE_STATE pre;/*the previous state*/
		UBA_LINE_STATE current; /*the current state*/
		UBA_LINE_STATE next;/*next state id there is any*/
		uint32_t tick;/*the time the line enter a new state*/
	} state;
	MCP47X6 EX_DAC; /*the control for MCP4726 device */
	MCP3221 EX_ADC;
	TPL0102 digital_potentiometer;
	ADC_HandleTypeDef *adc_handle;
	DAC_HandleTypeDef *dac_handle;
	I2C_HandleTypeDef *i2c_handle;
	TIM_HandleTypeDef *tim_handle;
	UBA_DC_DC buck_boost;
	uint32_t dac_channel;
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} bat_negtive;
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} charge_enable;
	struct {
		GPIO_TypeDef *GPIOx;
		uint16_t GPIO_Pin;
	} discharge_enable;
	struct {
		int32_t voltage;/*the voltage on the line*/
		int32_t gen_voltage;/*the voltage that the line generate*/
		int32_t vps;/*the voltage on the input of the line*/
		float bat_temperature; /*the temperature on the battery */
		float amb_temperature; /*the temperature on the heat sink*/
		int32_t discharge_current; /*in mA*/
		int32_t charge_current; /*in mA*/
		float capacity; /*in mA for sec */
		bool isPending;
	} data;
	struct {
		uint32_t dac_cc; // the discharge dac value
		uint16_t dac_ex; // the charge external dac value
	} data_write;
	uint16_t ADC_raw_data[ADC_CHNNEL_MAX];
	uint8_t ADC_DMA_SIZE;
	uint16_t EX_ADC_raw_data;
	bool isBusy;
	bool isBattery_connected; // if a battery connection has made
	bool isBattery_temperature_connected; //
	uint16_t R28_R60;
	uint32_t R23_R55;
	uint32_t adc_resistance[4];
	uint16_t adc_step_value[4];
	struct {
		int32_t voltage;/*the target voltage of the line in mV*/
		int32_t current; /*the target current of the line in mA*/
	} target;
	struct {
		bool isCalibrated;
		liner_equation vbat[UBA_LINE_ADC_RANGE_MAX];
		liner_equation vgen;
		liner_equation vps;
		liner_equation charge_current;
		liner_equation discharge_current;
		liner_equation amb_temp;
		liner_equation ntc_temp;
		uint8_t file_name[16];
		uint32_t max_charge_current;
		uint32_t max_discharge_current;
		uint32_t max_charge_voltage;
	} calibration;
} UBA_line;
// @formatter:off

#define UBA_INTRENAL_LINE_DEFUALT 	\
{									\
	UBA_PROTO_UBA6_ERROR_NO_ERROR,	\
	"LINE ?",						\
	UBA_LINE_MAX_RETRY,/*init retry*/\
	0,								\
	0,	/*print tick*/				\
	{								\
		UBA_LINE_STATE_INIT,		\
		UBA_LINE_STATE_INIT,		\
		UBA_LINE_STATE_INIT,			\
		0 /*new state tick*/		\
	},								\
	MCP47X6_DEFUALT,				\
	MCP3221_DEFUALT,				\
	TPL0102_DEFUALT,				\
	NULL,							\
	NULL,							\
	NULL,							\
	NULL,							\
	UBA_DC_DC_DEFUALT,				\
	0,		/*dac channel*/			\
	{0}, /*bat negitibe*/						\
	{0},							\
	{0},							\
	{0},							\
	{0},							\
	{0},							\
	0,		/*ADC DMA size*/		\
	0,		/*EX_ADC_raw_data*/		\
	false,/*isBusy*/				\
	false,							\
	false,							\
	0,								\
	0,								\
	{0},							\
	{0},							\
	{0,0},							\
	{								\
		false,						\
		{							\
			LINER_EQUATION_DEFUALT,	\
			LINER_EQUATION_DEFUALT,	\
			LINER_EQUATION_DEFUALT,	\
			LINER_EQUATION_DEFUALT	\
		},							\
		LINER_EQUATION_DEFUALT,		\
		LINER_EQUATION_DEFUALT,		\
		LINER_EQUATION_DEFUALT,		\
		LINER_EQUATION_DEFUALT,		\
		LINER_EQUATION_DEFUALT,		\
		LINER_EQUATION_DEFUALT,		\
		{0},						\
		0,							\
		0,							\
		0							\
	} 								\
}
// @formatter:on

void UBA_line_run(UBA_line *line);
void UBA_line_init_local_lines(void);
bool UBA_line_isCharging(UBA_line *line);
bool UBA_line_isDischarging(UBA_line *line);
void UBA_line_get_line_data(UBA_line *line);

UBA_PROTO_UBA6_ERROR UBA_line_set_next_state(UBA_line *line, UBA_LINE_STATE next_state);

UBA_PROTO_UBA6_ERROR UBA_line_increase_discharge_current(UBA_line *line, uint32_t diff_dac);
UBA_PROTO_UBA6_ERROR UBA_line_decrease_discharge_current(UBA_line *line, uint32_t diff_dac);

UBA_PROTO_UBA6_ERROR UBA_line_increase_charge_current(UBA_line *line, uint32_t diff_dac);
UBA_PROTO_UBA6_ERROR UBA_line_decrease_charge_current(UBA_line *line, uint32_t diff_dac);
UBA_PROTO_UBA6_ERROR UBA_line_increase_charge_voltage(UBA_line *line, int delta);
UBA_PROTO_UBA6_ERROR UBA_line_decrease_charge_voltage(UBA_line *line);

bool UBA_line_post_error(UBA_line *line, UBA_PROTO_UBA6_ERROR new_error);
bool UBA_line_clear_error(UBA_line *line, UBA_PROTO_UBA6_ERROR error2celar);
void UBA_line_update_message(UBA_line *line, UBA_PROTO_LINE_status *msg);
void UBA_line_load_calibration(UBA_line *line, UBA_PROTO_CALIBRATION_line_calibration_message *msg);
void UBA_line_command_execute(UBA_line *line, UBA_PROTO_LINE_command *cmd);

extern UBA_line UBA_LINE_A;
extern UBA_line UBA_LINE_B;

#endif /* UBA_LINE_H_ */
