/*
 * UBA_6.c
 *
 *  Created on: Sep 24, 2024
 *      Author: ORA
 */

#include "UBA_6.h"
#include "LCD.h"
#include "uart_log.h"
#include "UBA_battery_performance_test.h"
#include "UBA_line_ADC.h"
#include "UBA_buzzer.h"
#include "rtc.h"
#include <time.h>
#include "UBA_PROTO_helper.h"
#include "UBA_file_manager.h"

#define UBA_SETTINGS_FOLDER "/Settings"
#define UBA_COMP "UBA"
#define UBA_FAN_ON_TEMP (40.0f)
#define UBA_FAN_OFF_TEMP (30.0f)

UBA_6 UBA_6_device_g = UBA_6_DEFUALT;

void UBA_6_init_enter(UBA_6 *uba);
void UBA_6_init(UBA_6 *uba);
void UBA_6_init_exit(UBA_6 *uba);
void UBA_6_single_channels_enter(UBA_6 *uba);
void UBA_6_single_channels(UBA_6 *uba);
void UBA_6_single_channels_exit(UBA_6 *uba);
void UBA_6_dual_channel_enter(UBA_6 *uba);
void UBA_6_dual_channel(UBA_6 *uba);
void UBA_6_dual_channel_exit(UBA_6 *uba);

typedef void (*step_cb_t)(UBA_6 *uba);
/***
 * UBA State Machine Assigner Rule
 */
struct UBASMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};
/*UBA State Machine Assigner */
#define UBASMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

//@formatter:off

static const struct UBASMA_rule rule_g[UBA_6_STATE_MAX] ={
		//				State
		UBASMA(UBA_6_STATE_INIT,				UBA_6_init_enter,				UBA_6_init,			UBA_6_init_exit),
		UBASMA(UBA_6_STATE_SINGLE_CHANNELS,		UBA_6_single_channels_enter,	UBA_6_single_channels,	UBA_6_single_channels_exit),
		UBASMA(UBA_6_STATE_DUAL_CHANNEL,		UBA_6_dual_channel_enter,		UBA_6_dual_channel,	UBA_6_dual_channel_exit),
};
//@formatter:on

#define COPY_MESSAGE_PARAM(param) msg->param = uba->param

//========================private functions ==========================================
bool UBA_6_set_RTC(UBA_6 *uba, uint32_t unix_ts) {
	//TODO: chack that no BPT is ruunig
	struct tm *timeinfo;
	time_t rawtime = (time_t) unix_ts;
	// Convert timestamp to broken-down time (UTC)
	timeinfo = gmtime(&rawtime);
	UART_LOG_DEBUG("RTC Update", "before:%s", get_RTC_date_time_str());
	if (timeinfo == NULL) {
		return false;
	}

	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;

	// Fill date struct (year is offset from 2000)
	sDate.Year = timeinfo->tm_year - 100;  // tm_year is years since 1900
	sDate.Month = timeinfo->tm_mon + 1;    // tm_mon: 0-11
	sDate.Date = timeinfo->tm_mday;
	sDate.WeekDay = ((timeinfo->tm_wday == 0) ? 7 : timeinfo->tm_wday); // RTC: 1=Mon ... 7=Sun

	// Fill time struct
	sTime.Hours = timeinfo->tm_hour;
	sTime.Minutes = timeinfo->tm_min;
	sTime.Seconds = timeinfo->tm_sec;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	HAL_StatusTypeDef res = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	// Set RTC date and time
	if (res != HAL_OK)
		return false;
	res = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	if (res != HAL_OK)
		return false;
	UART_LOG_DEBUG("RTC after", "before:%s", get_RTC_date_time_str());
	return true;
}

void UBA_6_fan_control(UBA_6 *uba) {

	if (uba->isFan_on) {
		if (UBA_BPT_isRunning(&uba->BPT_A) || UBA_BPT_isRunning(&uba->BPT_B) || UBA_BPT_isRunning(&uba->BPT_AB)) {

		} else {
			HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_RESET);
			UART_LOG_WARNNING(UBA_COMP, "=======================FAN Off========================================");
			uba->isFan_on = false;
		}
	} else {
		if (UBA_BPT_isRunning(&uba->BPT_A) || UBA_BPT_isRunning(&uba->BPT_B) || UBA_BPT_isRunning(&uba->BPT_AB)) {
			HAL_GPIO_WritePin(FAN_GPIO_Port, FAN_Pin, GPIO_PIN_SET);
			uba->isFan_on = true;
			UART_LOG_WARNNING(UBA_COMP, "=======================FAN ON========================================");
		} else {
		}
	}
}

//========================state machine functions ==========================================
void UBA_6_update_state(UBA_6 *uba) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", uba->state.current, uba->state.next);
	uba->state.pre = uba->state.current;
	uba->state.current = uba->state.next;
	uba->state.next = UBA_6_STATE_INVALID;
	uba->state.tick = HAL_GetTick();
}

void UBA_6_init_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
	uba->info.firmware.major = UBA_FIRMWARE_MAJOR;
	uba->info.firmware.minor = UBA_FIRMWARE_MINOR;
	uba->info.firmware.patch = UBA_FIRMWARE_PATCH;
	uba->info.firmware.build = UBA_FIRMWARE_BUILD;
	uba->settings.address = UBA_DEFUALT_ADRESS;
	if (UBA_PROTO_load_from_file(UBA_FM_FOLDER_SETTINGS, UBA_FM_FILE_NAME_SETTINGS, UBA_PROTO_UBA6_settings_fields, &uba->settings) == false) {
		UBA_PROTO_save_to_file(UBA_FM_FOLDER_SETTINGS, UBA_FM_FILE_NAME_SETTINGS, UBA_PROTO_UBA6_settings_fields, &uba->settings);
	}
	UBA_TR_unpack(&TR_file.list[0], &uba->BPT_A);
	UBA_TR_unpack(&TR_file.list[1], &uba->BPT_B);
	UBA_TR_unpack(&TR_file.list[2], &uba->BPT_AB);
	if (uba->settings.buzzer) {
		buzzer_g.state.next = UBA_BUZZER_STATE_OFF;
	} else {
		buzzer_g.state.next = UBA_BUZZER_STATE_MUTE;
	}
	if (uba->settings.address == 0) {
		UART_LOG_CRITICAL(UBA_COMP, "UBA Address init with zero ,set default address ");
		uba->settings.address = UBA_DEFUALT_ADRESS;
	}

}
void UBA_6_init(UBA_6 *uba) {
	UBA_6_set_next_state(uba, UBA_6_STATE_SINGLE_CHANNELS);
}
void UBA_6_init_exit(UBA_6 *uba) {
	UBA_line_get_line_data(&UBA_LINE_A);
	UART_LOG_INFO(UBA_COMP, "Device init Done At: %s", get_RTC_date_time_str());
}
void UBA_6_single_channels_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
	UBA_LCD_g.state.next = UBA_LCD_STATE_SIDE_BY_SIDE;
}

void UBA_6_single_channels(UBA_6 *uba) {
	if (uba->BPT_A.type == UBA_BPT_TYPE_SINGLE_CHANNEL) {
		UBA_BPT_run(&uba->BPT_A);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "A Test is not single channel");
	}
	if (uba->BPT_B.type == UBA_BPT_TYPE_SINGLE_CHANNEL) {
		UBA_BPT_run(&uba->BPT_B);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "B Test is not single channel");
	}
	UBA_6_fan_control(uba);

}

void UBA_6_single_channels_exit(UBA_6 *uba) {
}
void UBA_6_dual_channel_enter(UBA_6 *uba) {
	UBA_6_update_state(uba);
	UBA_LCD_g.state.next = UBA_LCD_STATE_FULL_SCREEN;
	UART_LOG_WARNNING(UBA_COMP, "=======================HV Enable Off========================================");
}

void UBA_6_dual_channel(UBA_6 *uba) {
	if (uba->BPT_AB.type == UBA_BPT_TYPE_DUAL_CHANNEL) {
		UBA_BPT_run(&uba->BPT_AB);
	} else {
		UART_LOG_CRITICAL(UBA_COMP, "AB Test is not dual channel");
	}
	UBA_6_fan_control(uba);
}

void UBA_6_dual_channel_exit(UBA_6 *uba) {
}

UBA_6_ERROR UBA_6_high_voltage_enable(UBA_6 *uba, UBA_6_HV_CONSUMER c) {
	if (c > UBA_6_HV_CONSUMER_MAX) {
		return UBA_6_ERROR_PARAM;
	}
	uba->high_voltage_consumers |= c;
	if (uba->high_voltage_consumers != UBA_6_HV_CONSUMER_NONE) {
	}
	return UBA_6_ERROR_NO_ERROR;
}

UBA_6_ERROR UBA_6_high_voltage_disable(UBA_6 *uba, UBA_6_HV_CONSUMER c) {
	if (c > UBA_6_HV_CONSUMER_MAX) {
		return UBA_6_ERROR_PARAM;
	}
	uba->high_voltage_consumers &= ~c; // clear selected consumers
	if (uba->high_voltage_consumers == UBA_6_HV_CONSUMER_NONE) {
	}
	return UBA_6_ERROR_NO_ERROR;
}

//========================public  functions ==========================================
void UBA_6_run(UBA_6 *uba) {
	if (uba->state.next == UBA_6_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[uba->state.current].run) {
			rule_g[uba->state.current].run(uba); // run the main function of the state
		}
	} else {
		if (uba->state.current < UBA_6_STATE_MAX) {
			if (rule_g[uba->state.current].exit) {
				rule_g[uba->state.current].exit(uba); // run the status exit function
			}
		}
		if (rule_g[uba->state.next].enter) {
			rule_g[uba->state.next].enter(uba); // run the next state enter function
		}
	}
}

UBA_6_ERROR UBA_6_set_next_state(UBA_6 *uba, UBA_6_STATE next_state) {
	UBA_6_ERROR ret = UBA_6_ERROR_NO_ERROR;
	if (next_state < UBA_6_STATE_MAX) {
		uba->state.next = next_state;
	} else {
		ret = UBA_6_ERROR_PARAM;
		UART_LOG_ERROR(UBA_COMP, "State Value is invalid %u ", next_state);
	}
	UART_LOG_DEBUG(UBA_COMP, "%s Set Next State Value  %u ", uba->settings.name, next_state);
	return ret;
}

void UBA_6_update_message(UBA_6 *uba, UBA_PROTO_UBA6_status *msg) {
	uba->info.state = uba->state.current;
	memcpy(&msg->settings, &uba->settings, sizeof(UBA_PROTO_UBA6_settings));
	memcpy(&msg->info, &uba->info, sizeof(UBA_PROTO_UBA6_info));
}

void UBA_6_command_execute(UBA_6 *uba, UBA_PROTO_UBA6_command *cmd) {
	switch (cmd->id) {
		case UBA_PROTO_UBA6_CMD_ID_TEST:
			UART_LOG_DEBUG(UBA_COMP, "%s Test Command", uba->settings.name);
			break;
		case UBA_PROTO_UBA6_CMD_ID_BUZZER:
			uba->settings.buzzer = cmd->value;
			if (uba->settings.buzzer) {
				buzzer_g.state.next = UBA_BUZZER_STATE_OFF;
			} else {
				buzzer_g.state.next = UBA_BUZZER_STATE_MUTE;
			}
			break;
		case UBA_PROTO_UBA6_CMD_ID_MODE:
			UBA_6_set_next_state(uba, cmd->value);
			break;
		case UBA_PROTO_UBA6_CMD_ID_ADDRESSS:
			uba->settings.address = 0x01 << cmd->value;
			if (uba->settings.address == 0) {
				UART_LOG_ERROR(UBA_COMP, "Invalid Adress %u ", uba->settings.address);
				uba->settings.address = UBA_DEFUALT_ADRESS;
			}
			UART_LOG_WARNNING(UBA_COMP, "Address changed to %08x ", uba->settings.address);
			break;
		case UBA_PROTO_UBA6_CMD_ID_NAME:
			memcpy(uba->settings.name, cmd->name, strlen(cmd->name) < UBA_MAX_NAME_SIZE ? strlen(cmd->name) : UBA_MAX_NAME_SIZE - 1);
			UART_LOG_DEBUG(UBA_COMP, "Name was changed to %s", uba->settings.name);
			break;
		case UBA_PROTO_UBA6_CMD_ID_TIME:
			UBA_6_set_RTC(uba, cmd->value);
			UART_LOG_DEBUG(UBA_COMP, "Time was changed to %lu", cmd->value);
			break;
		case UBA_PROTO_UBA6_CMD_ID_BOOT:
			NVIC_SystemReset();
			break;
		case UBA_PROTO_UBA6_CMD_ID_SN:
			uba->settings.SN = cmd->value;
			UART_LOG_WARNNING(UBA_COMP, "SN changed to %08x ", uba->settings.SN);
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "Invalid Command ID %u ", cmd->id);
			break;
	}
	UBA_PROTO_save_to_file(UBA_FM_FOLDER_SETTINGS, UBA_FM_FILE_NAME_SETTINGS, UBA_PROTO_UBA6_settings_fields, &uba->settings);

}
