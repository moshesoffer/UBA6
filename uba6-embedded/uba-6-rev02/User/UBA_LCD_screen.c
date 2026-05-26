/*
 * UBA_LCD_screen.c
 *
 *  Created on: Sep 22, 2024
 *      Author: ORA
 */
#undef UART_LOG_DISABLE

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
extern void UBA_6_init(UBA_6 *uba);

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
#define LINE_TEST_NAME 				(LINE_CHANEL_NAME+LINE_CHANEL_NAME_FONT_SIZE)
#define LINE_TEST_NAME_FONT_SIZE 	(2)
#define LINE_TIME 					(LINE_TEST_NAME+LINE_TEST_NAME_FONT_SIZE+1)
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
#define LINE_EWI_FONT_SIZE			(1)
#define LINE_STEP 					(LINE_EWI+LINE_EWI_FONT_SIZE)
#define LINE_STEP_FONT_SIZE 		(1)

#define LINE(x) ((LINE_H*x) + LINE0_Y)
#define UBA_LCD_MAX_DISPLAY_TEST_SELECT (7)
#define UBA_EWI_MAX_LINE_CHAR_SIZE (12)

#define UBA_LCD_SCREEN_SINGEL_CH_CHAR (179)
#define UBA_LCD_SCREEN_DUAL_CH_CHAR (186)

#define BTN_LOWER_YPOS   (LINE(22) + 8)
#define BTN_UPPER_YPOS   (LINE(21) - 4)

#if (UBA_LOG_LEVEL_SCREEN <= UART_LOG_LEVEL_INFO)
#define UART_LOG_SCREEN_INFO(...) UART_LOG_INFO(UBA_COMP,##__VA_ARGS__)
#else
#define UART_LOG_SCREEN_INFO(...)
#endif

#if UBA_LOG_LEVEL_SCREEN <= UART_LOG_LEVEL_DEBUG
#define UART_LOG_SCREEN_DEBUG(...)  UART_LOG_DEBUG(UBA_COMP ,##__VA_ARGS__)
#else
#define UART_LOG_SCREEN_DEBUG(...)
#endif

bool LCD_screen_force_draw = false;

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

void UBA_LCD_screen_display_test_step_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_step(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_test_step_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_setting_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_setting(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_setting_exit(UBA_LCD_screen *screen);

void UBA_LCD_screen_display_exe_cmd_enter(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_exe_cmd(UBA_LCD_screen *screen);
void UBA_LCD_screen_display_exe_cmd_exit(UBA_LCD_screen *screen);

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
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_TEST_STEP,     UBA_LCD_screen_display_test_step_enter,		UBA_LCD_screen_display_test_step,	UBA_LCD_screen_display_test_step_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_SETTING,		UBA_LCD_screen_display_setting_enter,		UBA_LCD_screen_display_setting,		UBA_LCD_screen_display_setting_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_EXE_CMD,       UBA_LCD_screen_display_exe_cmd_enter,		UBA_LCD_screen_display_exe_cmd,		UBA_LCD_screen_display_exe_cmd_exit),
		UBABPTSMA(UBA_LCD_SCREEN_DISPLAY_OFF,			UBA_LCD_screen_display_off_enter,			UBA_LCD_screen_display_off,			UBA_LCD_screen_display_off_exit),
};
static const char *name_g[UBA_LCD_SCREEN_STATE_MAX] ={
		"INIT",		
		"CHANNEL",	
		"BPT",		
		"TEST_SELECT",
		"TEST_INFO",	
		"TEST_STEP",   
		"SETTING",	
		"EXE_CMD",     
		"OFF"	
};
// @formatter:on
//=================================================private functions========================================================//

void UBA_LCD_screen_update_state(UBA_LCD_screen *screen) {
	
	if ((screen->state.current < UBA_LCD_SCREEN_STATE_MAX) && (screen->state.current < UBA_LCD_SCREEN_STATE_MAX)) {
		UART_LOG(UBA_COMP, "(screen)update state %s ---> %s", name_g[screen->state.current], name_g[screen->state.next]);
	} else {
		UART_LOG(UBA_COMP, "(screen)update state %u ---> %u", screen->state.current, screen->state.next);
	}
	screen->state.pre = screen->state.current;
	screen->state.current = screen->state.next;
	screen->state.next = UBA_LCD_SCREEN_STATE_INVALID;
}

void UBA_LCD_screen_btn_press_up_or_down(UBA_LCD_screen *screen, UBA_LCD_page_BPT *lcd_ch) {

	if (lcd_ch->btn_pause_start.effect == UBA_GFX_EFFECT_SELECTED) {
		lcd_ch->btn_pause_start.effect = UBA_GFX_EFFECT_VISIBLE;
		//lcd_ch->btn_next.effect = UBA_GFX_EFFECT_VISIBLE; 
		lcd_ch->btn_back_stop.effect = UBA_GFX_EFFECT_SELECTED;

	} else if (lcd_ch->btn_back_stop.effect == UBA_GFX_EFFECT_SELECTED) {
		lcd_ch->btn_back_stop.effect = UBA_GFX_EFFECT_VISIBLE;

		if (screen->pages.screen_bpt.btn_next.effect == UBA_GFX_EFFECT_VISIBLE) {
			lcd_ch->btn_pause_start.effect = UBA_GFX_EFFECT_VISIBLE;
			lcd_ch->btn_next.effect = UBA_GFX_EFFECT_SELECTED;
		} else {
			lcd_ch->btn_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
			lcd_ch->btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;
		}

	} else if (lcd_ch->btn_next.effect == UBA_GFX_EFFECT_SELECTED) {
		lcd_ch->btn_next.effect = UBA_GFX_EFFECT_VISIBLE;
		lcd_ch->btn_back_stop.effect = UBA_GFX_EFFECT_VISIBLE;
		lcd_ch->btn_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
	}
	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
}

/*
 * return the number of button in Panding mode
 * */
int UBA_LCD_screen_isPanding(UBA_LCD_screen *screen) {
	int ret = 0;
	if (UBA_button_is_pending(screen->main_buttons.btn_up_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->main_buttons.btn_down_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->main_buttons.btn_select_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.btn_up_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.btn_down_p)) {
		ret++;
	}
	if (UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		ret++;
	}
	return ret;
}

bool UBA_LCD_is_mate_channel_AB(UBA_LCD_screen *screen) {
	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, do NOT draw 
	//if (screen->ch_control != UBA_PROTO_CHANNEL_ID_AB) {
		if (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) {
			//check channel B
			if (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) {
				return true;
			}

		} else if (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
			//check channel B
			if (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) {
				return true;
			}
		}
	//}
	return false;
}

//TODO: use in Util
uint32_t TimeToSeconds(RTC_TimeTypeDef *time) {
	return time->Hours * 3600 + time->Minutes * 60 + time->Seconds;
}

void UBA_LCD_screen_getRunTime(UBA_BPT *bpt, RTC_TimeTypeDef *time) {
	UBA_LCD_screen *screen = bpt->ch->current_screen;
	UBA_LCD_page_BPT *lcd_bpt = &(screen->pages.screen_bpt);
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;
	uint32_t time1_seconds;
	uint32_t time2_seconds;
	uint32_t diff_seconds;
	uint8_t hours, minutes, seconds;

	if (bpt->state.current == UBA_BPT_STATE_RUN_STEP) {
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
	
		diff_seconds += screen->bpt->start_date_time.add_pause_seconds;

		hours   =  diff_seconds / 3600;
		minutes = (diff_seconds % 3600) / 60;
		seconds =  diff_seconds % 60;

	} else if (bpt->state.current == UBA_BPT_STATE_PAUSE) {
		if (screen->bpt->start_date_time.update_pause_seconds == true) {
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
		
			screen->bpt->start_date_time.add_pause_seconds += diff_seconds;
			screen->bpt->start_date_time.update_pause_seconds = false;
		}

		hours   =  screen->bpt->start_date_time.add_pause_seconds / 3600;
		minutes = (screen->bpt->start_date_time.add_pause_seconds % 3600) / 60;
		seconds =  screen->bpt->start_date_time.add_pause_seconds % 60;

	} else {
		UART_LOG_SCREEN_INFO("====> ch %s, state %x, time=0", bpt->ch->name, bpt->state.current);
		hours   = 0;
		minutes = 0;
		seconds = 0;
	}
	UART_LOG_SCREEN_INFO(UBA_COMP, "====> (getRunTime) ch: %x, time: %d:%d:%d", bpt->ch->id, hours, minutes, seconds);
	time->Hours = hours; time->Minutes = minutes; time->Seconds = seconds;
}

void UBA_LCD_screen_load_channel(UBA_LCD_channel *lcd_ch, UBA_channel *ch) {
	float data_vaiue = 0;
	float temp_value;
	float step_value;
	char buffer[25] = { 0 };
	UBA_BPT *channel_test = ((UBA_LCD_screen *) ch->current_screen)->bpt;

	if (strcmp(lcd_ch->ch_name.elemnt.text.text, ch->name) != 0) {
		switch (ch->id) {
			case UBA_CHANNLE_ID_A:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH A");
				break;
			case UBA_CHANNLE_ID_B:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH B");
				break;
			case UBA_CHANNLE_ID_AB:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH AB");
				break;
			default:
				sprintf(lcd_ch->ch_name.elemnt.text.text, "CH N/A");
		}
		if (((UBA_LCD_screen *)ch->current_screen)->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
			sprintf(lcd_ch->ch_name.elemnt.text.text, "CH AB");
		}

		//update shadoe
		ch->shadow.ch_name_changed = true;
	}
	if (ch->state.current != ch->shadow.state) {
		switch (ch->state.current) {
			case UBA_CHANNEL_STATE_INIT:
				lcd_ch->status.effect = UBA_GFX_EFFECT_SOLID;
				lcd_ch->status.elemnt.status.color_fill = UBA_GFX_COLOR_INIT;
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

		//update shadow - will be set in draw function
		ch->shadow.state = ch->state.current;
	}

	lcd_ch->shadow.volt_vlaue_changed = false;
	if (ch->id == UBA_CHANNLE_ID_AB) {
		data_vaiue = UBA_channel_get_voltage(&UBA_CH_A) + UBA_channel_get_voltage(&UBA_CH_B);
	} else {
		data_vaiue = UBA_channel_get_voltage(ch);
	}
	if (data_vaiue != lcd_ch->shadow.volt_vlaue) {
		if (data_vaiue >= 999) {
			data_vaiue /= 1000.0f;
			//if (data_vaiue >= 999) {
			//	sprintf(buffer, "%3f %-3s", data_vaiue, "V");
			//} else {
				sprintf(buffer, "%3.2f %-3s", data_vaiue, "V");
			//}
		} else {
			sprintf(buffer, "%.2f %-3s", data_vaiue, "mV");
		}
		lcd_ch->volt.effect = UBA_GFX_EFFECT_SOLID;
		snprintf(lcd_ch->volt.elemnt.text.text, 12, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.volt_vlaue = data_vaiue;
		lcd_ch->shadow.volt_vlaue_changed = true;
	}

	lcd_ch->shadow.current_value_changed = false;
	if (ch->id == UBA_CHANNLE_ID_AB) {
		data_vaiue = UBA_channel_get_current(&UBA_CH_A) + UBA_channel_get_current(&UBA_CH_B);
	} else {
		data_vaiue = UBA_channel_get_current(ch);
	}
	if (data_vaiue != lcd_ch->shadow.current_vlaue) {
		//if (abs(data_vaiue) >= 2000.0f) {
			data_vaiue /= 1000.0f;
			sprintf(buffer, "%.5f %-3s", data_vaiue, "A");
		//	sprintf(buffer, "%.2f %-3s", data_vaiue, "A");
		//} else {
		//	sprintf(buffer, "%.2f %-3s", data_vaiue, "mA");
		//}
		lcd_ch->current.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_ch->current.elemnt.text.text, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.current_vlaue = data_vaiue;
		lcd_ch->shadow.current_value_changed = true;
	}

	lcd_ch->shadow.capacity_vlaue_changed = false;
	if (ch->id == UBA_CHANNLE_ID_AB) {
		data_vaiue = UBA_channel_get_capacity(&UBA_CH_A) + UBA_channel_get_capacity(&UBA_CH_B);
	} else {
		data_vaiue = UBA_channel_get_capacity(ch);
	}
	if (data_vaiue != lcd_ch->shadow.capacity_vlaue) {
		//if (abs(data_vaiue) >= 1000) {
			data_vaiue /= 1000.0f;
			sprintf(buffer, "%.5f %-3s", data_vaiue, "Ah");
		//} else {
		//	sprintf(buffer, "%.2f %-3s", data_vaiue, "mAh");
		//}
		lcd_ch->capacity.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_ch->capacity.elemnt.text.text, CHANNEL_DISPALY_DATA_PAD, strlen(buffer), buffer);

		//update shadow
		lcd_ch->shadow.capacity_vlaue = data_vaiue;
		lcd_ch->shadow.capacity_vlaue_changed = true;
	}

	lcd_ch->shadow.temp_value_changed = false;
	if (ch->id == UBA_CHANNLE_ID_AB) {
		temp_value = UBA_channel_get_temperature(&UBA_CH_A) + UBA_channel_get_temperature(&UBA_CH_B);
	} else {
		temp_value = UBA_channel_get_temperature(ch);
	}
	if (temp_value != lcd_ch->shadow.temp_value) {
		sprintf(buffer, "%+3.2f\xf8 %s", temp_value, "C");
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

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, do NOT draw
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {
	//	return;
	//}
	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
		if (screen->bpt->ch->id != UBA_PROTO_CHANNEL_ID_A) {
			//draw channel A only
			return;
		}
	}

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

	//update current screen
	ch->current_screen = screen;

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

	int i;
	UBA_GFX measure[4];
	for (i=0; i< 4; i++)
	{
		measure[i].id = UBA_GFX_ELEMNET_TEXT;
		measure[i].effect = UBA_GFX_EFFECT_SOLID;
		measure[i].elemnt.text.size = 1;
		measure[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		measure[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	}			

	if ((rt & UBA_LCD_REFRESH_TYPE_DATA) == UBA_LCD_REFRESH_TYPE_DATA) {
		if (lcd_channel_shadow->volt_vlaue_changed == true) {
			i = 0;
			measure[i].pos.x = screen->pages.channel.channel.volt.pos.x;
			measure[i].pos.y = screen->pages.channel.channel.volt.pos.y-2;
			//clean measure title
			sprintf(measure[i].elemnt.text.text, "                       ");
			UBA_GFX_draw_text(&measure[i]);
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  voltage:");
			UBA_GFX_draw_text(&measure[i]);

			screen->pages.channel.channel.volt.pos.y += 8;
			UBA_GFX_draw_text(&screen->pages.channel.channel.volt);
			screen->pages.channel.channel.volt.pos.y -= 8;
			//update shadow
			lcd_channel_shadow->volt_vlaue_changed = false;
		}
		if (lcd_channel_shadow->current_value_changed == true) {
			i = 1;
			measure[i].pos.x = screen->pages.channel.channel.current.pos.x;
			measure[i].pos.y = screen->pages.channel.channel.current.pos.y+6;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  current:");
			UBA_GFX_draw_text(&measure[i]);

			screen->pages.channel.channel.current.pos.y += 16;
			UBA_GFX_draw_text(&screen->pages.channel.channel.current);
			screen->pages.channel.channel.current.pos.y -= 16;
			//update shadow
			lcd_channel_shadow->current_value_changed = false;
		}
		if (lcd_channel_shadow->capacity_vlaue_changed == true) {
			i = 2;
			measure[i].pos.x = screen->pages.channel.channel.capacity.pos.x;
			measure[i].pos.y = screen->pages.channel.channel.capacity.pos.y+14;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  capacity:");
			UBA_GFX_draw_text(&measure[i]);

			screen->pages.channel.channel.capacity.pos.y += 24;
			UBA_GFX_draw_text(&screen->pages.channel.channel.capacity);
			screen->pages.channel.channel.capacity.pos.y -= 24;
			//update shadow
			lcd_channel_shadow->capacity_vlaue_changed = false;
		}
		if (lcd_channel_shadow->temp_value_changed == true) {
			i = 3;
			measure[i].pos.x = screen->pages.channel.channel.temp.pos.x;
			measure[i].pos.y = screen->pages.channel.channel.temp.pos.y+22;
			//clean measure title
			sprintf(measure[i].elemnt.text.text, "                       ");
			UBA_GFX_draw_text(&measure[i]);
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  temperature:");
			UBA_GFX_draw_text(&measure[i]);

			screen->pages.channel.channel.temp.pos.y += 32;
			UBA_GFX_draw_text(&screen->pages.channel.channel.temp);
			screen->pages.channel.channel.temp.pos.y -= 32;
			//update shadow
			lcd_channel_shadow->temp_value_changed = false;
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_EWI) == UBA_LCD_REFRESH_TYPE_EWI) {
		if (lcd_bpt_shadow->current_state != ch->state.current) {
			if (lcd_bpt_shadow->error != screen->bpt->error) {
				//screen->pages.channel.EWI_msg.pos.y += 32;
				UBA_GFX_draw_text_center(&screen->pages.channel.EWI_msg);
				//screen->pages.channel.EWI_msg.pos.y -= 32;
				//update shadow
				lcd_bpt_shadow->error = screen->bpt->error;
			}
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		if ((strcmp(lcd_bpt_shadow->btn_select.text, screen->pages.channel.btn_select.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->btn_select.color_bg != screen->pages.channel.btn_select.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->btn_select.color_text != screen->pages.channel.btn_select.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->btn_select.effect != screen->pages.channel.btn_select.effect)) {
			UBA_GFX_draw_button(&screen->pages.channel.btn_select);
			//update shadow
			strcpy(lcd_bpt_shadow->btn_select.text, screen->pages.channel.btn_select.elemnt.button.text);
		    lcd_bpt_shadow->btn_select.color_bg = screen->pages.channel.btn_select.elemnt.button.color_bg;
		    lcd_bpt_shadow->btn_select.color_text = screen->pages.channel.btn_select.elemnt.button.color_text;
		    lcd_bpt_shadow->btn_select.effect = screen->pages.channel.btn_select.effect;
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
	RTC_TimeTypeDef time;

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, do NOT draw 
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {
	//	return;
	//}
	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
		if (screen->bpt->ch->id != UBA_PROTO_CHANNEL_ID_A) {
			//draw channel A only
			return;
		}
	}

	//update current screen
	ch->current_screen = screen;

	UBA_LCD_screen_load_channel(&screen->pages.screen_bpt.channel, ch);

	//mate lcd screen
	//UBA_LCD_page_BPT *mate_lcd_bpt = NULL;
	//if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//	mate_lcd_bpt = (strcmp(screen->pages.screen_bpt.channel.ch_name.elemnt.text.text, "CH A")) ? &UBA_LCD_g.screen_ch_B : 
	//				   (strcmp(screen->pages.screen_bpt.channel.ch_name.elemnt.text.text, "CH B")) ? &UBA_LCD_g.screen_ch_A : NULL;
	//}

	//frame update
	lcd_bpt->frame.effect = UBA_GFX_EFFECT_SOLID;

	// channel Name update

	// Test
	if (((screen->bpt)) != NULL) {
		char channel_test_name[UBA_BPT_NAME_MAX_SIZE];

		channel_test = screen->bpt;

		UBA_BPT_get_tr_filename (screen->bpt, channel_test_name);
		if (strlen(channel_test_name)) {
			if (strcmp(lcd_bpt->test_name.elemnt.text.text, channel_test_name/*lcd_bpt_shadow->test_name*/) != 0) {
				sprintf(lcd_bpt->test_name.elemnt.text.text, "%s", channel_test_name);
				lcd_bpt->test_name.elemnt.text.text[10] = '\0'; //limit test name to 10 chars
				lcd_bpt->test_name.effect = UBA_GFX_EFFECT_SOLID;
				//update shadow
				test_name_changed = true;
				sprintf(lcd_bpt_shadow->test_name, "%s", lcd_bpt->test_name.elemnt.text.text);
			}
		} else {
			lcd_bpt->test_name.effect = UBA_GFX_EFFECT_INVISIBLE;
		}
	}
	else
	{
		screen->bpt->error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
		if (lcd_bpt_shadow->error != screen->bpt->error)
		{
			UART_LOG_ERROR(UBA_COMP, "channel control id %u in unknoun ", screen->ch_control);
			//update shadow - will be done later
			//lcd_bpt_shadow->error = screen->bpt->error;
		}
		return;
	}

	UBA_LCD_screen_getRunTime(screen->bpt, &time);
#if 0
	if (channel_test->state.current == UBA_BPT_STATE_RUN_STEP) {
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
	
		diff_seconds += screen->bpt->start_date_time.add_pause_seconds;

		hours   =  diff_seconds / 3600;
		minutes = (diff_seconds % 3600) / 60;
		seconds =  diff_seconds % 60;

	} else if (channel_test->state.current == UBA_BPT_STATE_PAUSE) {
		if (screen->bpt->start_date_time.update_pause_seconds == true) {
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
		
			screen->bpt->start_date_time.add_pause_seconds += diff_seconds;
			screen->bpt->start_date_time.update_pause_seconds = false;
		}

		hours   =  screen->bpt->start_date_time.add_pause_seconds / 3600;
		minutes = (screen->bpt->start_date_time.add_pause_seconds % 3600) / 60;
		seconds =  screen->bpt->start_date_time.add_pause_seconds % 60;

	} else {
		hours   = 0;
		minutes = 0;
		seconds = 0;
	}
#endif
	snprintf(lcd_bpt->time.elemnt.text.text, 9, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
	//if (mate_lcd_bpt) {
	//	snprintf(mate_lcd_bpt->time.elemnt.text.text, 9, "%02u:%02u:%02u", time.Hours, time.Minutes, time.Seconds);
	//}
	
	// EWI line
	//if (lcd_bpt_shadow->error != screen->bpt->error)
	{
		if (screen->bpt->error & UBA_BPT_CRITICAL) {
			lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_YELLOW;
			lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_RED;
			lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
		} else if (screen->bpt->error & UBA_BPT_ERROR) {
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

			if (screen->bpt->state.current == UBA_BPT_STATE_TEST_COMPLETE) {
				lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
				lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_GREEN;
				lcd_bpt->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
				snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, " Completed ");
			}
			else if (screen->bpt->state.current == UBA_BPT_STATE_TEST_FAILED) {
				lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
				lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_RED;
				snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_EWI_MAX_LINE_CHAR_SIZE, " Failed ");
			}
			else {
				if (screen->refresh_msg) {
					screen->bpt->error = UBA_PROTO_UBA6_ERROR_NO_ERROR;
					screen->refresh_msg = false;
				}

				if ((screen->bpt->error & UBA_PROTO_UBA6_ERROR_USER_ABORT) == UBA_PROTO_UBA6_ERROR_USER_ABORT) {
					lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
					lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_GREEN;
					snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_GFX_TEXT_MAX_LENGTH, "Stoped by User");
				}
				
				bool line_connected [2];
				bool is_connected = false;
				UBA_channel_get_lines_connected(screen->bpt->ch, line_connected);
				if (screen->ch_control == UBA_CHANNLE_ID_A || screen->ch_control == UBA_CHANNLE_ID_B) {
					is_connected = line_connected[0];
				} else /*CHANNEL_ID_AB*/ {
					is_connected = line_connected[0] || line_connected[1];
				}
				if (is_connected == false) {
				//if (UBA_channel_are_lines_connected (screen->bpt->ch) == false) { 
					screen->bpt->error = UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED;
				} else {
					screen->bpt->error &= (~UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED);
				}
				if ((screen->bpt->error & UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED) == UBA_PROTO_UBA6_ERROR_LINE_NOT_CONNECTED) {
					lcd_bpt->EWI_msg.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
					lcd_bpt->EWI_msg.elemnt.text.color_text = UBA_GFX_COLOR_RED;
					snprintf(lcd_bpt->EWI_msg.elemnt.text.text, UBA_GFX_TEXT_MAX_LENGTH, "Battery Disconnected");
				}
			}
		}
		if ((screen == &UBA_LCD_g.screen_ch_A) &&
			(screen->bpt->error) &&
			(TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) &&
			(strcmp(((TR_Test_Routine *) screen->bpt->tr)->name, ((TR_Test_Routine *) UBA_LCD_g.screen_ch_B.bpt->tr)->name) == 0)) {
				
			UBA_LCD_g.screen_ch_B.pages.screen_bpt.EWI_msg.elemnt.text.color_bg = lcd_bpt->EWI_msg.elemnt.text.color_bg;
			UBA_LCD_g.screen_ch_B.pages.screen_bpt.EWI_msg.elemnt.text.color_text = lcd_bpt->EWI_msg.elemnt.text.color_text;
			sprintf(UBA_LCD_g.screen_ch_B.pages.screen_bpt.EWI_msg.elemnt.text.text, lcd_bpt->EWI_msg.elemnt.text.text);
		}

		//update shadow - will be done later
		//lcd_bpt_shadow->current_state = channel_test->state.current;
	}

	//Status
	//if (ch->shadow.state != ch->state.current) {
	//	//updated already in load screen
	//	ch->shadow.state = UBA_CHANNEL_STATE_INIT;
	//} else {
	if (lcd_bpt_shadow->current_state != channel_test->state.current)
	{
		switch (screen->bpt->state.current) {
			case UBA_BPT_STATE_PAUSE:
				sprintf(lcd_bpt->channel.status.elemnt.status.text, "PAUSE");
				lcd_bpt->channel.status.elemnt.status.color_fill = UBA_GFX_COLOR_WHITE;
				lcd_bpt->channel.status.effect = UBA_GFX_EFFECT_BLINK_SLOW;
				ch->shadow.state = UBA_CHANNEL_STATE_INIT;
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
		//update shadow - will be done later
		//lcd_bpt_shadow->current_state = channel_test->state.current;
	}

	//Step	
	if (UBA_BPT_isRunning(channel_test)) {
		lcd_bpt->test_step.effect = UBA_GFX_EFFECT_SOLID;
		sprintf(lcd_bpt->test_step.elemnt.text.text, "STEP-%d", channel_test->current_step->step_index+1);
		////sprintf(lcd_bpt->test_step.elemnt.text.text, "%02u/%02u", channel_test->current_step->step_index, channel_test->last_step_index);
		//if (mate_lcd_bpt) {
		//	sprintf(mate_lcd_bpt->test_step.elemnt.text.text, "STEP-%d", channel_test->current_step->step_index+1);
		//}

	UART_LOG_SCREEN_INFO(UBA_COMP, "==> (screen) ch %s, tr-index %d, step: %d", 
		channel_test->ch->name, screen->tr_list_select_index, channel_test->current_step->step_index);

	} else {
		lcd_bpt->test_step.effect = UBA_GFX_EFFECT_INVISIBLE;
	}

	//Buttons
	if (UBA_BPT_isPause(screen->bpt)) {
		sprintf(lcd_bpt->btn_pause_start.elemnt.button.text, "RESUM");      
		sprintf(lcd_bpt->btn_back_stop.elemnt.button.text, "STOP");
		sprintf(lcd_bpt->btn_next.elemnt.button.text, "NEXT ");
		if (lcd_bpt->btn_next.effect == UBA_GFX_EFFECT_INVISIBLE) {
			lcd_bpt->btn_next.effect = UBA_GFX_EFFECT_VISIBLE;
		}
		if ((screen->tr != NULL && screen->bpt != NULL) &&
			((screen->bpt)->current_step->step_index+1 == (screen->tr)->length)) {
			UBA_LCD_page_BPT *lcd_ch = &screen->pages.screen_bpt;
			lcd_ch->btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;
		}

	} else if (UBA_BPT_isRunning(screen->bpt)) {
		sprintf(lcd_bpt->btn_pause_start.elemnt.button.text, "PAUSE");
		sprintf(lcd_bpt->btn_back_stop.elemnt.button.text, "STOP");
		sprintf(lcd_bpt->btn_next.elemnt.button.text, "NEXT ");
		if ((lcd_bpt->btn_back_stop.effect != UBA_GFX_EFFECT_SELECTED) &&
			(lcd_bpt->btn_next.effect == UBA_GFX_EFFECT_INVISIBLE)) {
			lcd_bpt->btn_next.effect = UBA_GFX_EFFECT_VISIBLE;
		}
		if ((screen->tr != NULL && screen->bpt != NULL) &&
			((screen->bpt)->current_step->step_index+1 == (screen->tr)->length)) {
			UBA_LCD_page_BPT *lcd_ch = &screen->pages.screen_bpt;
			lcd_ch->btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;
		}

	} else {
		sprintf(lcd_bpt->btn_pause_start.elemnt.button.text, "START");
		sprintf(lcd_bpt->btn_back_stop.elemnt.button.text, "BACK");
		if (lcd_bpt->btn_next.effect = UBA_GFX_EFFECT_INVISIBLE) {
			sprintf(lcd_bpt->btn_next.elemnt.button.text, "     ");
		}
	}
	
	lcd_bpt->btn_back_stop.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	lcd_bpt->btn_pause_start.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	lcd_bpt->btn_next.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;

	//draw BPT screen
	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&lcd_bpt->frame);

		//force drawing ALL info in case of LCD clean
		test_name_changed = true; 
		ch->shadow.ch_name_changed = true;
		lcd_channel_shadow->volt_vlaue_changed = true;
		lcd_channel_shadow->current_value_changed = true;
		lcd_channel_shadow->capacity_vlaue_changed = true;
		lcd_channel_shadow->temp_value_changed;
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		if ((ch->shadow.ch_name_changed == true) || (LCD_screen_force_draw == true)) {
			UBA_GFX_draw_text(&lcd_bpt->channel.ch_name);
			//update shadow
			ch->shadow.ch_name_changed = false;
		}
		if ((test_name_changed == true)  || (LCD_screen_force_draw == true)) {
			UBA_GFX_draw_text_center(&lcd_bpt->test_name);
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_STATUS) == UBA_LCD_REFRESH_TYPE_STATUS) {
		if ((lcd_bpt_shadow->current_state != channel_test->state.current) || (LCD_screen_force_draw == true)) {
			UBA_GFX_draw_status(&lcd_bpt->channel.status);
			if (lcd_bpt->test_step.effect != UBA_GFX_EFFECT_INVISIBLE) {
				UBA_GFX_draw_text(&lcd_bpt->test_step);
			} else {
				UBA_GFX_erase_text(&lcd_bpt->test_step);
			}
			//update shadow - will be done later
			//lcd_bpt_shadow->current_state = channel_test->state.current;
		}
	}

	int i;
	UBA_GFX measure[4];
	for (i=0; i< 4; i++)
	{
		measure[i].id = UBA_GFX_ELEMNET_TEXT;
		measure[i].effect = UBA_GFX_EFFECT_SOLID;
		measure[i].elemnt.text.size = 1;
		measure[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		measure[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	}			

	if ((rt & UBA_LCD_REFRESH_TYPE_DATA) == UBA_LCD_REFRESH_TYPE_DATA) {
		UBA_GFX_draw_text_center(&lcd_bpt->time);
		if ((lcd_channel_shadow->volt_vlaue_changed == true) || (LCD_screen_force_draw == true)) {
			i = 0;
			measure[i].pos.x = lcd_bpt->channel.volt.pos.x;
			measure[i].pos.y = lcd_bpt->channel.volt.pos.y-2;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  voltage:");
			UBA_GFX_draw_text(&measure[i]);

			lcd_bpt->channel.volt.pos.y += 8;
			UBA_GFX_draw_text(&lcd_bpt->channel.volt);
			lcd_bpt->channel.volt.pos.y -= 8;

			//update shadow
			lcd_channel_shadow->volt_vlaue_changed = false;
		}
		if ((lcd_channel_shadow->current_value_changed == true) || (LCD_screen_force_draw == true)) {
			i = 1;
			measure[i].pos.x = lcd_bpt->channel.current.pos.x;
			measure[i].pos.y = lcd_bpt->channel.current.pos.y+6;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  current:");
			UBA_GFX_draw_text(&measure[i]);

			lcd_bpt->channel.current.pos.y += 16;
			if ((screen->shadow.ch_control != screen->ch_control) && (screen->ch_control != UBA_CHANNLE_ID_AB)) {
				//do not draw current value;
			} else {
				//if (UBA_BPT_isRunning(channel_test)) {
					UBA_GFX_draw_text(&lcd_bpt->channel.current);
				//}
			}
			lcd_bpt->channel.current.pos.y -= 16;
			//update shadow
			lcd_channel_shadow->current_value_changed = false;
		}
		if ((lcd_channel_shadow->capacity_vlaue_changed == true) || (LCD_screen_force_draw == true)) {
			i = 2;
			measure[i].pos.x = lcd_bpt->channel.capacity.pos.x;
			measure[i].pos.y = lcd_bpt->channel.capacity.pos.y+14;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  capacity:");
			UBA_GFX_draw_text(&measure[i]);

			lcd_bpt->channel.capacity.pos.y += 24;
			if ((screen->shadow.ch_control != screen->ch_control) && (screen->ch_control != UBA_CHANNLE_ID_AB)) {
				//do not draw current value;
			} else {
				//if (UBA_BPT_isRunning(channel_test)) {
					UBA_GFX_draw_text(&lcd_bpt->channel.capacity);
				//}
			}
			lcd_bpt->channel.capacity.pos.y -= 24;
			//update shadow
			lcd_channel_shadow->capacity_vlaue_changed = false;
		}
		if ((lcd_channel_shadow->temp_value_changed == true) || (LCD_screen_force_draw == true)) {
			i = 3;
			measure[i].pos.x = lcd_bpt->channel.temp.pos.x;
			measure[i].pos.y = lcd_bpt->channel.temp.pos.y+22;
			//draw measure title
			sprintf(measure[i].elemnt.text.text, "  temperature:");
			UBA_GFX_draw_text(&measure[i]);

			lcd_bpt->channel.temp.pos.y += 32;
			UBA_GFX_draw_text(&lcd_bpt->channel.temp);
			lcd_bpt->channel.temp.pos.y -= 32;
			//update shadow
			lcd_channel_shadow->temp_value_changed = false;
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_EWI) == UBA_LCD_REFRESH_TYPE_EWI) {
		if (strlen (lcd_bpt->EWI_msg.elemnt.text.text)) {
			UBA_GFX_draw_text(&lcd_bpt->EWI_msg);
			memset(lcd_bpt->EWI_msg.elemnt.text.text, ' ', UBA_GFX_TEXT_MAX_LENGTH-8);
		}
	}

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		if ((strcmp(lcd_bpt_shadow->btn_back_stop.text, lcd_bpt->btn_back_stop.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->btn_back_stop.color_bg != lcd_bpt->btn_back_stop.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->btn_back_stop.color_text != lcd_bpt->btn_back_stop.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->btn_back_stop.effect != lcd_bpt->btn_back_stop.effect) ||
			(LCD_screen_force_draw == true)) {
			UBA_GFX_draw_button(&lcd_bpt->btn_back_stop);
			//update shadow
			strcpy(lcd_bpt_shadow->btn_back_stop.text, lcd_bpt->btn_back_stop.elemnt.button.text);
		    lcd_bpt_shadow->btn_back_stop.color_bg = lcd_bpt->btn_back_stop.elemnt.button.color_bg;
		    lcd_bpt_shadow->btn_back_stop.color_text = lcd_bpt->btn_back_stop.elemnt.button.color_text;
		    lcd_bpt_shadow->btn_back_stop.effect = lcd_bpt->btn_back_stop.effect;
		}
		if ((strcmp(lcd_bpt_shadow->btn_pause_start.text, lcd_bpt->btn_pause_start.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->btn_pause_start.color_bg != lcd_bpt->btn_pause_start.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->btn_pause_start.color_text != lcd_bpt->btn_pause_start.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->btn_pause_start.effect != lcd_bpt->btn_pause_start.effect) ||
			(LCD_screen_force_draw == true)) {
			UBA_GFX_draw_button(&lcd_bpt->btn_pause_start);
			//update shadow
			strcpy(lcd_bpt_shadow->btn_pause_start.text, lcd_bpt->btn_pause_start.elemnt.button.text);
		    lcd_bpt_shadow->btn_pause_start.color_bg = lcd_bpt->btn_pause_start.elemnt.button.color_bg;
		    lcd_bpt_shadow->btn_pause_start.color_text = lcd_bpt->btn_pause_start.elemnt.button.color_text;
		    lcd_bpt_shadow->btn_pause_start.effect = lcd_bpt->btn_pause_start.effect;
		}
		if ((strcmp(lcd_bpt_shadow->btn_next.text, lcd_bpt->btn_next.elemnt.button.text) != 0) ||
		    (lcd_bpt_shadow->btn_next.color_bg != lcd_bpt->btn_next.elemnt.button.color_bg) ||
		    (lcd_bpt_shadow->btn_next.color_text != lcd_bpt->btn_next.elemnt.button.color_text) ||
		    (lcd_bpt_shadow->btn_next.effect != lcd_bpt->btn_next.effect) ||
			(LCD_screen_force_draw == true)) {
			if (lcd_bpt->btn_next.effect != UBA_GFX_EFFECT_INVISIBLE) {
				UBA_GFX_draw_button(&lcd_bpt->btn_next);
			}
			else{
				UBA_GFX_erase_button(&lcd_bpt->btn_next);
			}
			//update shadow
			strcpy(lcd_bpt_shadow->btn_next.text, lcd_bpt->btn_next.elemnt.button.text);
		    lcd_bpt_shadow->btn_next.color_bg = lcd_bpt->btn_next.elemnt.button.color_bg;
		    lcd_bpt_shadow->btn_next.color_text = lcd_bpt->btn_next.elemnt.button.color_text;
		    lcd_bpt_shadow->btn_next.effect = lcd_bpt->btn_next.effect;
		}
	}

	//update shadow
	lcd_bpt_shadow->current_state = channel_test->state.current;
	lcd_bpt_shadow->error = screen->bpt->error;
}

void UBA_LCD_draw_screen(UBA_LCD_screen *screen) {

	if (screen->ch_control == UBA_CHANNLE_ID_A) {
		//draw channel B
		UBA_LCD_screen_draw_bpt(&UBA_LCD_g.screen_ch_B, UBA_LCD_REFRESH_TYPE_ALL);

	} else if (screen->ch_control == UBA_CHANNLE_ID_B) {
		//draw channel B
		UBA_LCD_screen_draw_bpt(&UBA_LCD_g.screen_ch_A, UBA_LCD_REFRESH_TYPE_ALL);

	} else if (screen->ch_control == UBA_CHANNLE_ID_AB) {
		//draw channel A
		UBA_LCD_screen_draw_bpt(&UBA_LCD_g.screen_ch_A, UBA_LCD_REFRESH_TYPE_ALL);

		//draw channel B
		UBA_LCD_screen_draw_bpt(&UBA_LCD_g.screen_ch_B, UBA_LCD_REFRESH_TYPE_ALL);
	}
}

bool UBA_LCD_screen_btn_next_invisible(UBA_LCD_screen *screen) {
	if ((screen->tr != NULL && screen->bpt != NULL) &&
		((screen->bpt)->current_step->step_index+2 == (screen->tr)->length)) {
		UBA_LCD_page_BPT *lcd_ch = &screen->pages.screen_bpt;
		if (lcd_ch->btn_next.effect == UBA_GFX_EFFECT_SELECTED) {
			lcd_ch->btn_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
			lcd_ch->btn_back_stop.effect = UBA_GFX_EFFECT_VISIBLE;
			lcd_ch->btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;
		}
	}

	return true;
}

/**
 * return true is back is presses
 */
bool UBA_LCD_screen_btn_press_select(UBA_LCD_screen *screen) {
	bool is_dual_channel = (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) ?
							true : false;

	if (screen->pages.screen_bpt.btn_pause_start.state == UBA_GFX_STATE_SELECTED) {
		if (UBA_BPT_isPause(screen->bpt)) {
			if (is_dual_channel) {
				UBA_LCD_g.screen_ch_A.bpt->start_date_time.add_pause_seconds = 0;
				UBA_LCD_g.screen_ch_A.tr = screen->tr;
				UBA_LCD_g.screen_ch_A.tr_list_select_index = screen->tr_list_select_index;
				UBA_TR_unpack(&TR_file.list[UBA_LCD_g.screen_ch_A.tr_list_select_index], UBA_LCD_g.screen_ch_A.bpt);
				UBA_LCD_g.screen_ch_A.bpt->TR_selected_index = UBA_LCD_g.screen_ch_A.tr_list_select_index;
				
				UBA_BPT_start(UBA_LCD_g.screen_ch_A.bpt);
			} else {
				screen->bpt->start_date_time.add_pause_seconds = 0;
				UBA_BPT_start(screen->bpt);
			}

		} else if (UBA_BPT_isRunning(screen->bpt)) {
			UBA_BPT_pause_test(screen->bpt);

		} else {
			if (is_dual_channel) {
				//keep/shadow
				UBA_LCD_g.screen_ch_A.shadow.tr = UBA_LCD_g.screen_ch_A.tr;
				UBA_LCD_g.screen_ch_A.shadow.tr_list_select_index = UBA_LCD_g.screen_ch_A.tr_list_select_index;
				
				UBA_LCD_g.screen_ch_A.bpt->start_date_time.add_pause_seconds = 0;
				UBA_LCD_g.screen_ch_A.tr = screen->tr;
				UBA_LCD_g.screen_ch_A.tr_list_select_index = screen->tr_list_select_index;
				UBA_TR_unpack(&TR_file.list[UBA_LCD_g.screen_ch_A.tr_list_select_index], UBA_LCD_g.screen_ch_A.bpt);
				UBA_LCD_g.screen_ch_A.bpt->TR_selected_index = UBA_LCD_g.screen_ch_A.tr_list_select_index;
				UBA_LCD_g.screen_ch_A.refresh_msg = true;

				UBA_BPT_start(UBA_LCD_g.screen_ch_A.bpt);
			} else {
				screen->bpt->start_date_time.add_pause_seconds = 0;
				screen->refresh_msg = true;

				UBA_BPT_start(screen->bpt);
			}
		}
		if (is_dual_channel) {
			UBA_LCD_screen_btn_next_invisible(&UBA_LCD_g.screen_ch_A);
		} else {
			UBA_LCD_screen_btn_next_invisible(screen);
		}

	} else if (screen->pages.screen_bpt.btn_back_stop.state == UBA_GFX_STATE_SELECTED) {
		if (UBA_BPT_isRunning(screen->bpt)) {
			if (is_dual_channel) {
				//retrieve/shadow
				UBA_LCD_g.screen_ch_A.tr = UBA_LCD_g.screen_ch_A.shadow.tr;
				UBA_LCD_g.screen_ch_A.tr_list_select_index = UBA_LCD_g.screen_ch_A.shadow.tr_list_select_index;
				UBA_TR_unpack(&TR_file.list[UBA_LCD_g.screen_ch_A.tr_list_select_index], UBA_LCD_g.screen_ch_A.bpt);
				UBA_LCD_g.screen_ch_A.bpt->TR_selected_index = UBA_LCD_g.screen_ch_A.tr_list_select_index;

				UBA_BPT_stop(UBA_LCD_g.screen_ch_A.bpt);
				UBA_LCD_screen_btn_next_invisible(&UBA_LCD_g.screen_ch_A);
			} else {
				UBA_BPT_stop(screen->bpt);
				UBA_LCD_screen_btn_next_invisible(screen);
			}
			screen->bpt->ch->state.current = UBA_CHANNEL_STATE_STANDBY;
			
		} else {
			return true; // go back
		}

	} else if (screen->pages.screen_bpt.btn_next.state == UBA_GFX_STATE_SELECTED) {
		if (UBA_BPT_isRunning(screen->bpt)) {
			if (is_dual_channel) {
				UBA_BPT_next(UBA_LCD_g.screen_ch_A.bpt);
				UBA_LCD_screen_btn_next_invisible(&UBA_LCD_g.screen_ch_A);
			} else {
				UBA_BPT_next(screen->bpt);
				UBA_LCD_screen_btn_next_invisible(screen);
			}
		}
		else if (UBA_BPT_isPause(screen->bpt)) {
			if (is_dual_channel) {
				UBA_LCD_screen_btn_next_invisible(&UBA_LCD_g.screen_ch_A);
			} else {
				UBA_LCD_screen_btn_next_invisible(screen);
			}

		} else {
			return true; // go back
		}
	}
	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI | UBA_LCD_REFRESH_TYPE_STATUS);
	return false;
}

int UBA_LCD_screen_line_max_str_length(UBA_LCD_screen *screen, uint8_t size) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	//UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];
	int ret = ((position->width - (BORDER_PADDING * 2)) / (CHAR_WIDTH * size)) - 1;
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

#if 1//end of list (eol)
	UBA_GFX eol;
	eol.id = UBA_GFX_ELEMNET_TEXT;
	eol.effect = UBA_GFX_EFFECT_SOLID;
	eol.elemnt.text.size = 1;
	eol.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	eol.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
#endif//eol

	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		uint16_t y_add = 0;
		for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
#if 1//end of list (eol)
			//clean eol
			eol.pos.x = lcd_test_select->test_name_list[i].pos.x;
			eol.pos.y = lcd_test_select->test_name_list[i].pos.y + 17;
			sprintf(eol.elemnt.text.text, "                       ");
			UBA_GFX_draw_text(&eol);
#endif//eol
			lcd_test_select->test_name_list[i].pos.y += y_add;
			UBA_GFX_draw_text(&lcd_test_select->test_name_list[i]);
			lcd_test_select->test_name_list[i].pos.y -= y_add;

			if ((i < UBA_LCD_MAX_DISPLAY_TEST_SELECT-1) &&
			    (lcd_test_select->list_end_index == i)) {
#if 1//end of list (eol)
				//draw eol
				sprintf(eol.elemnt.text.text, "  --end-of-list---------");
				UBA_GFX_draw_text(&eol);
#endif//eol
				y_add = 12;
			}
		}

		UBA_GFX_draw_button(&lcd_test_select->btn_back);
	}
}

//====================================================state machine functions============================================//

void UBA_LCD_screen_display_init_enter(UBA_LCD_screen *screen) {

	UBA_LCD_screen_update_state(screen);

	switch (screen->ch_control) {
		case UBA_CHANNLE_ID_A:
			screen->main_buttons.btn_up_p = &UBA_BTN_CH_1_UP;
			screen->main_buttons.btn_down_p = &UBA_BTN_CH_1_DOWN;
			screen->main_buttons.btn_select_p = &UBA_BTN_CH_1_SELECT;
			break;
		case UBA_CHANNLE_ID_B:
			screen->main_buttons.btn_up_p = &UBA_BTN_CH_2_UP;
			screen->main_buttons.btn_down_p = &UBA_BTN_CH_2_DOWN;
			screen->main_buttons.btn_select_p = &UBA_BTN_CH_2_SELECT;
			break;
		case UBA_CHANNLE_ID_AB:
			screen->main_buttons.btn_up_p = &UBA_BTN_CH_1_UP;
			screen->main_buttons.btn_down_p = &UBA_BTN_CH_1_DOWN;
			screen->main_buttons.btn_select_p = &UBA_BTN_CH_1_SELECT;
			screen->secondery_buttons.btn_up_p = &UBA_BTN_CH_2_UP;
			screen->secondery_buttons.btn_down_p = &UBA_BTN_CH_2_DOWN;
			screen->secondery_buttons.btn_select_p = &UBA_BTN_CH_2_SELECT;
			break;
		default:
			UART_LOG_ERROR(UBA_COMP, "channel control id %u in unknoun ", screen->ch_control);
		}

	screen->pages.test_list.start_display_index = 0;
	screen->pages.test_list.list_select_index = 0;
	screen->pages.test_list.list_end_index = UBA_TR_LIST_SIZE;
	screen->pages.test_list.btn_back.effect = UBA_GFX_EFFECT_SOLID;

	screen->tr_list_select_index = 0;
	screen->tr = &TR_file.list[screen->tr_list_select_index];

	//full selection:
	//screen->tr_list_select_index = 0;
	//screen->tr = &TR_file.list[screen->tr_list_select_index];
	//if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], screen->bpt) != 0) {
	//	UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
	//}

	//shadow
	screen->shadow.ch_control = screen->ch_control;
}

void UBA_LCD_screen_display_init(UBA_LCD_screen *screen) {
	if (UBA_BPT_isUnpacked(screen->bpt)) {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
	} else {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_CHANNEL;
	}
	screen->btn_active = true;
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

	channel->btn_select.effect = UBA_GFX_EFFECT_SELECTED;
	channel->btn_select.button_size = 2;
	channel->btn_select.button_color_bg = UBA_GFX_COLOR_WHITE;
	channel->btn_select.button_color_text = UBA_GFX_COLOR_BLACK;
	sprintf(channel->btn_select.button_text, "%s", "SELECT");

	channel->EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
	channel->EWI_msg.text_size = LINE_EWI_FONT_SIZE;
}

void UBA_LCD_screen_display_channel_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_LCD_STATIC_PAGE *channel = &LCD_handler->pages.channel;
	//UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];
	int y_offset = 7;
	
	UBA_LCD_screen_update_state(screen);

	screen->pages.channel.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.channel.frame.pos.x = position->start_x;
	screen->pages.channel.frame.pos.y = position->start_y;
	screen->pages.channel.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.frame.elemnt.frame.width = position->width;
	screen->pages.channel.frame.elemnt.frame.heigth = position->height;
	screen->pages.channel.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;
#if 0
	screen->pages.channel.test_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.test_name.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.channel.test_name.pos.y = LINE(LINE_TEST_NAME) - 2;
	screen->pages.channel.test_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.test_name.elemnt.text.text [0] = '\0';
	screen->pages.channel.test_name.elemnt.text.size = LINE_TEST_NAME_FONT_SIZE;
	screen->pages.channel.test_name.elemnt.text.color_bg = WHITE;
	screen->pages.channel.test_name.elemnt.text.color_text = BLACK;

	screen->pages.channel.time.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.time.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.channel.time.pos.y = LINE(LINE_TIME) - y_offset;
	screen->pages.channel.time.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.time.elemnt.text.size = LINE_TIME_FONT_SIZE;
	screen->pages.channel.time.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.time.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.channel.test_step.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.test_step.pos.x = position->start_x + BORDER_PADDING + ((position->width - (2 * BORDER_PADDING)) / 4) + 8;
	screen->pages.channel.test_step.pos.y = LINE(LINE_STEP) + 23;
	screen->pages.channel.test_step.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.test_step.elemnt.text.size = LINE_STEP_FONT_SIZE;
	screen->pages.channel.test_step.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.test_step.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
#endif
	screen->pages.channel.channel.ch_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.ch_name.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.channel.channel.ch_name.pos.y = LINE(LINE_CHANEL_NAME);
	screen->pages.channel.channel.ch_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.ch_name.elemnt.text.size = LINE_CHANEL_NAME_FONT_SIZE;
	screen->pages.channel.channel.ch_name.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.channel.channel.ch_name.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.channel.channel.status.id = UBA_GFX_ELEMNET_STATUS;
	screen->pages.channel.channel.status.pos.x = position->start_x + (((position->width - (2 * BORDER_PADDING)) * 13) / 16); /*center 7/8*/
	screen->pages.channel.channel.status.pos.y = LINE(1);
	screen->pages.channel.channel.status.effect = UBA_GFX_EFFECT_BLINK_FAST;
	screen->pages.channel.channel.status.elemnt.status.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.channel.volt.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.volt.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.channel.channel.volt.pos.y = LINE(LINE_V) - y_offset;
	screen->pages.channel.channel.volt.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.volt.elemnt.text.size = LINE_V_FONT_SIZE;
	screen->pages.channel.channel.volt.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.volt.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.channel.current.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.current.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.channel.channel.current.pos.y = LINE(LINE_C) - y_offset;
	screen->pages.channel.channel.current.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.current.elemnt.text.size = LINE_C_FONT_SIZE;
	screen->pages.channel.channel.current.elemnt.text.size = LCD_DATA_FONT_SIZE;
	screen->pages.channel.channel.current.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.current.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.channel.capacity.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.capacity.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.channel.channel.capacity.pos.y = LINE(LINE_CAP) - y_offset;
	screen->pages.channel.channel.capacity.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.capacity.elemnt.text.size = LINE_CAP_FONT_SIZE;
	screen->pages.channel.channel.capacity.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.capacity.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.channel.temp.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.channel.temp.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.channel.channel.temp.pos.y = LINE(LINE_TEMP) - y_offset;
	screen->pages.channel.channel.temp.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.channel.temp.elemnt.text.size = LINE_TEMP_FONT_SIZE;
	screen->pages.channel.channel.temp.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.channel.channel.temp.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.channel.btn_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.channel.btn_select.pos.x = position->start_x + position->width - 40;
	screen->pages.channel.btn_select.pos.y = BTN_LOWER_YPOS;
	screen->pages.channel.btn_select.effect = channel->btn_select.effect;
	screen->pages.channel.btn_select.elemnt.button.size = channel->btn_select.button_size;
	screen->pages.channel.btn_select.elemnt.button.color_bg = channel->btn_select.button_color_bg;
	screen->pages.channel.btn_select.elemnt.button.color_text = channel->btn_select.button_color_text;
	sprintf(screen->pages.channel.btn_select.elemnt.button.text, channel->btn_select.button_text);

	screen->pages.channel.EWI_msg.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.channel.EWI_msg.pos.x = position->start_x + BORDER_PADDING + 8;
	//screen->pages.channel.EWI_msg.pos.y = LINE(LINE_EWI) + 22;
	screen->pages.channel.EWI_msg.pos.y = LINE(LINE_STEP) + 12;
	screen->pages.channel.EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.channel.EWI_msg.elemnt.text.size = LINE_EWI_FONT_SIZE;

	// update shadow
	screen->pages.channel.channel.shadow.volt_vlaue = -1;
	screen->pages.channel.channel.shadow.current_vlaue = -1;
	screen->pages.channel.channel.shadow.capacity_vlaue = -1;
	screen->pages.channel.channel.shadow.temp_value = -1;
 
	screen->bpt->shadow.test_name[0] = '\0';
	screen->bpt->shadow.current_state = UBA_BPT_STATE_INVALID;
	screen->bpt->shadow.error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	screen->bpt->shadow.btn_select.text[0] = '\0';
	screen->bpt->shadow.btn_select.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.btn_select.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.btn_select.effect = UBA_GFX_EFFECT_MAX;
	screen->bpt->shadow.btn_pause_start.text[0] = '\0';
	screen->bpt->shadow.btn_pause_start.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.btn_pause_start.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.btn_pause_start.effect = UBA_GFX_EFFECT_MAX;

	UBA_LCD_screen_draw_channel(screen, UBA_LCD_REFRESH_TYPE_ALL);

	screen->start_tick = HAL_GetTick();
}

void UBA_LCD_screen_display_channel(UBA_LCD_screen *screen) {
	uint32_t refreshTime = ((screen->pages.channel.channel.status.effect == UBA_GFX_EFFECT_BLINK_FAST) ?
							UBA_LCD_FAST_REFRESH_TIME : UBA_LCD_SLOW_REFRESH_TIME);

	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
		screen->pages.screen_bpt.btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;

	}
	if ((HAL_GetTick() - screen->start_tick) >= refreshTime) {
		screen->start_tick = HAL_GetTick();
		UBA_LCD_screen_draw_channel(screen,
				UBA_LCD_REFRESH_TYPE_DATA | UBA_LCD_REFRESH_TYPE_STATUS | UBA_LCD_REFRESH_TYPE_UI | UBA_LCD_REFRESH_TYPE_EWI);
	}
}

void UBA_LCD_screen_display_channel_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

	//Moshe
//	UBA_CHANNLE_ID ch_control = screen->ch_control;
//	screen->ch_control = screen->shadow.ch_control;
//	screen->shadow.ch_control = ch_control;

	LCD_screen_force_draw = true;
	UBA_LCD_draw_screen(screen);
	LCD_screen_force_draw = false;
}

bool UBA_LCD_screen_bpt_update_position(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_CHANNLE_ID ch_control = screen->ch_control;
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	int y_offset = 7;
	bool is_change = false;

	if (screen->pages.screen_bpt.frame.pos.x != position->start_x) {
		screen->pages.screen_bpt.frame.pos.x = position->start_x;
		is_change = true;
	}
	if (screen->pages.screen_bpt.frame.pos.y != position->start_y) {
		screen->pages.screen_bpt.frame.pos.y = position->start_y;
		is_change = true;
	}
	if (screen->pages.screen_bpt.frame.elemnt.frame.width != position->width) {
		screen->pages.screen_bpt.frame.elemnt.frame.width = position->width;
		is_change = true;
	}
	if (screen->pages.screen_bpt.frame.elemnt.frame.heigth != position->height) {
		screen->pages.screen_bpt.frame.elemnt.frame.heigth = position->height;
		is_change = true;
	}

	if (is_change = true) {
		screen->pages.screen_bpt.test_name.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/

		screen->pages.screen_bpt.time.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/

		screen->pages.screen_bpt.test_step.pos.x = position->start_x + BORDER_PADDING + ((position->width - (2 * BORDER_PADDING)) / 4) + 8;

		screen->pages.screen_bpt.channel.ch_name.pos.x = position->start_x + BORDER_PADDING;

		screen->pages.screen_bpt.channel.status.pos.x = position->start_x + (((position->width - (2 * BORDER_PADDING)) * 13) / 16); /*center 7/8*/

		screen->pages.screen_bpt.channel.volt.pos.x = position->start_x + BORDER_PADDING;

		screen->pages.screen_bpt.channel.current.pos.x = position->start_x + BORDER_PADDING;

		screen->pages.screen_bpt.channel.capacity.pos.x = position->start_x + BORDER_PADDING;

		screen->pages.screen_bpt.channel.temp.pos.x = position->start_x + BORDER_PADDING;

		screen->pages.screen_bpt.btn_back_stop.pos.x = position->start_x + BORDER_PADDING + 30;

		screen->pages.screen_bpt.btn_pause_start.pos.x = position->start_x + position->width - 40;

		screen->pages.screen_bpt.btn_next.pos.x = position->start_x + position->width - 40;

		screen->pages.screen_bpt.EWI_msg.pos.x = position->start_x + BORDER_PADDING + 8;
	}
	return is_change;
}

void UBA_LCD_screen_display_bpt_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
	UBA_CHANNLE_ID ch_control = screen->ch_control;
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	int y_offset = 7;

	UBA_LCD_screen_update_state(screen);

	screen->pages.screen_bpt.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.screen_bpt.frame.pos.x = position->start_x;
	screen->pages.screen_bpt.frame.pos.y = position->start_y;
	screen->pages.screen_bpt.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.frame.elemnt.frame.width = position->width;
	screen->pages.screen_bpt.frame.elemnt.frame.heigth = position->height;
	screen->pages.screen_bpt.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.test_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.test_name.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.test_name.pos.y = LINE(LINE_TEST_NAME) - 2;
	screen->pages.screen_bpt.test_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.test_name.elemnt.text.text [0] = '\0';
	screen->pages.screen_bpt.test_name.elemnt.text.size = LINE_TEST_NAME_FONT_SIZE;
	screen->pages.screen_bpt.test_name.elemnt.text.color_bg = WHITE;
	screen->pages.screen_bpt.test_name.elemnt.text.color_text = BLACK;

	screen->pages.screen_bpt.time.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.time.pos.x = position->start_x + ((position->width - (2 * BORDER_PADDING)) / 2); /*center*/
	screen->pages.screen_bpt.time.pos.y = LINE(LINE_TIME) - y_offset;
	screen->pages.screen_bpt.time.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.time.elemnt.text.size = LINE_TIME_FONT_SIZE;
	screen->pages.screen_bpt.time.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.time.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.test_step.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.test_step.pos.x = position->start_x + BORDER_PADDING + ((position->width - (2 * BORDER_PADDING)) / 4) + 8;
	screen->pages.screen_bpt.test_step.pos.y = LINE(LINE_STEP) + 23;
	screen->pages.screen_bpt.test_step.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.test_step.elemnt.text.size = LINE_STEP_FONT_SIZE;
	screen->pages.screen_bpt.test_step.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.test_step.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.channel.ch_name.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.ch_name.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.ch_name.pos.y = LINE(LINE_CHANEL_NAME);
	screen->pages.screen_bpt.channel.ch_name.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.size = LINE_CHANEL_NAME_FONT_SIZE;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.screen_bpt.channel.ch_name.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.screen_bpt.channel.status.id = UBA_GFX_ELEMNET_STATUS;
	screen->pages.screen_bpt.channel.status.pos.x = position->start_x + (((position->width - (2 * BORDER_PADDING)) * 13) / 16); /*center 7/8*/
	screen->pages.screen_bpt.channel.status.pos.y = LINE(1);
	screen->pages.screen_bpt.channel.status.effect = UBA_GFX_EFFECT_BLINK_FAST;
	screen->pages.screen_bpt.channel.status.elemnt.status.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.channel.volt.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.volt.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.volt.pos.y = LINE(LINE_V) - y_offset;
	screen->pages.screen_bpt.channel.volt.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.volt.elemnt.text.size = LINE_V_FONT_SIZE;
	screen->pages.screen_bpt.channel.volt.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.volt.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.channel.current.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.current.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.current.pos.y = LINE(LINE_C) - y_offset;
	screen->pages.screen_bpt.channel.current.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.current.elemnt.text.size = LINE_C_FONT_SIZE;
	screen->pages.screen_bpt.channel.current.elemnt.text.size = LCD_DATA_FONT_SIZE;
	screen->pages.screen_bpt.channel.current.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.current.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.channel.capacity.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.capacity.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.capacity.pos.y = LINE(LINE_CAP) - y_offset;
	screen->pages.screen_bpt.channel.capacity.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.size = LINE_CAP_FONT_SIZE;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.capacity.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.channel.temp.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.channel.temp.pos.x = position->start_x + BORDER_PADDING;
	screen->pages.screen_bpt.channel.temp.pos.y = LINE(LINE_TEMP) - y_offset;
	screen->pages.screen_bpt.channel.temp.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.channel.temp.elemnt.text.size = LINE_TEMP_FONT_SIZE;
	screen->pages.screen_bpt.channel.temp.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	screen->pages.screen_bpt.channel.temp.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;

	screen->pages.screen_bpt.btn_back_stop.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.screen_bpt.btn_back_stop.pos.x = position->start_x + BORDER_PADDING + 30;
	screen->pages.screen_bpt.btn_back_stop.pos.y = BTN_LOWER_YPOS;
	screen->pages.screen_bpt.btn_back_stop.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.btn_back_stop.elemnt.button.size = 2;

	screen->pages.screen_bpt.btn_pause_start.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.screen_bpt.btn_pause_start.pos.x = position->start_x + position->width - 40;
	screen->pages.screen_bpt.btn_pause_start.pos.y = BTN_LOWER_YPOS;
	screen->pages.screen_bpt.btn_pause_start.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.screen_bpt.btn_pause_start.elemnt.button.size = 2;

	screen->pages.screen_bpt.btn_next.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.screen_bpt.btn_next.pos.x = position->start_x + position->width - 40;
	screen->pages.screen_bpt.btn_next.pos.y = BTN_UPPER_YPOS;
	screen->pages.screen_bpt.btn_next.effect = UBA_GFX_EFFECT_VISIBLE;
	screen->pages.screen_bpt.btn_next.elemnt.button.size = 2;

	screen->pages.screen_bpt.EWI_msg.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.screen_bpt.EWI_msg.pos.x = position->start_x + BORDER_PADDING + 8;
	//screen->pages.screen_bpt.EWI_msg.pos.y = LINE(LINE_EWI) + 22;
	screen->pages.screen_bpt.EWI_msg.pos.y = LINE(LINE_STEP) + 12;
	screen->pages.screen_bpt.EWI_msg.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.screen_bpt.EWI_msg.elemnt.text.size = LINE_EWI_FONT_SIZE;

	UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_ALL);

	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

	// update shadow
	screen->pages.screen_bpt.channel.shadow.volt_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.current_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.capacity_vlaue = -1;
	screen->pages.screen_bpt.channel.shadow.temp_value = -1;

	screen->bpt->shadow.test_name[0] = '\0';
	screen->bpt->shadow.current_state = UBA_BPT_STATE_INVALID;
	screen->bpt->shadow.error = UBA_PROTO_UBA6_ERROR_LINE_NOT_AVAILABLE;
	screen->bpt->shadow.btn_back_stop.text[0] = '\0';
	screen->bpt->shadow.btn_back_stop.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.btn_back_stop.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.btn_back_stop.effect = UBA_GFX_EFFECT_MAX;
	screen->bpt->shadow.btn_pause_start.text[0] = '\0';
	screen->bpt->shadow.btn_pause_start.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.btn_pause_start.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.btn_pause_start.effect = UBA_GFX_EFFECT_MAX;
	screen->bpt->shadow.btn_next.text[0] = '\0';
	screen->bpt->shadow.btn_next.color_text = GRAYBLUE;//not in use
	screen->bpt->shadow.btn_next.color_bg =  GRAYBLUE;//not in use
	screen->bpt->shadow.btn_next.effect = UBA_GFX_EFFECT_MAX;
}

bool UBA_LCD_screen_mate_display_bpt(UBA_LCD_screen *mate_screen, UBA_LCD_screen *screen) {
	bool pending = false;

	if (   (UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p))
		|| (UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
		memcpy (mate_screen->main_buttons.btn_up_p,          screen->main_buttons.btn_up_p, sizeof(UBA_button));
		memcpy (mate_screen->main_buttons.btn_down_p,        screen->main_buttons.btn_down_p, sizeof(UBA_button));
		memcpy (mate_screen->secondery_buttons.btn_up_p,     screen->secondery_buttons.btn_up_p, sizeof(UBA_button));
		memcpy (mate_screen->secondery_buttons.btn_down_p,   screen->secondery_buttons.btn_down_p, sizeof(UBA_button));

		UBA_button_clear_pending(screen->main_buttons.btn_up_p);
		UBA_button_clear_pending(screen->main_buttons.btn_down_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
		pending = true;
	}

	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		memcpy (mate_screen->main_buttons.btn_select_p,      screen->main_buttons.btn_select_p, sizeof(UBA_button));
		memcpy (mate_screen->secondery_buttons.btn_select_p, screen->secondery_buttons.btn_select_p, sizeof(UBA_button));

		UBA_button_clear_pending(screen->main_buttons.btn_select_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
		pending = true;
	}

	return pending;
}

void UBA_LCD_screen_display_bpt(UBA_LCD_screen *screen) {

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, buttons activate mate screen 
	UBA_LCD_screen *mate_screen = NULL;
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {

	//	mate_screen = (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) ? &UBA_LCD_g.screen_ch_B : 
	//				  (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) ? &UBA_LCD_g.screen_ch_A : NULL;

	//	if (UBA_LCD_screen_mate_display_bpt(mate_screen, screen)) {
	//		UBA_LCD_screen_display_bpt(mate_screen);
	//	}
	//	return;
	//}
	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
		if (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) {
			mate_screen = &UBA_LCD_g.screen_ch_A;

			if ((UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p)) ||
				(UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
				memcpy (mate_screen->main_buttons.btn_up_p,        screen->main_buttons.btn_up_p       , sizeof (UBA_button));
				memcpy (mate_screen->secondery_buttons.btn_up_p,   screen->secondery_buttons.btn_up_p  , sizeof (UBA_button));
				memcpy (mate_screen->main_buttons.btn_down_p,      screen->main_buttons.btn_down_p     , sizeof (UBA_button));
				memcpy (mate_screen->secondery_buttons.btn_down_p, screen->secondery_buttons.btn_down_p, sizeof (UBA_button));

				UBA_button_clear_pending(screen->main_buttons.btn_up_p);
				UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
				UBA_button_clear_pending(screen->main_buttons.btn_down_p);
				UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
			}
			if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
				memcpy (mate_screen->main_buttons.btn_select_p,      screen->main_buttons.btn_select_p     , sizeof (UBA_button));
				memcpy (mate_screen->secondery_buttons.btn_select_p, screen->secondery_buttons.btn_select_p, sizeof (UBA_button));
			
				UBA_button_clear_pending(screen->main_buttons.btn_select_p);
				UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
			}

			UBA_LCD_screen_display_bpt (mate_screen);
			return;
		}
	}

	uint32_t refreshTime = ((screen->pages.screen_bpt.channel.status.effect == UBA_GFX_EFFECT_BLINK_FAST) ?
							UBA_LCD_FAST_REFRESH_TIME : UBA_LCD_SLOW_REFRESH_TIME);

	if (   (UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p))
		|| (UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
		// Buttons: UP/DOWN
		UBA_LCD_screen_btn_press_up_or_down(screen, &screen->pages.screen_bpt);

		UBA_button_clear_pending(screen->main_buttons.btn_up_p);
		UBA_button_clear_pending(screen->main_buttons.btn_down_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	}

	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		//Button: SELECT
		if (UBA_LCD_screen_btn_press_select(screen)) {
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
			screen->pages.screen_bpt.btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;

		} else {
			UBA_button_clear_pending(screen->main_buttons.btn_select_p);
			UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
			UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
		}
	}

	if (screen->shadow.ch_control != screen->ch_control) {
		if (UBA_LCD_screen_bpt_update_position(screen)) {
			LCD_screen_force_draw = true;
			UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_ALL);
			LCD_screen_force_draw = false;
		}
		screen->shadow.ch_control = screen->ch_control;
	} 
	else
	if ((HAL_GetTick() - screen->start_tick) >= refreshTime) {
		screen->start_tick = HAL_GetTick();
		UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_DATA | UBA_LCD_REFRESH_TYPE_STATUS | UBA_LCD_REFRESH_TYPE_UI | UBA_LCD_REFRESH_TYPE_EWI);
	}
}

void UBA_LCD_screen_display_bpt_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

	screen->pages.screen_bpt.btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;

	//Moshe
	//UBA_CHANNLE_ID ch_control = screen->ch_control;
	//screen->ch_control = screen->shadow.ch_control;
	//screen->shadow.ch_control = ch_control;
//	screen->shadow.ch_control = screen->ch_control;
//	screen->ch_control = (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) ? UBA_CHANNLE_ID_A : 
//		  		         (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) ? UBA_CHANNLE_ID_B : UBA_CHANNLE_ID_A;
	
	LCD_screen_force_draw = true;
	UBA_LCD_draw_screen(screen);
	LCD_screen_force_draw = false;
}

void UBA_LCD_screen_display_test_select_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
//	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];
	int i, j;

	UBA_LCD_screen_update_state(screen);

	screen->pages.test_list.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_list.frame.pos.x = position->start_x;
	screen->pages.test_list.frame.pos.y = position->start_y;
	screen->pages.test_list.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_list.frame.elemnt.frame.width = position->width;
	screen->pages.test_list.frame.elemnt.frame.heigth = position->height;
	screen->pages.test_list.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_list.title.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.test_list.title.pos.x = position->width / 2 + position->start_x;
	screen->pages.test_list.title.pos.y = LINE(1);
	screen->pages.test_list.title.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_list.title.elemnt.text.size = 2;
	screen->pages.test_list.title.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.title.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_list.title.elemnt.text.text, "Test Select");

	screen->pages.test_list.btn_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_list.btn_back.pos.x = position->start_x + BORDER_PADDING + 30;
	screen->pages.test_list.btn_back.pos.y = BTN_LOWER_YPOS;

	screen->pages.test_list.btn_back.elemnt.button.size = 2;
	screen->pages.test_list.btn_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_list.btn_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_list.btn_back.elemnt.button.text, "BACK");

	for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
		screen->pages.test_list.test_name_list[i].id = UBA_GFX_ELEMNET_TEXT;
		screen->pages.test_list.test_name_list[i].pos.x = position->start_x + BORDER_PADDING;
		screen->pages.test_list.test_name_list[i].pos.y = LINE((4 + (i * 2)));
		j = (screen->pages.test_list.start_display_index+i) % UBA_TR_LIST_SIZE;
		sprintf(screen->pages.test_list.test_name_list[i].elemnt.text.text, "%d-%-s", j, TR_file.list[j].name);
		screen->pages.test_list.test_name_list[i].elemnt.text.text[12] = '\0'; //limit file name to 12
		screen->pages.test_list.test_name_list[i].effect = 
			(screen->pages.test_list.btn_back.effect == UBA_GFX_EFFECT_SELECTED) ? UBA_GFX_EFFECT_SOLID :
			(j == screen->pages.test_list.list_select_index) ? UBA_GFX_EFFECT_SELECTED : UBA_GFX_EFFECT_SOLID;
		screen->pages.test_list.test_name_list[i].elemnt.text.size = 2;
		screen->pages.test_list.test_name_list[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_list.test_name_list[i].elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	}

	UBA_LCD_screen_display_test_select_refresh(&screen->pages.test_list, UBA_LCD_REFRESH_TYPE_ALL);
}

void UBA_LCD_screen_display_test_select(UBA_LCD_screen *screen) {
	int i, j;
	bool need_to_refresh = false;
	int8_t list_select_index;
	int8_t start_display_index;

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, buttons activate mate screen 
	UBA_LCD_screen *mate_screen = NULL;
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {

	//	mate_screen = (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) ? &UBA_LCD_g.screen_ch_B : 
	//				  (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) ? &UBA_LCD_g.screen_ch_A : NULL;

	//	if (UBA_LCD_screen_mate_display_bpt(mate_screen, screen)) {
	//		UBA_LCD_screen_display_bpt(mate_screen);
	//	}
	//	return;
	//}

	list_select_index = screen->pages.test_list.list_select_index;
	start_display_index = screen->pages.test_list.start_display_index;

	if (UBA_button_is_pending(screen->main_buttons.btn_up_p) || 
		UBA_button_is_pending(screen->secondery_buttons.btn_up_p)) {
		UBA_button_clear_pending(screen->main_buttons.btn_up_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);

		if ((list_select_index-1 < 0) &&
			(screen->pages.test_list.btn_back.effect != UBA_GFX_EFFECT_SELECTED)) {
			/*BACK button*/
			screen->pages.test_list.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
		}
		else {
			/*Test info*/
			list_select_index--;
			if (list_select_index > start_display_index) {
			    if ((list_select_index - start_display_index) >= UBA_LCD_MAX_DISPLAY_TEST_SELECT) {
					start_display_index--;
				}
			} else if (start_display_index > list_select_index) {
				if ((UBA_TR_LIST_SIZE - (start_display_index - list_select_index)) >= UBA_LCD_MAX_DISPLAY_TEST_SELECT) {
					start_display_index--;
				}
			}

			list_select_index = (list_select_index+UBA_TR_LIST_SIZE) % UBA_TR_LIST_SIZE;
			start_display_index = (start_display_index+UBA_TR_LIST_SIZE) % UBA_TR_LIST_SIZE;
			screen->pages.test_list.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
		}
		need_to_refresh = true;

	} else if (UBA_button_is_pending(screen->main_buttons.btn_down_p) ||
			   UBA_button_is_pending(screen->secondery_buttons.btn_down_p)) {
		UBA_button_clear_pending(screen->main_buttons.btn_down_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
 
		if ((list_select_index+1 == UBA_TR_LIST_SIZE) &&
			(screen->pages.test_list.btn_back.effect != UBA_GFX_EFFECT_SELECTED)) {
			/*BACK button*/
			screen->pages.test_list.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
		}
		else {
			/*Test info*/
			list_select_index++;
			if (list_select_index > start_display_index) {
			    if ((list_select_index - start_display_index) >= UBA_LCD_MAX_DISPLAY_TEST_SELECT) {
					start_display_index++;
				}
			} else if (start_display_index > list_select_index) {
				if ((UBA_TR_LIST_SIZE - (start_display_index - list_select_index)) >= UBA_LCD_MAX_DISPLAY_TEST_SELECT) {
					start_display_index++;
				}
			}

			list_select_index = (list_select_index+UBA_TR_LIST_SIZE) % UBA_TR_LIST_SIZE;
			start_display_index = (start_display_index+UBA_TR_LIST_SIZE) % UBA_TR_LIST_SIZE;
			screen->pages.test_list.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
		}
		need_to_refresh = true;

	} else if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || 
			   UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		UBA_button_clear_pending(screen->main_buttons.btn_select_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

		if (screen->pages.test_list.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
			/*next: BACK button*/
//nothing selected: 
//			screen->tr = NULL;
			if (UBA_BPT_isUnpacked(screen->bpt)) {
//nothing selected: 
//				if (UBA_TR_unpack(&TR_file.list[list_select_index], screen->bpt) != 0) {
//					UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
//				}
				//Moshe
				//if (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
				//	screen->shadow.ch_control = screen->ch_control;
				//	screen->ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_AB; 
				//}
				////screen->bpt->ch->id = screen->ch_control;

				screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;

			} else {
				if ((screen->bpt != NULL) && (screen->tr != NULL)) {
					screen->bpt->type = screen->tr->mode == UBA_PROTO_BPT_MODE_SINGLE_CHANNEL ? 
										UBA_BPT_TYPE_SINGLE_CHANNEL : UBA_BPT_TYPE_DUAL_CHANNEL;
				}
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_CHANNEL;
			}

		} else {
			//selected file: display test info
			screen->tr_list_select_index = list_select_index;
			screen->tr = &TR_file.list[list_select_index];

//			if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], screen->bpt) != 0) {
//				UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
//			}
//
//			screen->bpt->TR_selected_index = screen->tr_list_select_index;

			/*next: Test info*/
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO;
		}
	}
	
	if (need_to_refresh) {
		if (list_select_index < UBA_TR_LIST_SIZE) {
			screen->pages.test_list.list_end_index = UBA_TR_LIST_SIZE;
			for (i = 0; i < UBA_LCD_MAX_DISPLAY_TEST_SELECT; i++) {
				j = (start_display_index+i) % UBA_TR_LIST_SIZE;
				sprintf(screen->pages.test_list.test_name_list[i].elemnt.text.text, "%d-%s     ", j, TR_file.list[j].name);
				screen->pages.test_list.test_name_list[i].elemnt.text.text[12] = '\0'; //limit file name to 12
				screen->pages.test_list.test_name_list[i].effect = 
					(screen->pages.test_list.btn_back.effect == UBA_GFX_EFFECT_SELECTED) ? UBA_GFX_EFFECT_SOLID :
					(j == list_select_index) ? UBA_GFX_EFFECT_SELECTED : UBA_GFX_EFFECT_SOLID;
				
				if (j == UBA_TR_LIST_SIZE-1) {
					screen->pages.test_list.list_end_index = i;
				}
			}
		}
		UBA_LCD_screen_display_test_select_refresh(&screen->pages.test_list, UBA_LCD_REFRESH_TYPE_ALL);
	}

	screen->pages.test_list.list_select_index = list_select_index;
	screen->pages.test_list.start_display_index = start_display_index;
}

void UBA_LCD_screen_display_test_select_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
}

void UBA_LCD_screen_dispaly_test_info_refresh(UBA_LCD_page_test_info *test_info, UBA_LCD_REFRESH_TYPE rt) {
	int i;
	if ((rt & UBA_LCD_REFRESH_TYPE_FRAME) == UBA_LCD_REFRESH_TYPE_FRAME) {
		UBA_GFX_draw_frame(&test_info->frame);
	}
	if ((rt & UBA_LCD_REFRESH_TYPE_INFO) == UBA_LCD_REFRESH_TYPE_INFO) {
		UBA_GFX_draw_text_center(&test_info->title);
		for (i = 0; i < 12; i++) {
			UBA_GFX_draw_text(&test_info->test_info[i]);
		}
	}
	if ((rt & UBA_LCD_REFRESH_TYPE_UI) == UBA_LCD_REFRESH_TYPE_UI) {
		UBA_GFX_draw_button(&test_info->btn_back);
		UBA_GFX_draw_button(&test_info->btn_step);
		UBA_GFX_draw_button(&test_info->btn_select);
	}
}

void UBA_LCD_screen_display_test_info_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
//	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];
	int i = 0;
	int from = 0;

	UBA_LCD_screen_update_state(screen);

	screen->pages.test_info.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_info.frame.pos.x = position->start_x;
	screen->pages.test_info.frame.pos.y = position->start_y;
	screen->pages.test_info.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.frame.elemnt.frame.width = position->width;
	screen->pages.test_info.frame.elemnt.frame.heigth = position->height;
	screen->pages.test_info.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.title.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.test_info.title.pos.x = position->width / 2 + position->start_x;
	screen->pages.test_info.title.pos.y = LINE(1);
	screen->pages.test_info.title.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.title.elemnt.text.size = 2;
	screen->pages.test_info.title.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.title.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	if ((screen->tr) != NULL) {
		sprintf(screen->pages.test_info.title.elemnt.text.text, "%.*s",
				UBA_LCD_screen_line_max_str_length(screen, screen->pages.test_info.title.elemnt.text.size), (screen->tr)->name);
	}

	for (i = 0; i < 11; i++) {
		screen->pages.test_info.test_info[i].id = UBA_GFX_ELEMNET_TEXT;
		screen->pages.test_info.test_info[i].pos.x = position->start_x;// + BORDER_PADDING;
		screen->pages.test_info.test_info[i].pos.y = (i % 2) == 0 ? LINE((4 + (i/2 * 3)))  : LINE((4 + (i/2 * 3 + 1)));
		screen->pages.test_info.test_info[i].effect = UBA_GFX_EFFECT_SOLID;
		screen->pages.test_info.test_info[i].elemnt.text.size = (i % 2) == 0 ? 1 : 2;
		screen->pages.test_info.test_info[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
//		sprintf(screen->pages.test_info.test_info[i].elemnt.text.text, "info[%d]", i);
	}

	from = 0;
	i = 0;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  battery P/N: ");
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].pos.x = screen->pages.test_info.test_info[i].pos.x + 12;
	screen->pages.test_info.test_info[i].pos.y = screen->pages.test_info.test_info[i].pos.y + 4;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %.*s", (screen->tr)->battery.part_number);
	
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  battery S/N: ");
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].pos.x = screen->pages.test_info.test_info[i].pos.x + 12;
	screen->pages.test_info.test_info[i].pos.y = screen->pages.test_info.test_info[i].pos.y + 4;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %.*s", (screen->tr)->battery.serial_number);
	
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  battery type: ");
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].pos.x = screen->pages.test_info.test_info[i].pos.x + 12;
	screen->pages.test_info.test_info[i].pos.y = screen->pages.test_info.test_info[i].pos.y + 4;
	switch (screen->tr->battery.type) {
		case UBA_BATTERY_TYPE_PRIMARY:
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  Primary");
			break;
		case UBA_BATTERY_TYPE_SECONDERY:
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  Secondary");
			break;
		default:
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  Unknown");
			break;
	}

	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  cell ser/par: mode:");
	screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 2;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "   %d/%d", 
			(screen->tr)->battery.num_cells_in_serial, (screen->tr)->battery.num_cells_in_parallel);

	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  num steps: ");
	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 2;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "   %d", (screen->tr)->length);


	i++;
	screen->pages.test_info.test_info[i].elemnt.text.size = 1;
	screen->pages.test_info.test_info[i].pos.x = screen->pages.test_info.test_info[i-3].pos.x + 96;
	screen->pages.test_info.test_info[i].pos.y = screen->pages.test_info.test_info[i-3].pos.y + 4;
	sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "%s", 
			(screen->tr)->mode == UBA_PROTO_BPT_MODE_SINGLE_CHANNEL ? "Single" : "Dual");


	screen->pages.test_info.btn_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_back.pos.x = position->start_x + BORDER_PADDING + 30;
	screen->pages.test_info.btn_back.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
	screen->pages.test_info.btn_back.elemnt.button.size = 2;
	screen->pages.test_info.btn_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_back.elemnt.button.text, "BACK");

	screen->pages.test_info.btn_step.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_step.pos.x = position->start_x + position->width - 40;
	screen->pages.test_info.btn_step.pos.y = BTN_UPPER_YPOS;
	screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.test_info.btn_step.elemnt.button.size = 2;
	screen->pages.test_info.btn_step.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_step.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_step.elemnt.button.text, "STEP  ");

	screen->pages.test_info.btn_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_select.pos.x = position->start_x + position->width - 40;
	screen->pages.test_info.btn_select.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
	screen->pages.test_info.btn_select.elemnt.button.size = 2;
	screen->pages.test_info.btn_select.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_select.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_select.elemnt.button.text, "SELECT");

	UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_ALL);
}

void UBA_LCD_screen_display_test_info(UBA_LCD_screen *screen) {

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, buttons activate mate screen 
	//UBA_LCD_screen *mate_screen = NULL;
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {

	//	mate_screen = (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) ? &UBA_LCD_g.screen_ch_B : 
	//				  (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) ? &UBA_LCD_g.screen_ch_A : NULL;

	//	if (UBA_LCD_screen_mate_display_bpt(mate_screen, screen)) {
	//		UBA_LCD_screen_display_bpt(mate_screen);
	//	}
	//}

	if ((UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p))
			|| (UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
		if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
			screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_SELECTED;
			screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;

		} else if (screen->pages.test_info.btn_step.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
			screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_VISIBLE;
			screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_SELECTED;

		} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
			screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_VISIBLE;
			screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
		}

		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);

		UBA_button_clear_pending(screen->main_buttons.btn_up_p);
		UBA_button_clear_pending(screen->main_buttons.btn_down_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	}

	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
			//BACK button
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_SELECT;
			screen->pages.screen_bpt.btn_next.effect = UBA_GFX_EFFECT_INVISIBLE;

		} else if (screen->pages.test_info.btn_step.effect == UBA_GFX_EFFECT_SELECTED) {
			//STEP button
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_STEP; 
			screen->tr_step_display_index = 0;

		} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
			//SELECT button
			screen->tr_list_select_index = screen->pages.test_list.list_select_index;

			if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], screen->bpt) != 0) {
				UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
			}

			//Moshe
			//if (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
			//	screen->shadow.ch_control = screen->ch_control;
			//	screen->ch_control = UBA_CHANNLE_ID_AB;
			//	UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_AB;
			//	UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_AB;
			//} 
			////screen->bpt->ch->id = screen->ch_control;

			screen->bpt->TR_selected_index = screen->tr_list_select_index;

			screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
		}

		UBA_button_clear_pending(screen->main_buttons.btn_select_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
}

void UBA_LCD_screen_display_test_info_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

}

void UBA_LCD_screen_display_test_step_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
//	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];
	int i = 0;
	int from = 0;

	UBA_LCD_screen_update_state(screen);

	screen->pages.test_info.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_info.frame.pos.x = position->start_x;
	screen->pages.test_info.frame.pos.y = position->start_y;
	screen->pages.test_info.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.frame.elemnt.frame.width = position->width;
	screen->pages.test_info.frame.elemnt.frame.heigth = position->height;
	screen->pages.test_info.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.title.id = UBA_GFX_ELEMNET_TEXT;
	screen->pages.test_info.title.pos.x = position->width / 2 + position->start_x;
	screen->pages.test_info.title.pos.y = LINE(1);
	screen->pages.test_info.title.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.title.elemnt.text.size = 2;
	screen->pages.test_info.title.elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.title.elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
	if ((screen->tr) != NULL) {
		if (((screen->tr)->config[screen->tr_step_display_index].type_id) == TR_STEP_TYPE_STEP_TYPE_CHARGE)
			sprintf(screen->pages.test_info.title.elemnt.text.text, "Charge");
		else if (((screen->tr)->config[screen->tr_step_display_index].type_id) == TR_STEP_TYPE_STEP_TYPE_DISCHARGE)
			sprintf(screen->pages.test_info.title.elemnt.text.text, "Discharge");
		else
			sprintf(screen->pages.test_info.title.elemnt.text.text, "Step %d", screen->tr_step_display_index+1);
	}

	for (int i = 0; i < 12; i++) {
		screen->pages.test_info.test_info[i].effect = UBA_GFX_EFFECT_INVISIBLE;
	}


	for (i = 0; i < 12; i++) {
		screen->pages.test_info.test_info[i].id = UBA_GFX_ELEMNET_TEXT;
		screen->pages.test_info.test_info[i].pos.x = position->start_x;// + BORDER_PADDING;
		screen->pages.test_info.test_info[i].pos.y = (i % 2) == 0 ? LINE((4 + (i/2 * 3))) - 6 : LINE((4 + (i/2 * 3 + 1))) - 6;
		screen->pages.test_info.test_info[i].effect = UBA_GFX_EFFECT_SOLID;
		screen->pages.test_info.test_info[i].elemnt.text.size = (i % 2) == 0 ? 1 : 2;
		screen->pages.test_info.test_info[i].elemnt.text.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLACK;
//		sprintf(screen->pages.test_info.test_info[i].elemnt.text.text, "info[%d]", i);
		sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "              ");
	}

#define MAX_CHARGE 0x7ffffff
	TR_config_step config[10];
	int index = screen->tr_step_display_index;
	switch ((screen->tr)->config[index].type_id)
	{
		case TR_STEP_TYPE_STEP_TYPE_CHARGE:
			from = 0;
			i = 0;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  charge current: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d mA", (screen->tr)->config[index].type.charge.current);

			//Moshe: not needed
			//i++;
			//screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			//sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  min temperature: ");
			//i++;
			//screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			//sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d C", (screen->tr)->config[index].type.charge.min_temperature); //float
			
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  max temperature: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d C", (screen->tr)->config[index].type.charge.sc.max_temperature); //float
			
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  cut-off current: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d mA", (screen->tr)->config[index].type.charge.sc.cut_off_current); 

			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  charge limit: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			if ((screen->tr)->config[index].type.charge.sc.charge_limit < MAX_CHARGE) {
				sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d mAh", (screen->tr)->config[index].type.charge.sc.charge_limit);
			} else {
				sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  undef");
			} 

			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  charge per cell: ");
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %.2f V", (float)(screen->tr)->config[index].type.charge.voltage/1000);
			
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  step: ");
			i++;
			screen->pages.test_info.test_info[i].pos.x += 12; 
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d of %d", index+1, (screen->tr)->length);
			break;

		case TR_STEP_TYPE_STEP_TYPE_DISCHARGE:
			from = 0;
			i = 0;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  discharge current: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  -%d mA", (screen->tr)->config[index].type.discharge.current.value);

			//Moshe: not needed
			//i++;
			//screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			//screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			//sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  min temperature: ");
			//i++;
			//screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			//sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d C", (screen->tr)->config[index].type.discharge.min_temperature); //float

			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  max temperature: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d C", (screen->tr)->config[index].type.discharge.sc.max_temperature); //float
			
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  cut-off voltage: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %3.2f V", ((float)(screen->tr)->config[index].type.discharge.sc.cut_off_voltag)/1000); 

			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  discharge limit: ");
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 2;
			if ((screen->tr)->config[index].type.discharge.sc.charge_limit < MAX_CHARGE) {
				sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d mAh", (screen->tr)->config[index].type.discharge.sc.charge_limit);
			} else {
				sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  undef");
			}
			
			i++;
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			screen->pages.test_info.test_info[i].elemnt.text.color_text = UBA_GFX_COLOR_BLUE;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  step: ");
			i++;
			screen->pages.test_info.test_info[i].pos.x += 12; 
			screen->pages.test_info.test_info[i].elemnt.text.size = 1;
			sprintf(&screen->pages.test_info.test_info[i].elemnt.text.text[from], "  %d of %d", index+1, (screen->tr)->length);
			break;
	}

	screen->pages.test_info.btn_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_back.pos.x = position->start_x + BORDER_PADDING + 30;
	screen->pages.test_info.btn_back.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
	screen->pages.test_info.btn_back.elemnt.button.size = 2;
	screen->pages.test_info.btn_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_back.elemnt.button.text, "BACK");

	if (screen->tr_step_display_index+1 < (screen->tr)->length) {
		screen->pages.test_info.btn_step.id = UBA_GFX_ELEMNET_BUTTON;
		screen->pages.test_info.btn_step.pos.x = position->start_x + position->width - 40;
		screen->pages.test_info.btn_step.pos.y = BTN_UPPER_YPOS;
		screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_SELECTED;
		screen->pages.test_info.btn_step.elemnt.button.size = 2;
		screen->pages.test_info.btn_step.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
		screen->pages.test_info.btn_step.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
		sprintf(screen->pages.test_info.btn_step.elemnt.button.text, "STEP  ");
	}
	else {
		screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_INVISIBLE;
		screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
	}

	screen->pages.test_info.btn_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_select.pos.x = position->start_x + position->width - 40;
	screen->pages.test_info.btn_select.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
	screen->pages.test_info.btn_select.elemnt.button.size = 2;
	screen->pages.test_info.btn_select.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_select.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_select.elemnt.button.text, "SELECT");

	UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_ALL);
}

void UBA_LCD_screen_display_test_step(UBA_LCD_screen *screen) {

	//check mate channel. if it's id is UBA_PROTO_CHANNEL_ID_AB --> BPT is not useable, buttons activate mate screen 
	//UBA_LCD_screen *mate_screen = NULL;
	//if (UBA_LCD_is_mate_channel_AB (screen) == true) {

	//	mate_screen = (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_A) ? &UBA_LCD_g.screen_ch_B : 
	//				  (screen->bpt->ch->id == UBA_PROTO_CHANNEL_ID_B) ? &UBA_LCD_g.screen_ch_A : NULL;

	//	if (UBA_LCD_screen_mate_display_bpt(mate_screen, screen)) {
	//		UBA_LCD_screen_display_bpt(mate_screen);
	//	}
	//}

	if ((UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p))
		|| (UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
		//UP/DOWN
		if (screen->tr_step_display_index+1 < (screen->tr)->length) {
			if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
				screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_SELECTED;
				screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;

			} else if (screen->pages.test_info.btn_step.effect == UBA_GFX_EFFECT_SELECTED) {
				screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_SELECTED;

			} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
				screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
				screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
			}

		} else {
			if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
				screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_SELECTED;
				screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_INVISIBLE;

			} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
				screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
				screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
				screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_INVISIBLE;
			}
		}

		UBA_button_clear_pending(screen->main_buttons.btn_up_p);
		UBA_button_clear_pending(screen->main_buttons.btn_down_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);

		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}

	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		if ((screen->tr != NULL && screen->bpt != NULL) &&
			((screen->bpt)->current_step->step_index+1 < (screen->tr)->length)) {
			if (screen->pages.test_info.btn_step.effect == UBA_GFX_EFFECT_SELECTED) {
				//STEP button
				screen->tr_step_display_index++;
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_STEP;

				if (screen->tr_step_display_index+1 == (screen->tr)->length) {
				 	screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
					screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_INVISIBLE;
					screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_VISIBLE;
				}

//				UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_ALL);

			} else if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
				//BACK button
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO; 
				screen->tr_step_display_index = 0;

			} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
				//SELECT button
				if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], screen->bpt) != 0) {
					UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
				}
				//Moshe
				//if (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
				//	screen->shadow.ch_control = screen->ch_control;
				//	screen->ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_AB;
				//} 
				////screen->bpt->ch->id = screen->ch_control;

				screen->bpt->TR_selected_index = screen->tr_list_select_index;

				screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
				screen->tr = NULL;
				screen->tr_step_display_index = 0;
			}
		}
		else {
			if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
				//BACK button
				screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO; 
				screen->tr_step_display_index = 0;
			
			} else if (screen->pages.test_info.btn_select.effect == UBA_GFX_EFFECT_SELECTED) {
				//SELECT button
				if (UBA_TR_unpack(&TR_file.list[screen->tr_list_select_index], screen->bpt) != 0) {
					UART_LOG_CRITICAL(UBA_COMP, "TR unpack Failed");
				}
				//Moshe
				//if (TR_file.list[screen->tr_list_select_index].mode == UBA_PROTO_BPT_MODE_DUAL_CHANNEL) {
				//	screen->shadow.ch_control = screen->ch_control;
				//	screen->ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_A.ch_control = UBA_CHANNLE_ID_AB;
				//	UBA_LCD_g.screen_ch_B.ch_control = UBA_CHANNLE_ID_AB;
				//} 
				////screen->bpt->ch->id = screen->ch_control;

				screen->bpt->TR_selected_index = screen->tr_list_select_index;

				screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
				screen->tr = NULL;
				screen->tr_step_display_index = 0;
			}

			UBA_button_clear_pending(screen->main_buttons.btn_select_p);
			UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

			UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
		}
	}
}

void UBA_LCD_screen_display_test_step_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);

}


void UBA_LCD_screen_display_setting_enter(UBA_LCD_screen *screen) {
	UBA_LCD *LCD_handler = (UBA_LCD *) screen->LCD_handler;
//	UBA_LCD_POSITION_INFO *position = LCD_handler->screen_position;
	UBA_CHANNLE_ID ch_control = screen->ch_control;

	//if (UBA_BPT_isRunning(screen->bpt) == false) {
	//	if (screen->ch_control == UBA_PROTO_CHANNEL_ID_AB) {
	//		ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_A :
	//					 (UBA_LCD_g.screen_ch_B.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? UBA_PROTO_CHANNEL_ID_B : UBA_PROTO_CHANNEL_ID_AB;
	//	}
	//}
	UBA_LCD_POSITION_INFO *position = &LCD_handler->screen_position[ch_control-1];
	//if (screen->bpt->ch->id == UBA_CHANNLE_ID_AB) {
	//	ch_control = (UBA_LCD_g.screen_ch_A.ch_control == UBA_PROTO_CHANNEL_ID_AB) ? 
	//				 UBA_PROTO_CHANNEL_ID_A : UBA_PROTO_CHANNEL_ID_B;
	//}
	//UBA_LCD_POSITION_INFO *position = UBA_BPT_isRunning(screen->bpt) ? 
	//								  &LCD_handler->screen_position[screen->ch_control-1] : 
	//								  &LCD_handler->screen_position[ch_control-1];

	UBA_LCD_screen_update_state(screen);

	screen->pages.test_info.frame.id = UBA_GFX_ELEMNET_FRAME;
	screen->pages.test_info.frame.pos.x = position->start_x;
	screen->pages.test_info.frame.pos.y = position->start_y;
	screen->pages.test_info.frame.effect = UBA_GFX_EFFECT_SOLID;
	screen->pages.test_info.frame.elemnt.frame.width = position->width;
	screen->pages.test_info.frame.elemnt.frame.heigth = position->height;
	screen->pages.test_info.frame.elemnt.frame.color_fill = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.frame.elemnt.frame.color_border = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.btn_back.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_back.pos.x = position->start_x + BORDER_PADDING + 30;
	screen->pages.test_info.btn_back.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_back.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.test_info.btn_back.elemnt.button.size = 2;
	screen->pages.test_info.btn_back.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_back.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;

	screen->pages.test_info.btn_step.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_step.pos.x = position->start_x + position->width - 40;
	screen->pages.test_info.btn_step.pos.y = BTN_UPPER_YPOS;
	screen->pages.test_info.btn_step.effect = UBA_GFX_EFFECT_SOLID; //UBA_GFX_EFFECT_INVISIBLE
	screen->pages.test_info.btn_step.elemnt.button.size = 2;
	screen->pages.test_info.btn_step.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_step.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_back.elemnt.button.text, "BACK");

	screen->pages.test_info.btn_select.id = UBA_GFX_ELEMNET_BUTTON;
	screen->pages.test_info.btn_select.pos.x = position->start_x + position->width - 40;
	screen->pages.test_info.btn_select.pos.y = BTN_LOWER_YPOS;
	screen->pages.test_info.btn_select.effect = UBA_GFX_EFFECT_SELECTED;
	screen->pages.test_info.btn_select.elemnt.button.size = 2;
	screen->pages.test_info.btn_select.elemnt.button.color_bg = UBA_GFX_COLOR_WHITE;
	screen->pages.test_info.btn_select.elemnt.button.color_text = UBA_GFX_COLOR_BLACK;
	sprintf(screen->pages.test_info.btn_select.elemnt.button.text, "SELECT");

	UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
}

void UBA_LCD_screen_display_setting(UBA_LCD_screen *screen) {
	if ((UBA_button_is_pending(screen->main_buttons.btn_up_p) || UBA_button_is_pending(screen->main_buttons.btn_down_p))
			|| (UBA_button_is_pending(screen->secondery_buttons.btn_up_p) || UBA_button_is_pending(screen->secondery_buttons.btn_down_p))) {
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
	if (UBA_button_is_pending(screen->main_buttons.btn_select_p) || UBA_button_is_pending(screen->secondery_buttons.btn_select_p)) {
		if (screen->pages.test_info.btn_back.effect == UBA_GFX_EFFECT_SELECTED) {
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_TEST_INFO;
		} 
		UBA_button_clear_pending(screen->main_buttons.btn_select_p);
		UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
		UBA_LCD_screen_dispaly_test_info_refresh(&screen->pages.test_info, UBA_LCD_REFRESH_TYPE_UI);
	}
}

void UBA_LCD_screen_display_setting_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
}

void UBA_LCD_screen_display_exe_cmd_enter(UBA_LCD_screen *screen) {
	UBA_LCD_screen_update_state(screen);

//	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
//	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
//	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
//	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
//	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
//	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
}

void UBA_LCD_screen_display_exe_cmd(UBA_LCD_screen *screen) {
UART_LOG(UBA_COMP, "exe_cmd: event %d", screen->event);

	switch (screen->event) {
		case UBA_LCD_SCREEN_DISPLAY_EVENT_TEST  :
			break;	
		case UBA_LCD_SCREEN_DISPLAY_EVENT_SELECT: /* start BPT */
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
			
			//emulate button select
			//UBA_button_set_pending(screen->main_buttons.btn_select_p);
			break;	

		case UBA_LCD_SCREEN_DISPLAY_EVENT_STOP  : /* stop bpt */
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
			break;	
		case UBA_LCD_SCREEN_DISPLAY_EVENT_PAUSE : /* pause bpt */
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
			break;	
		case UBA_LCD_SCREEN_DISPLAY_EVENT_CLEAR : /* Clear the BPT reset errro and set to STANDBY */
			return;	
		case UBA_LCD_SCREEN_DISPLAY_EVENT_STEP  : /* next-step bpt */
			screen->state.next = UBA_LCD_SCREEN_DISPLAY_BPT;
			break;	
	}
	screen->event = UBA_LCD_SCREEN_DISPLAY_EVENT_NONE;
}

void UBA_LCD_screen_display_exe_cmd_exit(UBA_LCD_screen *screen) {
	//UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	//UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	//UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	//UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	//UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	//UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
}

void UBA_LCD_screen_display_off_enter(UBA_LCD_screen *screen) {
	UBA_LCD_screen_update_state(screen);
}

void UBA_LCD_screen_display_off(UBA_LCD_screen *screen) {
}

void UBA_LCD_screen_display_off_exit(UBA_LCD_screen *screen) {
	UBA_button_clear_pending(screen->main_buttons.btn_up_p);
	UBA_button_clear_pending(screen->main_buttons.btn_down_p);
	UBA_button_clear_pending(screen->main_buttons.btn_select_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_up_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_down_p);
	UBA_button_clear_pending(screen->secondery_buttons.btn_select_p);
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

void UBA_LCD_screen_event(UBA_LCD_screen *screen, UBA_LCD_SCREEN_DISPLAY_STATE next_state, UBA_LCD_SCREEN_DISPLAY_EVENT event)
{
	//if (UBA_BPT_isUnpacked(screen->bpt)) {
		screen->state.next = next_state;
		screen->event = event;
	//} 
}

void UBA_LCD_screen_update(UBA_LCD_screen *screen)
{
	if (UBA_BPT_isUnpacked(screen->bpt)) {
		if (screen->bpt->state.next == UBA_BPT_STATE_TEST_COMPLETE) {
			UBA_LCD_screen_btn_press_up_or_down(screen, &screen->pages.screen_bpt);
		}
		UBA_LCD_screen_draw_bpt(screen, UBA_LCD_REFRESH_TYPE_UI);
	}
	return;
}

bool UBA_LCD_screen_isLastStep(UBA_LCD_screen *screen)
{
	UBA_BPT *bpt = screen->bpt;

	if (bpt != NULL) {
		if (UBA_BPT_isRunning(bpt)) {
			if (bpt->current_step->step_index == (screen->tr)->length-1) {
				return true;
			}
		}
	}

	return false;
}
