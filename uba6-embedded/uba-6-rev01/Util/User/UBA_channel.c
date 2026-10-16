/*
 * UBA_channel.c
 *
 *  Created on: Aug 29, 2024
 *      Author: ORA
 */

#include <UBA_channel.h>
#include "uart_log.h"
#include "adc.h"
#include "UBA_util.h"
#include "string.h"
#include "stdbool.h"
#include "file_logger.h"
#include "UBA_Message.pb.h"

UBA_channel UBA_CH_A;
UBA_channel UBA_CH_B;
UBA_channel UBA_CH_AB;

#define UBA_GEN_VOLTAGE_HYSTERESIS (50) /*in mV*/

#if (UBA_LOG_LEVEL_CHANNEL <= UART_LOG_LEVEL_INFO)
#define UART_LOG_CHANNEL_INFO(ch_name,...) UART_LOG_INFO(ch_name,##__VA_ARGS__)
#else
#define UART_LOG_CHANNEL_INFO(...)
#endif

#if (UBA_LOG_LEVEL_CHANNEL <= UART_LOG_LEVEL_DEBUG)
	#define UART_LOG_CHANNEL_DEBUG(ch_name,...) UART_LOG_DEBUG(ch_name,##__VA_ARGS__)
#else
#define UART_LOG_CHANNEL_DEBUG(...)
#endif

#define UBA_COMP 		"Channel"
#define UBA_CHANNEL_DISCHARGE_HYSTERESIS (1) /*5mA HYSTERESIS*/
#define HOURS2MILISEC (3600000) /*1000*60*60*/
#define UBA_channel_MIN_BAT_VOLTAGE (2000)

void UBA_channel_init_enter(UBA_channel *ch);
void UBA_channel_init(UBA_channel *ch);
void UBA_channel_init_exit(UBA_channel *ch);

void UBA_channel_standby_enter(UBA_channel *ch);
void UBA_channel_standby(UBA_channel *ch);
void UBA_channel_standby_exit(UBA_channel *ch);

void UBA_channel_delay_enter(UBA_channel *ch);
void UBA_channel_delay(UBA_channel *ch);
void UBA_channel_delay_exit(UBA_channel *ch);

void UBA_channel_charging_enter(UBA_channel *ch);
void UBA_channel_charging(UBA_channel *ch);
void UBA_channel_charging_exit(UBA_channel *ch);

void UBA_channel_discharging_enter(UBA_channel *ch);
void UBA_channel_discharging(UBA_channel *ch);
void UBA_channel_discharging_exit(UBA_channel *ch);

void UBA_channel_dead_enter(UBA_channel *ch);
void UBA_channel_dead(UBA_channel *ch);
void UBA_channel_dead_exit(UBA_channel *ch);

typedef void (*step_cb_t)(UBA_channel *ch);

/***
 * UBA Channel State Machine Assigner Rule
 */
struct UBACSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
	char *name;
};

/*UBA Channel State Machine Assigner */
#define UBACSMA(step, cbe, cbr, cbx)[UBA_CHANNEL_STATE_##step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx , .name=#step}

// @formatter:off
static const struct UBACSMA_rule rule_g[UBA_CHANNEL_STATE_MAX] ={
		UBACSMA(INIT,		UBA_channel_init_enter,			UBA_channel_init,			UBA_channel_init_exit),
		UBACSMA(STANDBY,	UBA_channel_standby_enter,		UBA_channel_standby,		UBA_channel_standby_exit),
		UBACSMA(DELAY,		UBA_channel_delay_enter,		UBA_channel_delay,			UBA_channel_delay_exit),
		UBACSMA(CHARGE,		UBA_channel_charging_enter,		UBA_channel_charging,		UBA_channel_charging_exit),
		UBACSMA(DISCHARGE,	UBA_channel_discharging_enter,	UBA_channel_discharging,	UBA_channel_discharging_exit),
		UBACSMA(OFF,		UBA_channel_dead_enter,			UBA_channel_dead,			UBA_channel_dead_exit),

};
// @formatter:on

//=================================================private functions========================================================//
void UBA_channel_update_state(UBA_channel *ch) {
	if (ch->state.current < UBA_CHANNEL_STATE_MAX && ch->state.next < UBA_CHANNEL_STATE_MAX) {
		UART_LOG_INFO(ch->name, "update state %s ---> %s", rule_g[ch->state.current].name, rule_g[ch->state.next].name);
	} else {
		UART_LOG_INFO(ch->name, "update state %u ---> %u", ch->state.current, ch->state.next);
	}
	ch->state.pre = ch->state.current;
	ch->state.current = ch->state.next;
	ch->state.next = UBA_CHANNEL_STATE_INVALID;
}

bool is_data_pending(UBA_channel *ch) {
	bool isPending = ch->line_size > 0 ? true : false;
	for (int index = 0; index < ch->line_size; index++) {
		if (ch->lines_p[index]->data.isPending) {
			isPending &= ch->lines_p[index]->data.isPending;
		}
	}
	return isPending;
}

void UBA_channel_set_line_cuurent(UBA_channel *ch, int32_t channel_current) {
	for (int index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->target.current = (channel_current / ch->line_size);
	}
	ch->target.current = channel_current;
}

bool is_lines_in_state(UBA_channel *ch, UBA_LINE_STATE line_state) {
	bool ret = true;
	for (int index = 0; index < ch->line_size; index++) {
		ret &= (ch->lines_p[index]->state.current == line_state);
	}
	return ret;
}
bool UBA_channel_lines_updated_state(UBA_channel *ch, UBA_LINE_STATE line_state) {
	UBA_PROTO_UBA6_ERROR lien_err = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	for (int index = 0; index < ch->line_size; index++) {
		lien_err = UBA_line_set_next_state(ch->lines_p[index], line_state);
		if (lien_err != UBA_PROTO_UBA6_ERROR_NO_ERROR) {
			return false;
		}
	}
	return true;
}

UBA_PROTO_UBA6_ERROR UBA_channel_get_lines_errors(UBA_channel *ch) {
	UBA_PROTO_UBA6_ERROR lien_err = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	for (int index = 0; index < ch->line_size; index++) {
		lien_err |= ch->lines_p[index]->error;
	}
	return lien_err;
}

void UBA_channel_save_data(UBA_channel *ch) {
	MSG_Message message = MSG_Message_init_zero;
	uint8_t file_name[11] = { 0 };
	switch (ch->id) {
		case UBA_CHANNLE_ID_A:
			strcpy((char*) file_name, "CH_A.csv");
			break;
		case UBA_CHANNLE_ID_B:

			strcpy((char*) file_name, "CH_B.csv");
			break;
		case UBA_CHANNLE_ID_AB:
			strcpy((char*) file_name, "CH_AB.csv");
			break;
		default:
			UART_LOG_ERROR(ch->name, "ID is unknoun:%x", ch->id);
			return;
	}
	UBA_channel_update_message(ch, &message.pyload.channel_status);
	file_logger_channel_append(file_name, &(message.pyload.channel_status.data));
}

//=================================================state machine functions========================================================//
void UBA_channel_init_enter(UBA_channel *ch) {
	uint8_t line_index = 0;
	UBA_channel_update_state(ch);
	ch->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
	if ((ch->id & UBA_CHANNLE_ID_A) == UBA_CHANNLE_ID_A) {
		ch->lines_p[line_index++] = &UBA_LINE_A;
	}
	if ((ch->id & UBA_CHANNLE_ID_B) == UBA_CHANNLE_ID_B) {
		ch->lines_p[line_index++] = &UBA_LINE_B;
	}
	if (line_index == 0) {
		UART_LOG_CRITICAL(ch->name, "init Failed , channel dose not have any connected lines");
	}
	ch->line_size = line_index;

}

void UBA_channel_init(UBA_channel *ch) {
	if (ch->line_size) {
		ch->state.next = UBA_CHANNEL_STATE_STANDBY;
	} else {
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
}

void UBA_channel_init_exit(UBA_channel *ch) {
	UNUSED(ch);
}

void UBA_channel_standby_enter(UBA_channel *ch) {
	UBA_channel_update_state(ch);
	if (UBA_channel_lines_updated_state(ch, UBA_LINE_STATE_IDLE) == false) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR );
	}
}

void UBA_channel_standby(UBA_channel *ch) {
	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_DEAD) {
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
}

void UBA_channel_standby_exit(UBA_channel *ch) {

}

void UBA_channel_delay_enter(UBA_channel *ch) {
	UBA_channel_update_state(ch);
	if (UBA_channel_lines_updated_state(ch, UBA_LINE_STATE_IDLE) == false) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
	}
}

void UBA_channel_delay(UBA_channel *ch) {
	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_DEAD) {
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
}

void UBA_channel_delay_exit(UBA_channel *ch) {

}

void UBA_channel_charging_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->target.current = (ch->target.current / ch->line_size);
		ch->lines_p[index]->target.voltage = ch->target.voltage;
	}
	if (UBA_channel_lines_updated_state(ch, UBA_LINE_STATE_PRE_CHARGING) == false) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_LINE_BUSY);
	}
}

void UBA_channel_charging(UBA_channel *ch) {
	uint8_t index;
	for (index = 0; index < ch->line_size; index++) {
		if (ch->lines_p[index]->data.isPending) {
			ch->lines_p[index]->data.isPending = false;
			UART_LOG_CHANNEL_INFO(ch->name, "charge voltage: %05d target Voltage :%05d", ch->lines_p[index]->data.gen_voltage,
					ch->target.voltage);
		}
	}

	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_DEAD) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_IDLE_ONLY) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
		ch->state.next = UBA_CHANNEL_STATE_STANDBY;
	}
}

void UBA_channel_charging_exit(UBA_channel *ch) {
}

void UBA_channel_discharging_enter(UBA_channel *ch) {
	uint8_t index;
	UBA_channel_update_state(ch);
	for (index = 0; index < ch->line_size; index++) {
		if (UBA_line_set_next_state(ch->lines_p[index], UBA_LINE_STATE_DISCHARGING) == UBA_PROTO_UBA6_ERROR_NO_ERROR) {
		} else {
			UART_LOG_CRITICAL(ch->name, "%s Failed set state :%u", ch->lines_p[index]->name, UBA_LINE_STATE_DISCHARGING);
		}
	}
	PID_init(&ch->target.pid, 2.0f, 0.2f, 0.2f, (float) ch->target.current);
	ch->target.pid_tick = HAL_GetTick();

}

void UBA_channel_discharging(UBA_channel *ch) {
	int32_t target_current = 0;
	uint32_t channel_voltage = UBA_channel_get_voltage(ch);
	if (channel_voltage < UBA_channel_MIN_BAT_VOLTAGE) {
		UART_LOG_CRITICAL(ch->name, "Channel Voltage is zero , can't calculate current");
	}
	if( ch->target.discharge_current.value == 0){
		UART_LOG_CRITICAL(ch->name, "discharge current target is not define");
	} else {
		switch (ch->target.discharge_current.type) {
			case UBA_PROTO_BPT_DISCHARGE_CURRENT_TYPE_ABSOLUTE:
				target_current = ch->target.discharge_current.value;
				break;
			case UBA_PROTO_BPT_DISCHARGE_CURRENT_TYPE_POWER: // value is in Watt (A*V)
				target_current = (ch->target.discharge_current.value * 1000 * 1000) / channel_voltage;

				break;
			case UBA_PROTO_BPT_DISCHARGE_CURRENT_TYPE_RESISTANCE:

				target_current = channel_voltage / ch->target.discharge_current.value;
				break;
			default:
				UART_LOG_CRITICAL(UBA_COMP, "Discharge Type:0x%02X is not supported yet", ch->target.discharge_current.type);
		}
	}
	if(target_current == 0){
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}else if (ch->target.current != target_current) {
		UART_LOG_CRITICAL(ch->name, "Changing discharge current target Voltage from %05lu to %05lu", ch->target.current, target_current);
		ch->target.current = target_current;
		UBA_channel_set_line_cuurent(ch, target_current);
	}

	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_DEAD) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
		ch->state.next = UBA_CHANNEL_STATE_OFF;
	}
	if (UBA_channel_get_lines_errors(ch) & UBA_LINE_ERROR_IDLE_ONLY) {
		UBA_channel_post_error(ch, UBA_PROTO_UBA6_ERROR_INTRENAL_LINE_ERROR);
		ch->state.next = UBA_CHANNEL_STATE_STANDBY;
	}
}

void UBA_channel_discharging_exit(UBA_channel *ch) {
}

void UBA_channel_dead_enter(UBA_channel *ch) {
	UBA_channel_update_state(ch);
}
void UBA_channel_dead(UBA_channel *ch) {
}
void UBA_channel_dead_exit(UBA_channel *ch) {
}

//=================================================public functions========================================================//

uint32_t UBA_channel_get_voltage(UBA_channel *ch) {
	uint32_t voltage = 0;
	uint8_t index = 0;
	if (ch->line_size > 0) {
		voltage += ch->lines_p[index++]->data.voltage;
		for (; index < ch->line_size; index++) {
			voltage += ch->lines_p[index]->data.voltage;
		}
		voltage /= ch->line_size;
		UART_LOG_CHANNEL_DEBUG(ch->name, "Voltage:%u", voltage);
	} else {
		UART_LOG_ERROR(ch->name, "line size is zero ");
	}
	return voltage;
}

float UBA_channel_get_temperature(UBA_channel *ch) {
	float temp = -273;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		temp = UBA_LCD_MAX(temp, ch->lines_p[index]->data.bat_temperature);
	}
	UART_LOG_CHANNEL_DEBUG(ch->name, "Temperature:%f", temp);
	return temp;
}

uint32_t UBA_channel_get_charge_current(UBA_channel *ch) {
	uint32_t charge_current = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		charge_current += ch->lines_p[index]->data.charge_current;
	}
	UART_LOG_CHANNEL_DEBUG(ch->name, "Charge Current:%u", charge_current);
	return charge_current;
}

uint32_t UBA_channel_get_discharge_current(UBA_channel *ch) {
	uint32_t discharge_current = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		discharge_current += ch->lines_p[index]->data.discharge_current;
	}
	UART_LOG_CHANNEL_DEBUG(ch->name, "Discharge current:%u", discharge_current);
	return discharge_current;
}

int32_t UBA_channel_get_current(UBA_channel *ch) {
	int32_t current = 0;
	for (uint8_t index = 0; index < ch->line_size; index++) {
		if (ch->state.current == UBA_CHANNEL_STATE_DISCHARGE) {
			current -= ch->lines_p[index]->data.discharge_current;
		} else if (UBA_channel_isCharging(ch)) {
			current += ch->lines_p[index]->data.charge_current;
		} else {
			// keep it zero
		}
	}
	return current;
}

void UBA_channel_reset_capacity(UBA_channel *ch) {
	for (int index = 0; index < ch->line_size; index++) {
		ch->lines_p[index]->data.capacity = 0;
	}
	ch->capacity  = 0;
}

void UBA_channel_updated_cap(UBA_channel *ch,uint32_t dt){
	ch->capacity +=  ((UBA_channel_get_current(ch) * (int32_t)UBA_channel_get_voltage * (int32_t)dt) / HOURS2MILISEC);
}

float UBA_channel_get_capacity(UBA_channel *ch) {
	float capacity = 0;
	uint8_t index = 0;
	for (index = 0; index < ch->line_size; index++) {
		capacity += (ch->lines_p[index]->data.capacity / HOURS2MILISEC);
	}

	UART_LOG_CHANNEL_DEBUG(ch->name, "Capacity:%u", capacity);
	//return ch->capacity;
	return capacity;
}

UBA_PROTO_UBA6_ERROR UBA_channel_set_discharge_param(UBA_channel *ch, UBA_PROTO_BPT_discharge_current *discharge_current) {
	ch->target.discharge_current.type = discharge_current->type;
	ch->target.discharge_current.value = discharge_current->value;
	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

UBA_PROTO_UBA6_ERROR UBA_channel_set_charge_param(UBA_channel *ch, int32_t charge_current, int32_t charge_voltage) {
	ch->target.current = charge_current;
	ch->target.voltage = charge_voltage;
	return UBA_PROTO_UBA6_ERROR_NO_ERROR;
}

uint32_t UBA_channel_set_next_state(UBA_channel *ch, UBA_CHANNEL_STATE next_state) {
	ch->state.next = next_state;
	return 0;
}

void UBA_channel_run(UBA_channel *ch) {
	if (ch->state.next == UBA_CHANNEL_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[ch->state.current].run) {
			rule_g[ch->state.current].run(ch); // run the main function of the state
		}
	} else {
		if (ch->state.current < UBA_CHANNEL_STATE_MAX) {
			if (rule_g[ch->state.current].exit) {
				rule_g[ch->state.current].exit(ch); // run the status exit function
			}
		}
		if (rule_g[ch->state.next].enter) {
			rule_g[ch->state.next].enter(ch); // run the next state enter function
		}
	}
}


bool UBA_channel_isCharging(UBA_channel *ch) {
	uint8_t index = 0;
	bool ret = true;
	for (index = 0; index < ch->line_size; index++) {
		ret &= UBA_line_isCharging(ch->lines_p[index]);
	}
	return ret;
}

bool UBA_channel_isDischarging(UBA_channel *ch) {
	uint8_t index = 0;
	bool ret = true;
	for (index = 0; index < ch->line_size; index++) {
		ret &= UBA_line_isDischarging(ch->lines_p[index]);
	}
	return ret;
}

void UBA_channel_post_error(UBA_channel *ch, UBA_PROTO_UBA6_ERROR error) {
	ch->error |= error;
}
void UBA_channel_clear_error(UBA_channel *ch, UBA_PROTO_UBA6_ERROR error) {
	ch->error = (ch->error & (~error));
}

void UBA_channel_init_g(void) {
	UBA_CH_A.id = UBA_CHANNLE_ID_A;
	memcpy(UBA_CH_A.name, "Channel A", 10);
	UBA_CH_B.id = UBA_CHANNLE_ID_B;
	memcpy(UBA_CH_B.name, "Channel B", 10);
	UBA_CH_AB.id = UBA_CHANNLE_ID_AB;
	memcpy(UBA_CH_AB.name, "Channel AB", 11);
}

void UBA_channle_start_cmd(UBA_channel *uba_ch) {

}

void UBA_channel_update_message(UBA_channel *ch, UBA_PROTO_CHANNEL_status *msg) {
	msg->id = ch->id;
	msg->size = ch->line_size;
	msg->state = ch->state.current;
	msg->error = ch->error;
	msg->has_data = true;
	msg->data.voltage = UBA_channel_get_voltage(ch);
	msg->data.temperature = UBA_channel_get_temperature(ch);
	msg->data.current = UBA_channel_get_current(ch);
	msg->data.capacity = UBA_channel_get_capacity(ch);


	for (int index = 0; index < ch->line_size; index++) {
		msg->line_status[index].has_adc_data = true; //rerive the adc data also
		UBA_line_update_message(ch->lines_p[index], &msg->line_status[index]);
	}
}

void UBA_channel_command_execute(UBA_channel *ch, UBA_PROTO_CHANNEL_command *cmd) {
	switch (cmd->id) {
		case UBA_PROTO_CHANNEL_CMD_ID_TEST:
			break;
		case UBA_PROTO_CHANNEL_CMD_ID_DCDC:
			break;
		default:

	}

}

