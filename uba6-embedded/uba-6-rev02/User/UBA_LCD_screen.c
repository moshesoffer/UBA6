/*
 * UBA_LCD_screen.c
 *
 *  Created on: Sep 22, 2024
 *      Author: ORA
 */

#include "UBA_LCD_screen.h"

#include "stdio.h"
#include "stdlib.h"
#include "5x5_font.h"
#include "ST7789_GFX.h"
#include "ST7789_STM32_Driver.h"
#include "uart_log.h"
#include "string.h"
#include "rtc.h"

#include "UBA_6.h"
#include "UBA_battery_performance_test.h"
#include "UBA_util.h"
#include "UBA_test_routine.h"
#include "LCD.h"

#define UBA_COMP "SCREEN"

#define START_X (0)
#define START_Y (0)
#define LCD_DATA_FONT_SIZE (2)
#define BORDER_PADDING (5) /*5 pixel border padding*/
#define LINE_H 	(CHAR_HEIGHT + 2) /*2 pixel from line to line*/
#define LINE0_Y (START_Y+BORDER_PADDING)
#define LINE2_Y (LINE0_Y + LINE_H)
#define LINE3_Y (LINE2_Y + LINE_H)
#define DATA_FIRST_LINE (4)
#define UBA_LCD_SLOW_REFRESH_TIME (1000)
#define UBA_LCD_FAST_REFRESH_TIME (300)

// @formatter:off
#define CHANNEL_DISPALY_STATUS "%-10s"
#define CHANNEL_DISPALY_DATA_PAD "%11.*s"
#define CHANNEL_DISPALY_DATA "%.2f %3s"


#define LINE_CHANEL_NAME 			(0)
#define LINE_CHANEL_NAME_FONT_SIZE 	(3)
#define LINE_TEST_NAME 				(LINE_CHANEL_NAME+LINE_CHANEL_NAME_FONT_SIZE+1)
#define LINE_TEST_NAME_FONT_SIZE 	(2)
#define LINE_STEP 					(LINE_TEST_NAME+LINE_TEST_NAME_FONT_SIZE)
#define LINE_STEP_FONT_SIZE 		(1)
#define LINE_TIME 					(LINE_STEP+LINE_STEP_FONT_SIZE)
#define LINE_TIME_FONT_SIZE			(2)
#define LINE_V 						(LINE_TIME+LINE_TIME_FONT_SIZE)
#define LINE_V_FONT_SIZE			(2)
#define LINE_C 						(LINE_V+LINE_V_FONT_SIZE)
#define LINE_C_FONT_SIZE			(2)
#define LINE_CAP 					(LINE_C+LINE_C_FONT_SIZE)
#define LINE_CAP_FONT_SIZE			(2)
#define LINE_TEMP 					(LINE_CAP+LINE_CAP_FONT_SIZE)
#define LINE_TEMP_FONT_SIZE			(2)
#define LINE_EWI 					(LINE_TEMP+LINE_TEMP_FONT_SIZE)
#define LINE_EWI_FONT_SIZE			(2)

#define LINE(x) ((LINE_H*x) + LINE0_Y)
#define UBA_LCD_MAX_DISPLAY_TEST_SELECT (7)
#define UBA_EWI_MAX_LINE_CHAR_SIZE (12)

#define UBA_LCD_SCREEN_SINGEL_CH_CHAR (179)
#define UBA_LCD_SCREEN_DUAL_CH_CHAR (186)




void UBA_LCD_screen_draw_bpt(UBA_LCD_screen* screen, UBA_LCD_REFRESH_TYPE rt);

void UBA_LCD_screen_update_state(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_init_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_init(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_init_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_channel_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_channel(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_channel_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_bpt_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_bpt(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_bpt_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_test_select_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_select(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_select_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_test_info_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_info(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_info_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_setting_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_setting(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_setting_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_off_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_off(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_off_exit(UBA_LCD_screen *screen);

typedef void (*step_cb_t)(UBA_LCD_screen *screen);

/***
 * UBA LCD State Machine Assigner Rule
 */
struct UBABPTSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};

/*UBA LCD Screen State Machine Assigner */
#define UBABPTSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

// @formatter:off
static const struct UBABPTSMA_rule rule_g[UBA_LCD_SCREEN_STATE_MAX] ={
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_INIT,			UBA_LCD_screen_display_init_enter,			UBA_LCD_screen_display_init,		UBA_LCD_screen_display_init_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_CHANNEL,		UBA_LCD_screen_display_channel_enter,		UBA_LCD_screen_display_channel,		UBA_LCD_screen_display_channel_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_BPT,			UBA_LCD_screen_display_bpt_enter,			UBA_LCD_screen_display_bpt,			UBA_LCD_screen_display_bpt_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_TEST_SELECT,	UBA_LCD_screen_display_test_select_enter,	UBA_LCD_screen_display_test_select,	UBA_LCD_screen_display_test_select_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_TEST_INFO,		UBA_LCD_screen_display_test_info_enter,		UBA_LCD_screen_display_test_info,	UBA_LCD_screen_display_test_info_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_SETTING,		UBA_LCD_screen_display_setting_enter,		UBA_LCD_screen_display_setting,		UBA_LCD_screen_display_setting_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_OFF,			UBA_LCD_screen_display_off_enter,			UBA_LCD_screen_display_off,			UBA_LCD_screen_display_off_exit),
};
// @formatter:on
//=================================================private functions========================================================//

void UBA_LCD_screen_update_state(UBA_LCD_screen *screen) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", screen->state.current, screen->state.next);
	screen->state.pre = screen->state.current;
	screen->state.current = screen->state.next;
	screen->state.next = UBA_LCD_SCREEN_STATE_INVALID;
}

void UBA_LCD_screen_bnt_press_up_or_down(UBA_LCD_screen *screen, UBA_LCD_page_BPT *lcd_ch) {
	if (lcd_ch->bnt_pause_start.effect == UBA_GFX_EFFECT_SELECTED) {
		lcd_ch->bnt_pause_start.effect = UBA_GFX_EFFECT_VISIBLE;
		lcd_ch->bnt_back_stop.effect = UBA_GFX_EFFECT_SELECTED;
	} else if (lcd_ch->bnt_back_stop.effect == UBA_GFX_EFFECT_SELECTED) {
		lcd_ch->bnt_back_stop.effect = UBA_GFX_EFFECT_VISIBLE;
		lcd_ch->bnt_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
	}
	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
}
/*
 * return the number of button in Panding mode
 * */
int UBA_LCD_screen_isPanding(UBA_LCD_screen *screen) {
	int ret = 0;
	if (UBA_button_is_pending(screen->main_buttons.bnt_up_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->main_buttons.bnt_down_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->main_buttons.bnt_select_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.bnt_up_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.bnt_down_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		ret++;
	}
	return ret;
}

//TODO: use in Util
uint32_t TimeToSeconds(RTC_TimeTypeDef *time) {
	return time->Hours * 3600 + time->Minutes * 60 + time->Seconds;
}
void UBA_LCD_screen_load_channel(UBA_LCD_channel *lcd_ch, UBA_channel *ch) {
	float data_vlaue = 0;
	float temp_value;
	char buffer[25] = { 0 };

	if (strcmp(lcd_ch->ch_name.elemnt.text.text, ch->name) != 0) {
		switch (ch->id) {
			case UBA_CHANNLE_ID_A:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH A");
				break;
			case UBA_CHANNLE_ID_B:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH B");
				break;
			case UBA_CHANNLE_ID_AB:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "Channel AB");
				break;
			default:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH N/A");
		}
		//update shadoe
		ch->shadow.ch_name_changed = true;
	}
	if (ch->state.current != ch->shadow.state) {
		switch (ch->state.current) {
			case UBA_CHANNEL_STATE_INIT:
				lcd_ch->status.effect = UBA_GFX_EFFECT_SOLID;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_DELAY;
				sprintf(lcd_ch->status.elemnt.status.text, CHANNEL_DISPALY_STATUS, "   INIT   ");
				break;
			case UBA_CHANNEL_STATE_DELAY:
				lcd_ch->status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_DELAY;
				sprintf(lcd_ch->status.elemnt.status.text, CHANNEL_DISPALY_STATUS, "   DELAY  ");
				break;
			case UBA_CHANNEL_STATE_STANDBY:
				lcd_ch->status.effect = UBA_GFX_EFFECT_SOLID;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_STANDBY;
				sprintf(lcd_ch->status.elemnt.status.text, CHANNEL_DISPALY_STATUS, "  STANDBY ");
				break;
			case UBA_CHANNEL_STATE_CHARGE:
				lcd_ch->status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_RUN;
				sprintf(lcd_ch->status.elemnt.status.text, "%-10s", " CHARGING ");
				break;
			case UBA_CHANNEL_STATE_DISCHARGE:
				lcd_ch->status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_RUN;
				sprintf(lcd_ch->status.elemnt.status.text, "%-10s", " DISCHARGE");
				break;
			case UBA_CHANNEL_STATE_OFF:
				lcd_ch->status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_OFF;
				sprintf(lcd_ch->status.elemnt.status.text, "%-10s", "N/A");
				break;
			default:
				UART_LOG_ERROR(UBA_COMP, "channel State :%u Unknown ", ch->state.current);
		}

		//update shadow
		ch->shadow.state = ch->state.current;
	}

	lcd_ch->shadow.volt_vlaue_changed = false;
	data_vlaue = UBA_channel_get_voltage(ch);
	if (data_vlaue != lcd_ch->shadow.volt_vlaue) {
		if (data_vlaue >= 999) {
			data_vlaue /= 1000.0f;
			if (data_vlaue >= 999) {
				sprintf(buffer, "%3f %-3s", data_vlaue, "V");
			} else {
				sprintf(buffer, "%3.2f %-3s", data_vlaue, "V");
			}
		} else {
			sprintf(buffer, "%.2f %-3s", data_vlaue, "mV");
		}
		lcd_ch->volt.effect = UBA_GFX_EFFECT_SOLID;
		snprintf(lcd_ch->volt.elemnt.text.text, 12, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.volt_vlaue = data_vlaue;
		lcd_ch->shadow.volt_vlaue_changed = true;
	}

	lcd_ch->shadow.current_vlaue_changed = false;
	data_vlaue = UBA_channel_get_current(ch);
	if (data_vlaue != lcd_ch->shadow.current_vlaue) {
		if (abs(data_vlaue) >= 2000.0f) {
			data_vlaue /= 1000.0f;
			sprintf(buffer, "%.2f %-3s", data_vlaue, "A");
		} else {
			sprintf(buffer, "%.2f %-3s", data_vlaue, "mA");
		}
		lcd_ch->current.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_ch->current.elemnt.text.text, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.current_vlaue = data_vlaue;
		lcd_ch->shadow.current_vlaue_changed = true;
	}

	lcd_ch->shadow.capacity_vlaue_changed = false;
	data_vlaue = UBA_channel_get_capacity(ch);
	if (data_vlaue != lcd_ch->shadow.capacity_vlaue) {
		if (abs(data_vlaue) >= 1000) {
			data_vlaue /= 1000.0f;
			sprintf(buffer, "%.2f %-3s", data_vlaue, "Ah");
		} else {
			sprintf(buffer, "%.2f %-3s", data_vlaue, "mAh");
		}
		lcd_ch->capacity.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_ch->capacity.elemnt.text.text, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.capacity_vlaue = data_vlaue;
		lcd_ch->shadow.capacity_vlaue_changed = true;
	}

	lcd_ch->shadow.temp_value_changed = false;
	temp_value = UBA_channel_get_temperature(ch);
	if (temp_value != lcd_ch->shadow.temp_value) {
		sprintf(buffer, "%+3.2f\xf8 %2s", temp_value, "C");
		lcd_ch->temp.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_ch->temp.elemnt.text.text, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.temp_value = temp_value;
		lcd_ch->shadow.temp_value_changed = true;
	}

}

void UBA_LCD_screen_draw_channel(UBA_LCD_screen *screen, UBA_LCD_REFRESH_TYPE rt) {
	UBA_channel *ch;
	UBA_BPT_SHADOW *lcd_bpt_shadow = &(screen->bpt->shadow);
	UBA_LCD_channel_shadow *lcd_channel_shadow = &screen->pages.channel.channel.shadow;
	bool test_name_changed = false;

	switch (screen->ch_control) {
		case UBA_CHANNLE_ID_A:
			ch = &UBA_CH_A;
			break;
		case UBA_CHANNLE_ID_B:
			ch = &UBA_CH_B;
			break;
		case UBA_CHANNLE_ID_AB:
			ch = &UBA_CH_AB;
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "Screen channel contorl not define");
			ch = &UBA_CH_AB;
	}

	UBA_LCD_screen_load_channel(&screen->pages.channel.channel, ch);

	if (lcd_bpt_shadow->error != ch->error)
	{
		if (ch->error) {
			//TODO: add EWI
			sprintf(screen->pages.channel.EWI_msg.elemnt.text.text, "Error:%x", ch->error);
		} else {
			sprintf(screen->pages.channel.EWI_msg.elemnt.text.text, "        ");
		}

		//update shadow
		lcd_bpt_shadow->error = ch->error;
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&screen->pages.channel.frame);
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		if (ch->shadow.ch_name_changed == true) {
			UBA_GFX_draw_text(&screen->pages.channel.channel.ch_name);
			//update shadow
			ch->shadow.ch_name_changed = false;
		}
		if (strcmp(screen->pages.screen_bpt.test_name.elemnt.text.text, lcd_bpt_shadow->test_name) != 0) {
			UBA_GFX_draw_text_center(&screen->pages.screen_bpt.test_name);
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_STATUS) == UBA_LCD_REFRESH_TYPE_STATUS) {
		//if (lcd_bpt_shadow->current_state != ch->state.current) {
			UBA_GFX_draw_status(&screen->pages.channel.channel.status);
			//update shadow
			lcd_bpt_shadow->current_state = ch->state.current;
		//}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_DATA) == UBA_LCD_REFRESH_TYPE_DATA) {
		if (lcd_channel_shadow->volt_vlaue_changed == true) {
			UBA_GFX_draw_text(&screen->pages.channel.channel.volt);
			//update shadow
			lcd_channel_shadow->volt_vlaue_changed = false;
		}
		if (lcd_channel_shadow->current_vlaue_changed == true) {
			UBA_GFX_draw_text(&screen->pages.channel.channel.current);
			//update shadow
			lcd_channel_shadow->current_vlaue_changed = false;
		}
		if (lcd_channel_shadow->capacity_vlaue_changed == true) {
			UBA_GFX_draw_text(&screen->pages.channel.channel.capacity);
			//update shadow
			lcd_channel_shadow->capacity_vlaue_changed = false;
		}
		if (lcd_channel_shadow->temp_value_changed == true) {
			UBA_GFX_draw_text(&screen->pages.channel.channel.temp);
			//update shadow
			lcd_channel_shadow->temp_value_changed = false;
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_EWI) == UBA_LCD_REFRESH_TYPE_EWI) {
		if (lcd_bpt_shadow->current_state != ch->state.current) {
			if (lcd_bpt_shadow->error != screen->bpt->error) {
				UBA_GFX_draw_text_center(&screen->pages.channel.EWI_msg);
				//update shadow
				lcd_bpt_shadow->error = screen->bpt->error;
			}
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		if ((strcmp(lcd_bpt_shadow->bnt_select.text, screen->pages.channel.bnt_select.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->bnt_select.color_bg != screen->pages.channel.bnt_select.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->bnt_select.color_text != screen->pages.channel.bnt_select.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->bnt_select.effect != screen->pages.channel.bnt_select.effect)) {
			UBA_GFX_draw_button(&screen->pages.channel.bnt_select);
			//update shadow
			strcpy(lcd_bpt_shadow->bnt_select.text, screen->pages.channel.bnt_select.elemnt.button.text);
		    lcd_bpt_shadow->bnt_select.color_bg = screen->pages.channel.bnt_select.elemnt.button.color_bg;
		    lcd_bpt_shadow->bnt_select.color_text = screen->pages.channel.bnt_select.elemnt.button.color_text;
		    lcd_bpt_shadow->bnt_select.effect = screen->pages.channel.bnt_select.effect;
		}
	}
}

void UBA_LCD_screen_draw_bpt(UBA_LCD_screen *screen, UBA_LCD_REFRESH_TYPE rt) {
	UBA_BPT *channel_test;
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;
	uint32_t time1_seconds;
	uint32_t time2_seconds;
	uint32_t diff_seconds;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	UBA_LCD_page_BPT *lcd_bpt = &(screen->pages.screen_bpt);
	UBA_BPT_SHADOW *lcd_bpt_shadow = &(screen->bpt->shadow);
	UBA_LCD_channel_shadow *lcd_channel_shadow = &(lcd_bpt->channel.shadow);
	UBA_channel *ch = screen->bpt->ch;
	bool test_name_changed = false;

	UBA_LCD_screen_load_channel(&screen->pages.screen_bpt.channel, ch);
	//frame update
	lcd_bpt->frame.effect = UBA_GFX_EFFECT_SOLID;

	// channel Name update

	// Test
	if (((screen->bpt)) != NULL) {
		channel_test = screen->bpt;

		if (strcmp(lcd_bpt->test_name.elemnt.text.text, lcd_bpt_shadow->test_name) != 0) {
			if (strlen(channel_test->name)) {
				sprintf(lcd_bpt->test_name.elemnt.text.text, "%.*s",
						UBA_LCD_MIN((int)strlen(channel_test->name),
								(lcd_bpt->frame.elemnt.frame.width - (BORDER_PADDING * 2)) / (CHAR_WIDTH * lcd_bpt->test_name.elemnt.text.size))
								, channel_test->name);
				lcd_bpt->test_name.effect = UBA_GFX_EFFECT_SOLID;
				//update shadow
				test_name_changed = true;
				sprintf(lcd_bpt_shadow->test_name, lcd_bpt->test_name.elemnt.text.text);
			} else {
				lcd_bpt->test_name.effect = UBA_GFX_EFFECT_INVISIBLE;
			}
		}

		if (lcd_bpt_shadow->current_state != channel_test->state.current)
		{
			if (UBA_BPT_isRunning(channel_test)) {
				lcd_bpt->test_step.effect = UBA_GFX_EFFECT_SOLID;
				sprintf(lcd_bpt->test_step.elemnt.text.text, "%02u/%02u", channel_test->current_step->step_index, channel_test->last_step_index);
			} else {
				lcd_bpt->test_step.effect = UBA_GFX_EFFECT_INVISIBLE;
			}
			lcd_bpt->test_step.effect = UBA_GFX_EFFECT_SOLID;
			sprintf(lcd_bpt->test_step.elemnt.text.text, "%02u/%02u", channel_test->current_step->step_index, channel_test->last_step_index);
		}
	}
	else
	{
		screen->bpt->error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
		if (lcd_bpt_shadow->error != screen->bpt->error)
		{
			UART_LOG_ERROR(UBA_COMP, "channel control id %u in unknoun ", screen->ch_control);
			lcd_bpt_shadow->error = screen->bpt->error;
		}
		return;
	}

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	lcd_bpt->time.effect = UBA_GFX_EFFECT_SOLID;
	time1_seconds = TimeToSeconds(&screen->bpt->start_date_time.time);
	time2_seconds = TimeToSeconds(&sTime);
	if (time2_seconds >= time1_seconds) {
		diff_seconds = time2_seconds - time1_seconds;
	} else {
		// Assuming the difference is within 24 hours, account for crossing midnight
		diff_seconds = (24 * 3600) - (time1_seconds - time2_seconds);
	}
	hours = diff_seconds / 3600;
	minutes = (diff_seconds % 3600) / 60;
	seconds = diff_seconds % 60;

	if (lcd_bpt_shadow->current_state != channel_test->state.current)
	{
		if (UBA_BPT_isRunning(channel_test)) {
			snprintf(lcd_bpt->time.elemnt.text.text, 9, "%02u:%02u:%02u", hours, minutes, seconds);
		} else {
			snprintf(lcd_bpt->time.elemnt.text.text, 9, "%02u:%02u:%02u", 0, 0, 0);
		}

		if (channel_test->state.current == UBA_BPT_STATE_PAUSE) {

		} else if (channel_test->state.current == UBA_BPT_STATE_TEST_FAILED) {

		}
	}

// EWI line

	if (lcd_bpt_shadow->error != screen->bpt->error)
	{
		if (screen->bpt->error & UBA_BPT_CRITICAL) {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_YELLOW;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_RED;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
		} else if (screen->bpt->error & UBA_BPT_ERROR)  {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_RED;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, 10, "ERR:%04u", screen->bpt->error);
		} else if (screen->bpt->error & UBA_BPT_WARNNING)  {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_YELLOW;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, "WARN:%04u", screen->bpt->error);
		} else {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_WHITE;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, "            ");
		}

		//update shadow
		lcd_bpt_shadow->error = screen->bpt->error;
	}

	if (lcd_bpt_shadow->current_state != channel_test->state.current)
	{
		if (screen->bpt->state.current == UBA_BPT_STATE_TEST_COMPLEATE) {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_GREEN;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, " Completed ");
		}
		if (screen->bpt->state.current == UBA_BPT_STATE_TEST_FAILED) {
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, " FAILED ");
		}
	}

	if (lcd_bpt_shadow->current_state != channel_test->state.current)
	{
		if ((screen->bpt->error & UBA_PROTO_UBA6_ERROR_USER_ABORT) == UBA_PROTO_UBA6_ERROR_USER_ABORT) {
			snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, "USER STOPED");
		}
	}

	if (lcd_bpt_shadow->current_state != channel_test->state.current)
	{
		switch (screen->bpt->state.current) {
			case UBA_BPT_STATE_PAUSE:
				sprintf(lcd_bpt->channel.status.elemnt.status.text, "PAUSE");
				lcd_bpt->channel.status.elemnt.status.color_fill = UBA_GFX_COLOR_YELLOW;
				lcd_bpt->channel.status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				break;
			case UBA_BPT_STATE_TEST_FAILED:
				lcd_bpt->channel.status.effect = UBA_GFX_EFFECT_SOLID;
				if ((screen->bpt->error & UBA_PROTO_UBA6_ERROR_USER_ABORT) == UBA_PROTO_UBA6_ERROR_USER_ABORT) {

					sprintf(lcd_bpt->channel.status.elemnt.status.text, "ABORT");
				} else {
					sprintf(lcd_bpt->channel.status.elemnt.status.text, "FAIL");
				}
				lcd_bpt->channel.status.elemnt.status.color_fill = UBA_GFX_COLOR_RED;
				break;
			default:
				break;

		}
	}

	if (UBA_BPT_isPause(screen->bpt)) {
		sprintf(lcd_bpt->bnt_pause_start.elemnt.button.text, "RESUM");
		sprintf(lcd_bpt->bnt_back_stop.elemnt.button.text, "STOP");
	} else if (UBA_BPT_isRunning(screen->bpt)) {
		sprintf(lcd_bpt->bnt_pause_start.elemnt.button.text, "PAUSE");
		sprintf(lcd_bpt->bnt_back_stop.elemnt.button.text, "STOP");
	} else {
		sprintf(lcd_bpt->bnt_pause_start.elemnt.button.text, "START");
		sprintf(lcd_bpt->bnt_back_stop.elemnt.button.text, "BACK");
	}
	lcd_bpt->bnt_back_stop.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	lcd_bpt->bnt_pause_start.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;

	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&lcd_bpt->frame);
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		if (ch->shadow.ch_name_changed == true) {
			UBA_GFX_draw_text(&lcd_bpt->channel.ch_name);
			//update shadow
			ch->shadow.ch_name_changed = false;
		}
		if (test_name_changed == true) {
			UBA_GFX_draw_text_center(&lcd_bpt->test_name);
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_STATUS) == UBA_LCD_REFRESH_TYPE_STATUS) {
		if (lcd_bpt_shadow->current_state != channel_test->state.current) {
			UBA_GFX_draw_status(&lcd_bpt->channel.status);
			UBA_GFX_draw_text_center(&lcd_bpt->test_step);
			//update shadow
			lcd_bpt_shadow->current_state = ch->state.current;
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_DATA) == UBA_LCD_REFRESH_TYPE_DATA) {
		UBA_GFX_draw_text_center(&lcd_bpt->time);
		if (lcd_channel_shadow->volt_vlaue_changed == true) {
			UBA_GFX_draw_text(&lcd_bpt->channel.volt);
			//update shadow
			lcd_channel_shadow->volt_vlaue_changed = false;
		}
		if (lcd_channel_shadow->current_vlaue_changed == true) {
			UBA_GFX_draw_text(&lcd_bpt->channel.current);
			//update shadow
			lcd_channel_shadow->current_vlaue_changed = false;
		}
		if (lcd_channel_shadow->capacity_vlaue_changed == true) {
			UBA_GFX_draw_text(&lcd_bpt->channel.capacity);
			//update shadow
			lcd_channel_shadow->capacity_vlaue_changed = false;
		}
		if (lcd_channel_shadow->temp_value_changed == true) {
			UBA_GFX_draw_text(&lcd_bpt->channel.temp);
			//update shadow
			lcd_channel_shadow->temp_value_changed = false;
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_EWI) == UBA_LCD_REFRESH_TYPE_EWI) {
		if (lcd_bpt_shadow->current_state != channel_test->state.current) {
			if (lcd_bpt_shadow->error != screen->bpt->error) {
				UBA_GFX_draw_text_center(&lcd_bpt->EWI_msg);
			}
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		if ((strcmp(lcd_bpt_shadow->bnt_back_stop.text, lcd_bpt->bnt_back_stop.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->bnt_back_stop.color_bg != lcd_bpt->bnt_back_stop.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->bnt_back_stop.color_text != lcd_bpt->bnt_back_stop.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->bnt_back_stop.effect != lcd_bpt->bnt_back_stop.effect)) {
			UBA_GFX_draw_button(&lcd_bpt->bnt_back_stop);
			//update shadow
			strcpy(lcd_bpt_shadow->bnt_back_stop.text, lcd_bpt->bnt_back_stop.elemnt.button.text);
		    lcd_bpt_shadow->bnt_back_stop.color_bg = lcd_bpt->bnt_back_stop.elemnt.button.color_bg;
		    lcd_bpt_shadow->bnt_back_stop.color_text = lcd_bpt->bnt_back_stop.elemnt.button.color_text;
		    lcd_bpt_shadow->bnt_back_stop.effect = lcd_bpt->bnt_back_stop.effect;
		}
		if ((strcmp(lcd_bpt_shadow->bnt_pause_start.text, lcd_bpt->bnt_pause_start.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->bnt_pause_start.color_bg != lcd_bpt->bnt_pause_start.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->bnt_pause_start.color_text != lcd_bpt->bnt_pause_start.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->bnt_pause_start.effect != lcd_bpt->bnt_pause_start.effect)) {
			UBA_GFX_draw_button(&lcd_bpt->bnt_pause_start);
			//update shadow
			strcpy(lcd_bpt_shadow->bnt_pause_start.text, lcd_bpt->bnt_pause_start.elemnt.button.text);
		    lcd_bpt_shadow->bnt_pause_start.color_bg = lcd_bpt->bnt_pause_start.elemnt.button.color_bg;
		    lcd_bpt_shadow->bnt_pause_start.color_text = lcd_bpt->bnt_pause_start.elemnt.button.color_text;
		    lcd_bpt_shadow->bnt_pause_start.effect = lcd_bpt->bnt_pause_start.effect;
		}
	}

	//update shadow
	lcd_bpt_shadow->current_state = channel_test->state.current;
	lcd_bpt_shadow->error = screen->bpt->error;
}

/**
 * return true is back is presses
 */
bool UBA_LCD_screen_bnt_press_select(UBA_LCD_screen *screen) {

	if (screen->pages.screen_bpt.bnt_pause_start.state == UBA_GFX_STATE_SELECTED) {
		if (UBA_BPT_isPause(screen->bpt)) {
			UBA_BPT_start(screen->bpt);
		} else if (UBA_BPT_isRunning(screen->bpt)) {
			UBA_BPT_pause_test(screen->bpt);
		} else {
			UBA_BPT_start(screen->bpt);
		}
	} else if (screen->pages.screen_bpt.bnt_back_stop.state == UBA_GFX_STATE_SELECTED) {
		if (UBA_BPT_isRunning(screen->bpt)) {
			UBA_BPT_stop(screen->bpt);
		} else {
			return true; // go back
		}
	}
	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
	return false;
}
int UBA_LCD_screen_line_max_str_length(UBA_LCD_screen *screen, uint8_t size) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;

	int ret = ((position[screen->ch_control-1].width - (BORDER_PADDING * 2)) / (CHAR_WIDTH * size)) - 1;
	UART_LOG_DEBUG(UBA_COMP, "the max number of char in screen size is %d", ret);
	return ret;
}

void UBA_LCD_screen_display_test_select_refresh(UBA_LCD_page_test_list_select *lcd_test_select, UBA_LCD_REFRESH_TYPE rt) {
	int i = 0;
	lcd_test_select->frame.effect = UBA_GFX_EFFECT_SOLID;

	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&lcd_test_select->frame);
	}
	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		UBA_GFX_draw_text_center(&lcd_test_select->title);
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
			if (i == (lcd_test_select->select_index)) {
				lcd_test_select->test_name_list[i].effect = UBA_GFX_EFFECT_SELECTED;
			} else {
				lcd_test_select->test_name_list[i].effect = UBA_GFX_EFFECT_SOLID;
			}
			UBA_GFX_draw_text(&lcd_test_select->test_name_list[i]);
		}

		UBA_GFX_draw_button(&lcd_test_select->bnt_cancel);
	}
}

//====================================================state machine functions============================================//

void UBA_LCD_screen_display_init_enter(UBA_LCD_screen *screen) {
	UBA_LCD_screen_update_state(screen);
	switch (screen->ch_control) {
		case UBA_CHANNLE_ID_A:
			screen->main_buttons.bnt_up_p = &UBA_BTN_CH_1_UP;
			screen->main_buttons.bnt_down_p = &UBA_BTN_CH_1_DOWN;
			screen->main_buttons.bnt_select_p = &UBA_BTN_CH_1_SELECT;
			break;
		case UBA_CHANNLE_ID_B:
			screen->main_buttons.bnt_up_p = &UBA_BTN_CH_2_UP;
			screen->main_buttons.bnt_down_p = &UBA_BTN_CH_2_DOWN;
			screen->main_buttons.bnt_select_p = &UBA_BTN_CH_2_SELECT;
			break;
		case UBA_CHANNLE_ID_AB:
			screen->main_buttons.bnt_up_p = &UBA_BTN_CH_1_UP;
			screen->main_buttons.bnt_down_p = &UBA_BTN_CH_1_DOWN;
			screen->main_buttons.bnt_select_p = &UBA_BTN_CH_1_SELECT;
			screen->secondery_buttons.bnt_up_p = &UBA_BTN_CH_2_UP;
			screen->secondery_buttons.bnt_down_p = &UBA_BTN_CH_2_DOWN;
			screen->secondery_buttons.bnt_select_p = &UBA_BTN_CH_2_SELECT;
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "channel control id %u in unknoun ", screen->ch_control);
	}

}

void UBA_LCD_screen_display_init(UBA_LCD_screen *screen) {
	if (UBA_BPT_isUnpacked(screen->bpt)) {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
	} else {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_CHANNEL;
	}
}

void UBA_LCD_screen_display_init_exit(UBA_LCD_screen *screen) {

}

void UBA_LCD_init_page_channel(UBA_LCD_STATIC_PAGE *channel)
{
	channel->frame.effect = UBA_GFX_EFFECT_SOLID;
	channel->frame.color_fill = UBA_GFX_COLOR_WHITE;

	channel->ch_name.effect = UBA_GFX_EFFECT_SOLID;
	channel->ch_name.text_size = LINE_CHANEL_NAME_FONT_SIZE;
	channel->ch_name.text_color_bg = UBA_GFX_COLOR_WHITE;
	channel->ch_name.text_color_text = UBA_GFX_COLOR_BLACK;

	channel->status.effect = UBA_GFX_EFFECT_BLINK_FAST;

	channel->volt.effect = UBA_GFX_EFFECT_SOLID;
	channel->volt.text_size = LINE_CHANEL_NAME_FONT_SIZE;
	channel->volt.text_color_text = UBA_GFX_COLOR_BLACK;
	channel->volt.text_color_bg = UBA_GFX_COLOR_WHITE;

	channel->current.effect = UBA_GFX_EFFECT_SOLID;
	channel->current.text_size = LINE_C_FONT_SIZE;
	channel->current.text_color_text = UBA_GFX_COLOR_BLACK;
	channel->current.text_color_bg = UBA_GFX_COLOR_WHITE;

	channel->capacity.effect = UBA_GFX_EFFECT_SOLID;
	channel->capacity.text_size = LINE_CAP_FONT_SIZE;
	channel->capacity.text_color_text = UBA_GFX_COLOR_BLACK;
	channel->capacity.text_color_bg = UBA_GFX_COLOR_WHITE;

	channel->temp.effect = UBA_GFX_EFFECT_SOLID;
	channel->temp.text_size = LINE_TEMP_FONT_SIZE;
	channel->temp.text_color_text = UBA_GFX_COLOR_BLACK;
	channel->temp.text_color_bg = UBA_GFX_COLOR_WHITE;

	channel->bnt_select.effect = UBA_GFX_EFFECT_SELECTED;
	channel->bnt_select.button_size = 2;
	channel->bnt_select.button_color_bg = UBA_GFX_COLOR_WHITE;
	channel->bnt_select.button_color_text = UBA_GFX_COLOR_BLACK;
	sprintf(channel->bnt_select.button_text, "%s", "SELECT");

	channel->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
	channel->EWI_msg.text_size = LINE_EWI_FONT_SIZE;
}

void UBA_LCD_screen_display_channel_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_STATIC_PAGE *channel = &LCD_handler->pages.channel;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;

	UBA_LCD_screen_update_state(screen);
	
	screen->pages.channel.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.channel.frame.pos.x = position[screen->ch_control-1].start_x;
	screen->pages.channel.frame.pos.y = position[screen->ch_control-1].start_y;
	screen->pages.channel.frame.effect = channel->frame.effect;
	screen->pages.channel.frame.elemnt.frame.width = position[screen->ch_control-1].width;
	screen->pages.channel.frame.elemnt.frame.heigth = position[screen->ch_control-1].height;
	screen->pages.channel.frame.elemnt.frame.color_fill = channel->frame.color_fill;
	screen->pages.channel.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.channel.channel.ch_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.ch_name.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.channel.channel.ch_name.pos.y = LINE(LINE_CHANEL_NAME);
	screen->pages.channel.channel.ch_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.ch_name.elemnt.text.size = LINE_CHANEL_NAME_FONT_SIZE;
	screen->pages.channel.channel.ch_name.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.channel.ch_name.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.channel.channel.status.id = UBA_GFX_ELEMNET_STATUS;
	screen->pages.channel.channel.status.pos.x = position[screen->ch_control-1].start_x + (((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) * 13) / 16); /*center 7/8*/
	screen->pages.channel.channel.status.pos.y = LINE(1);
	screen->pages.channel.channel.status.effect = UBA_GFX_EFFECT_BLINK_FAST;
	screen->pages.channel.channel.status.elemnt.status.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.channel.volt.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.volt.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.channel.channel.volt.pos.y = LINE(LINE_V);
	screen->pages.channel.channel.volt.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.volt.elemnt.text.size = LINE_V_FONT_SIZE;
	screen->pages.channel.channel.volt.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.volt.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.channel.current.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.current.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.channel.channel.current.pos.y = LINE(LINE_C);
	screen->pages.channel.channel.current.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.current.elemnt.text.size = LINE_C_FONT_SIZE;
	screen->pages.channel.channel.current.elemnt.text.size = LCD_DATA_FONT_SIZE;
	screen->pages.channel.channel.current.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.current.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.channel.capacity.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.capacity.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.channel.channel.capacity.pos.y = LINE(LINE_CAP);
	screen->pages.channel.channel.capacity.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.capacity.elemnt.text.size = LINE_CAP_FONT_SIZE;
	screen->pages.channel.channel.capacity.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.capacity.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.channel.temp.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.temp.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.channel.channel.temp.pos.y = LINE(LINE_TEMP);
	screen->pages.channel.channel.temp.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.temp.elemnt.text.size = LINE_TEMP_FONT_SIZE;
	screen->pages.channel.channel.temp.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.temp.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.bnt_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.channel.bnt_select.pos.x = position[screen->ch_control-1].start_x + position[screen->ch_control-1].width - 40;
	screen->pages.channel.bnt_select.pos.y = LINE(22);
	screen->pages.channel.bnt_select.effect = channel->bnt_select.effect;
	screen->pages.channel.bnt_select.elemnt.button.size = channel->bnt_select.button_size;
	screen->pages.channel.bnt_select.elemnt.button.color_bg = channel->bnt_select.button_color_bg;
	screen->pages.channel.bnt_select.elemnt.button.color_text = channel->bnt_select.button_color_text;
	sprintf(screen->pages.channel.bnt_select.elemnt.button.text, channel->bnt_select.button_text);

	screen->pages.channel.EWI_msg.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.EWI_msg.pos.x = position[screen->ch_control-1].start_x + ((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.channel.EWI_msg.pos.y = LINE(LINE_EWI);
	screen->pages.channel.EWI_msg.effect = channel->EWI_msg.effect;
	screen->pages.channel.EWI_msg.elemnt.text.size = channel->EWI_msg.text_size;

	// update shadow
	screen->pages.channel.channel.shadow.volt_vlaue = -1;
	screen->pages.channel.channel.shadow.current_vlaue = -1;
	screen->pages.channel.channel.shadow.capacity_vlaue = -1;
	screen->pages.channel.channel.shadow.temp_value = -1;
 
	screen->bpt->shadow.test_name[0] = 'x';
	screen->bpt->shadow.current_state = UBA_BPT_STATE_INVALID;
	screen->bpt->shadow.error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	screen->bpt->shadow.bnt_select.text[0] = '\0';
	screen->bpt->shadow.bnt_select.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_select.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_select.effect = UBA_GFX_EFFECT_MAX;
	screen->bpt->shadow.bnt_select.text[0] = '\0';
	screen->bpt->shadow.bnt_select.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_select.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_select.effect = UBA_GFX_EFFECT_MAX;

	UBA_LCD_screen_draw_channel(screen, UBA_LCD_REFRESH_TYPE_ALL);

	screen->start_tick = HAL_GetTick();
}
void UBA_LCD_screen_display_channel(UBA_LCD_screen *screen) {
	uint32_t refreshTime = ((screen->pages.channel.channel.status.effect == UBA_GFX_EFFECT_BLINK_FAST) ?
							UBA_LCD_FAST_REFRESH_TIME : UBA_LCD_SLOW_REFRESH_TIME);

	if (UBA_button_is_pending(screen->main_buttons.bnt_select_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
	}
	if ((HAL_GetTick() - screen->start_tick) >= refreshTime) {
		screen->start_tick = HAL_GetTick();
		UBA_LCD_screen_draw_channel(screen,
				UBA_LCD_REFRESH_TYPE_DATA | UBA_LCD_REFRESH_TYPE_STATUS | UBA_LCD_REFRESH_TYPE_UI | UBA_LCD_REFRESH_TYPE_EWI);
	}
}
void UBA_LCD_screen_display_channel_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_display_bpt_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;

	UBA_LCD_screen_update_state(screen);
	screen->pages.screen_bpt.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.screen_bpt.frame.pos.x = position[screen->ch_control-1].start_x;
	screen->pages.screen_bpt.frame.pos.y = position[screen->ch_control-1].start_y;
	screen->pages.screen_bpt.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.frame.elemnt.frame.width = position[screen->ch_control-1].width;
	screen->pages.screen_bpt.frame.elemnt.frame.heigth = position[screen->ch_control-1].height;
	screen->pages.screen_bpt.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.test_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.test_name.pos.x = position[screen->ch_control-1].start_x + ((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.test_name.pos.y = LINE(LINE_TEST_NAME);
	screen->pages.screen_bpt.test_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.test_name.elemnt.text.text [0] = '\0';
	screen->pages.screen_bpt.test_name.elemnt.text.size = LINE_TEST_NAME_FONT_SIZE;
	screen->pages.screen_bpt.test_name.elemnt.text.color_bg = WHITE;
	screen->pages.screen_bpt.test_name.elemnt.text.color_text = BLACK;

	screen->pages.screen_bpt.time.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.time.pos.x = position[screen->ch_control-1].start_x + ((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.time.pos.y = LINE(LINE_TIME);
	screen->pages.screen_bpt.time.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.time.elemnt.text.size = LINE_TIME_FONT_SIZE;
	screen->pages.screen_bpt.time.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.time.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.test_step.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.test_step.pos.x = position[screen->ch_control-1].start_x + ((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.test_step.pos.y = LINE(LINE_STEP);
	screen->pages.screen_bpt.test_step.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.test_step.elemnt.text.size = LINE_STEP_FONT_SIZE;
	screen->pages.screen_bpt.test_step.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.test_step.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.channel.ch_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.ch_name.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.ch_name.pos.y = LINE(LINE_CHANEL_NAME);
	screen->pages.screen_bpt.channel.ch_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.size = LINE_CHANEL_NAME_FONT_SIZE;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.channel.status.id = UBA_GFX_ELEMNET_STATUS;
	screen->pages.screen_bpt.channel.status.pos.x = position[screen->ch_control-1].start_x + (((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) * 13) / 16); /*center 7/8*/
	screen->pages.screen_bpt.channel.status.pos.y = LINE(1);
	screen->pages.screen_bpt.channel.status.effect = UBA_GFX_EFFECT_BLINK_FAST;
	screen->pages.screen_bpt.channel.status.elemnt.status.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.channel.volt.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.volt.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.volt.pos.y = LINE(LINE_V);
	screen->pages.screen_bpt.channel.volt.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.volt.elemnt.text.size = LINE_V_FONT_SIZE;
	screen->pages.screen_bpt.channel.volt.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.volt.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.channel.current.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.current.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.current.pos.y = LINE(LINE_C);
	screen->pages.screen_bpt.channel.current.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.current.elemnt.text.size = LINE_C_FONT_SIZE;
	screen->pages.screen_bpt.channel.current.elemnt.text.size = LCD_DATA_FONT_SIZE;
	screen->pages.screen_bpt.channel.current.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.current.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.channel.capacity.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.capacity.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.capacity.pos.y = LINE(LINE_CAP);
	screen->pages.screen_bpt.channel.capacity.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.size = LINE_CAP_FONT_SIZE;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.channel.temp.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.temp.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.temp.pos.y = LINE(LINE_TEMP);
	screen->pages.screen_bpt.channel.temp.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.temp.elemnt.text.size = LINE_TEMP_FONT_SIZE;
	screen->pages.screen_bpt.channel.temp.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.temp.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.bnt_back_stop.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.screen_bpt.bnt_back_stop.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING + 30;
	screen->pages.screen_bpt.bnt_back_stop.pos.y = LINE(22);
	screen->pages.screen_bpt.bnt_back_stop.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.bnt_back_stop.elemnt.button.size = 2;

	screen->pages.screen_bpt.bnt_pause_start.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.screen_bpt.bnt_pause_start.pos.x = position[screen->ch_control-1].start_x + position[screen->ch_control-1].width - 40;
	screen->pages.screen_bpt.bnt_pause_start.pos.y = LINE(22);
	screen->pages.screen_bpt.bnt_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.screen_bpt.bnt_pause_start.elemnt.button.size = 2;

	screen->pages.screen_bpt.EWI_msg.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.EWI_msg.pos.x = position[screen->ch_control-1].start_x + ((position[screen->ch_control-1].width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.EWI_msg.pos.y = LINE(LINE_EWI);
	screen->pages.screen_bpt.EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.EWI_msg.elemnt.text.size = LINE_EWI_FONT_SIZE;

	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_ALL);
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);

	// update shadow
	screen->pages.screen_bpt.channel.shadow.volt_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.current_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.capacity_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.temp_value = -1;

	screen->bpt->shadow.test_name[0] = 'x';
	screen->bpt->shadow.current_state = UBA_BPT_STATE_INVALID;
	screen->bpt->shadow.error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	screen->bpt->shadow.bnt_back_stop.text[0] = '\0';
	screen->bpt->shadow.bnt_back_stop.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_back_stop.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_back_stop.effect = UBA_GFX_EFFECT_MAX;
	screen->bpt->shadow.bnt_back_stop.text[0] = '\0';
	screen->bpt->shadow.bnt_back_stop.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_back_stop.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.bnt_back_stop.effect = UBA_GFX_EFFECT_MAX;
}

void UBA_LCD_screen_display_bpt(UBA_LCD_screen *screen) {
	uint32_t refreshTime = ((screen->pages.screen_bpt.channel.status.effect == UBA_GFX_EFFECT_BLINK_FAST) ?
	UBA_LCD_FAST_REFRESH_TIME : UBA_LCD_SLOW_REFRESH_TIME);

	if ((UBA_button_is_pending(screen->main_buttons.bnt_up_p) || UBA_button_is_pending(screen->main_buttons.bnt_down_p))
			|| (UBA_button_is_pending(screen->secondery_buttons.bnt_up_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_down_p))) {
		UBA_LCD_screen_bnt_press_up_or_down(screen, &screen->pages.screen_bpt);
		UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
		UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	}
	if (UBA_button_is_pending(screen->main_buttons.bnt_select_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		if (UBA_LCD_screen_bnt_press_select(screen)) {
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
		} else {
			UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
			UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
			UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
		}
	}
	if ((HAL_GetTick() - screen->start_tick) >= refreshTime) {
		screen->start_tick = HAL_GetTick();
		UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_DATA | UBA_LCD_REFRESH_TYPE_STATUS | UBA_LCD_REFRESH_TYPE_UI | UBA_LCD_REFRESH_TYPE_EWI);
	}
}

void UBA_LCD_screen_display_bpt_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_display_test_select_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	int i;

	UBA_LCD_screen_update_state(screen);
	screen->pages.test_list.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_list.frame.pos.x = position[screen->ch_control-1].start_x;
	screen->pages.test_list.frame.pos.y = position[screen->ch_control-1].start_y;
	screen->pages.test_list.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_list.frame.elemnt.frame.width = position[screen->ch_control-1].width;
	screen->pages.test_list.frame.elemnt.frame.heigth = position[screen->ch_control-1].height;
	screen->pages.test_list.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_list.title.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.test_list.title.pos.x = position[screen->ch_control-1].width / 2 + position[screen->ch_control-1].start_x;
	screen->pages.test_list.title.pos.y = LINE(1);
	screen->pages.test_list.title.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_list.title.elemnt.text.size = 2;
	screen->pages.test_list.title.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.title.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_list.title.elemnt.text.text, "Test Select");

	screen->pages.test_list.bnt_cancel.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_list.bnt_cancel.pos.x = position[screen->ch_control-1].start_x + position[screen->ch_control-1].width - 40;
	screen->pages.test_list.bnt_cancel.pos.y = LINE(22);
	screen->pages.test_list.bnt_cancel.effect = UBA_GFX_EFFECT_VISIBLE;

	screen->pages.test_list.bnt_cancel.elemnt.button.size = 2;
	screen->pages.test_list.bnt_cancel.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.bnt_cancel.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_list.bnt_cancel.elemnt.button.text, "CANCEL");

	for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
		screen->pages.test_list.test_name_list[i].id = UBA_GFX_ELEMNET_TEXT;
		screen->pages.test_list.test_name_list[i].pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
		screen->pages.test_list.test_name_list[i].pos.y = LINE((4 + (i * 2)));
		screen->pages.test_list.test_name_list[i].effect = UBA_GFX_EFFECT_SOLID;
		screen->pages.test_list.test_name_list[i].elemnt.text.size = 2;
		screen->pages.test_list.test_name_list[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_list.test_name_list[i].elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
		sprintf(screen->pages.test_list.test_name_list[i].elemnt.text.text, "%c-%.*s",
				TR_file.list[i].mode == UBA_PROTO_BPT_MODE_SINGLE_CHANNEL ? UBA_LCD_SCREEN_SINGEL_CH_CHAR : UBA_LCD_SCREEN_DUAL_CH_CHAR,
				UBA_LCD_screen_line_max_str_length(screen, screen->pages.test_list.test_name_list[i].elemnt.text.size) - 2, TR_file.list[i].name);
	}

	UBA_LCD_screen_display_test_select_refresh(&screen->pages.test_list, UBA_LCD_REFRESH_TYPE_ALL);
}

void UBA_LCD_screen_display_test_select(UBA_LCD_screen *screen) {
	int i, j;
	bool need_to_refresh = false;
	uint8_t list_select_index;

	if (UBA_button_is_pending(screen->main_buttons.bnt_up_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_up_p)) {
		UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
		screen->pages.test_list.select_index = screen->pages.test_list.select_index - 1;
				//screen->pages.test_list.select_index == 0 ? 0 : screen->pages.test_list.select_index - 1;
		screen->pages.test_list.list_select_index = screen->pages.test_list.list_select_index - 1;
				//screen->pages.test_list.list_select_index == 0 ? 0 : screen->pages.test_list.list_select_index - 1;
		
		list_select_index = screen->pages.test_list.list_select_index;
		if (screen->pages.test_list.list_select_index < 0)
		{
			screen->pages.test_list.list_select_index = UBA_TR_LIST_SIZE;
			screen->pages.test_list.select_index = UBA_LCD_MAX_DISPLAY_TEST_SELECT-1;
		}
		else
		if (screen->pages.test_list.list_select_index >= UBA_TR_LIST_SIZE)
		{
			screen->pages.test_list.list_select_index = 0;
			screen->pages.test_list.select_index = 0;
		}
		need_to_refresh = true;
	} else if (UBA_button_is_pending(screen->main_buttons.bnt_down_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_down_p)) {
		UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
		screen->pages.test_list.select_index =
				(screen->pages.test_list.select_index < UBA_LCD_MAX_DISPLAY_TEST_SELECT - 1) ?
						screen->pages.test_list.select_index + 1 :
						UBA_LCD_MAX_DISPLAY_TEST_SELECT - 1;
		screen->pages.test_list.list_select_index++;

		list_select_index = screen->pages.test_list.list_select_index;
		if (screen->pages.test_list.list_select_index > UBA_TR_LIST_SIZE)
		{
			screen->pages.test_list.list_select_index = 0;
			screen->pages.test_list.select_index = 0;
		}
		need_to_refresh = true;
	} else if (UBA_button_is_pending(screen->main_buttons.bnt_select_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
		if (screen->pages.test_list.list_select_index < UBA_TR_LIST_SIZE) {
			screen->tr = &TR_file.list[screen->pages.test_list.list_select_index];
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO;
		} else {
			screen->tr = NULL;
			if (UBA_BPT_isUnpacked(screen->bpt)) {
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
			} else {
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_CHANNEL;
			}
		}
		if (screen->pages.test_list.select_index < 0) {
			screen->pages.test_list.select_index = 0;
		}
	}
	
	list_select_index = screen->pages.test_list.list_select_index;
	if (screen->pages.test_list.list_select_index >= UBA_TR_LIST_SIZE) {
		screen->pages.test_list.list_select_index = UBA_TR_LIST_SIZE;
		screen->pages.test_list.bnt_cancel.effect = UBA_GFX_EFFECT_SELECTED;
		screen->pages.test_list.select_index = UBA_LCD_MAX_DISPLAY_TEST_SELECT;
	} else {
		screen->pages.test_list.bnt_cancel.effect = UBA_GFX_EFFECT_VISIBLE;
	}
	if ((screen->pages.test_list.select_index >= UBA_LCD_MAX_DISPLAY_TEST_SELECT - 1)
			|| ((screen->pages.test_list.select_index == 0) && (screen->pages.test_list.list_select_index > 0))) {
		for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
			if ((screen->pages.test_list.select_index == 0) && (screen->pages.test_list.list_select_index > 0)) {
				j = ((screen->pages.test_list.list_select_index - 1) + i) % UBA_TR_LIST_SIZE;
			} else {
				j = ((screen->pages.test_list.list_select_index - (UBA_LCD_MAX_DISPLAY_TEST_SELECT - 1)) + i) % UBA_TR_LIST_SIZE;
			}
			sprintf(screen->pages.test_list.test_name_list[i].elemnt.text.text, "%c-%.*s",
					TR_file.list[j].mode == UBA_PROTO_BPT_MODE_SINGLE_CHANNEL ? UBA_LCD_SCREEN_SINGEL_CH_CHAR : UBA_LCD_SCREEN_DUAL_CH_CHAR,
					UBA_LCD_screen_line_max_str_length(screen, screen->pages.test_list.test_name_list[i].elemnt.text.size) - 2, TR_file.list[j].name);
		}
	}

	if (need_to_refresh) {
		UBA_LCD_screen_display_test_select_refresh(&screen->pages.test_list, UBA_LCD_REFRESH_TYPE_UI);
	}

	if (list_select_index >= UBA_TR_LIST_SIZE)
	{
		//screen->pages.test_list.select_index = -1;
		//screen->pages.test_list.list_select_index = -1;
	}
}

void UBA_LCD_screen_display_test_select_exit(UBA_LCD_screen *screen) {
	screen->tr_list_select_index = screen->pages.test_list.list_select_index;
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_dispaly_test_info_refresh(UBA_LCD_page_test_info *test_info, UBA_LCD_REFRESH_TYPE rt) {
	int i;
	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&test_info->frame);
	}
	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		UBA_GFX_draw_text_center(&test_info->title);
		for (i = 0; i < 6; i++) {
			UBA_GFX_draw_text(&test_info->test_info[i]);
		}
	}
	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		UBA_GFX_draw_button(&test_info->bnt_back);
		UBA_GFX_draw_button(&test_info->bnt_select);
	}
}

void UBA_LCD_screen_display_test_info_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	int i = 0;

	UBA_LCD_screen_update_state(screen);
	screen->pages.test_info.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_info.frame.pos.x = position[screen->ch_control-1].start_x;
	screen->pages.test_info.frame.pos.y = position[screen->ch_control-1].start_y;
	screen->pages.test_info.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.frame.elemnt.frame.width = position[screen->ch_control-1].width;
	screen->pages.test_info.frame.elemnt.frame.heigth = position[screen->ch_control-1].height;
	screen->pages.test_info.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.title.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.test_info.title.pos.x = position[screen->ch_control-1].width / 2 + position[screen->ch_control-1].start_x;
	screen->pages.test_info.title.pos.y = LINE(1);
	screen->pages.test_info.title.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.title.elemnt.text.size = 2;
	screen->pages.test_info.title.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.title.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	if ((screen->tr) != NULL) {
		sprintf(screen->pages.test_info.title.elemnt.text.text, " %.*s",
				UBA_LCD_screen_line_max_str_length(screen, screen->pages.test_info.title.elemnt.text.size), (screen->tr)->name);
	}
	for (i = 0; i < 6; i++) {
		screen->pages.test_info.test_info[i].id = UBA_GFX_ELEMNET_TEXT;
		screen->pages.test_info.test_info[i].pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING;
		screen->pages.test_info.test_info[i].pos.y = LINE((5 + (i * 2)));
		screen->pages.test_info.test_info[i].effect = UBA_GFX_EFFECT_SOLID;
		screen->pages.test_info.test_info[i].elemnt.text.size = 2;
		screen->pages.test_info.test_info[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
		sprintf(screen->pages.test_info.test_info[i].elemnt.text.text, "info[%d]", i);
	}
	i = 0;
	sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "1.Type:");
	switch (screen->tr->battery.type) {
		case UBA_BATTERY_TYPE_PRIMERY:
			sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "Primary");
			break;
		case UBA_BATTERY_TYPE_SECONDERY:
			sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "Secondary");
			break;
		default:
			sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "Unknown");
			break;
	}
	sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "2.S/N:");
	sprintf(screen->pages.test_info.test_info[i].elemnt.text.text, " %.*s",
			UBA_LCD_screen_line_max_str_length(screen, screen->pages.test_info.test_info[i].elemnt.text.size), (screen->tr)->battery.serial_number);
	i++;
	sprintf(screen->pages.test_info.test_info[i++].elemnt.text.text, "3.Cell Num:");
	sprintf(screen->pages.test_info.test_info[i].elemnt.text.text, " %04u", (screen->tr)->battery.number_of_cells);

	screen->pages.test_info.bnt_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.bnt_back.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING + 30;
	screen->pages.test_info.bnt_back.pos.y = LINE(22);
	screen->pages.test_info.bnt_back.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.bnt_back.elemnt.button.size = 2;
	screen->pages.test_info.bnt_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.bnt_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.bnt_back.elemnt.button.text, "BACK");

	screen->pages.test_info.bnt_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.bnt_select.pos.x = position[screen->ch_control-1].start_x + position[screen->ch_control-1].width - 40;
	screen->pages.test_info.bnt_select.pos.y = LINE(22);
	screen->pages.test_info.bnt_select.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.test_info.bnt_select.elemnt.button.size = 2;
	screen->pages.test_info.bnt_select.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.bnt_select.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.bnt_select.elemnt.button.text, "SELECT");

	UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_ALL);
}

void UBA_LCD_screen_display_test_info(UBA_LCD_screen *screen) {
	if ((UBA_button_is_pending(screen->main_buttons.bnt_up_p) || UBA_button_is_pending(screen->main_buttons.bnt_down_p))
			|| (UBA_button_is_pending(screen->secondery_buttons.bnt_up_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_down_p))) {
		if (screen->pages.test_info.bnt_back.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->pages.test_info.bnt_back.effect = UBA_GFX_EFFECT_VISIBLE;
			screen->pages.test_info.bnt_select.effect = UBA_GFX_EFFECT_SELECTED;
		} else if (screen->pages.test_info.bnt_select.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->pages.test_info.bnt_back.effect = UBA_GFX_EFFECT_SELECTED;
			screen->pages.test_info.bnt_select.effect = UBA_GFX_EFFECT_VISIBLE;
		}
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
		UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
		UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	}
	if (UBA_button_is_pending(screen->main_buttons.bnt_select_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		if (screen->pages.test_info.bnt_back.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
		} else if (screen->pages.test_info.bnt_select.effect == UBA_GFX_EFFECT_SELECTED) {
			if (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
				if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], &(UBA_6_device_g.BPT_AB)) != 0) {
					UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
				} 
			} else {
				if (screen->ch_control == UBA_CHANNLE_ID_AB) {
					if (UBA_button_is_pending(screen->main_buttons.bnt_select_p)) {
						if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], &(UBA_6_device_g.BPT_A)) != 0) {
							UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
						}
					} else if (UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
						if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], &(UBA_6_device_g.BPT_B)) != 0) {
							UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
						}
					}
				} else if (screen->ch_control == UBA_CHANNLE_ID_A) {
					if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], &(UBA_6_device_g.BPT_A)) != 0) {
						UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
					}

				} else if (screen->ch_control == UBA_CHANNLE_ID_B) {
					if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], &(UBA_6_device_g.BPT_B)) != 0) {
						UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
					}
				}
			}
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_SETTING; //Moshe UBA_LCD_SCREEN_DISPLAY_BPT;
		}
		UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
}

void UBA_LCD_screen_display_test_info_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_display_setting_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;

	UBA_LCD_screen_update_state(screen);
	screen->pages.test_info.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_info.frame.pos.x = position[screen->ch_control-1].start_x;
	screen->pages.test_info.frame.pos.y = position[screen->ch_control-1].start_y;
	screen->pages.test_info.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.frame.elemnt.frame.width = position[screen->ch_control-1].width;
	screen->pages.test_info.frame.elemnt.frame.heigth = position[screen->ch_control-1].height;
	screen->pages.test_info.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.bnt_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.bnt_back.pos.x = position[screen->ch_control-1].start_x + BORDER_PADDING + 30;
	screen->pages.test_info.bnt_back.pos.y = LINE(22);
	screen->pages.test_info.bnt_back.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.test_info.bnt_back.elemnt.button.size = 2;
	screen->pages.test_info.bnt_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.bnt_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.bnt_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.bnt_select.pos.x = position[screen->ch_control-1].start_x + position[screen->ch_control-1].width - 40;
	screen->pages.test_info.bnt_select.pos.y = LINE(22);
	screen->pages.test_info.bnt_select.effect = UBA_GFX_EFFECT_INVISIBLE;//UBA_GFX_EFFECT_SOLID
	screen->pages.test_info.bnt_select.elemnt.button.size = 2;
	screen->pages.test_info.bnt_select.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.bnt_select.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.bnt_back.elemnt.button.text, "BACK");

	UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_FRAME | UBA_LCD_REFRESH_TYPE_UI);
}

void UBA_LCD_screen_display_setting(UBA_LCD_screen *screen) {
	if ((UBA_button_is_pending(screen->main_buttons.bnt_up_p) || UBA_button_is_pending(screen->main_buttons.bnt_down_p))
			|| (UBA_button_is_pending(screen->secondery_buttons.bnt_up_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_down_p))) {
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
	if (UBA_button_is_pending(screen->main_buttons.bnt_select_p) || UBA_button_is_pending(screen->secondery_buttons.bnt_select_p)) {
		if (screen->pages.test_info.bnt_back.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO;
		} 
		UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
		UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
}

void UBA_LCD_screen_display_setting_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_display_off_enter(UBA_LCD_screen *screen) {
	UBA_LCD_screen_update_state(screen);
}

void UBA_LCD_screen_display_off(UBA_LCD_screen *screen) {
}

void UBA_LCD_screen_display_off_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_panding(screen->main_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->main_buttons.bnt_select_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_up_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_down_p);
	UBA_button_clear_panding(screen->secondery_buttons.bnt_select_p);
}

void UBA_LCD_screen_run(UBA_LCD_screen *screen) {
	if (screen->state.next == UBA_LCD_SCREEN_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[screen->state.current].run) {
			rule_g[screen->state.current].run(screen); // run the main function of the state
		}
	} else {
		if (screen->state.current < UBA_LCD_SCREEN_STATE_MAX) {
			if (rule_g[screen->state.current].exit) {
				rule_g[screen->state.current].exit(screen); // run the status exit function
			}
		}
		if (rule_g[screen->state.next].enter) {
			rule_g[screen->state.next].enter(screen); // run the next state enter function
		}
	}
}

