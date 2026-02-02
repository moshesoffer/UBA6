/*
 * LCD.c
 *
 *  Created on: Sep 9, 2024
 *      Author: ORA
 */
#include "LCD.h"

#include "uart_log.h"
#include "ST7789_GFX.h"
#include "ST7789_STM32_Driver.h"
#include "5x5_font.h"
#include "string.h"
#include "stdio.h"
#include "UBA_GFX.h"
#include "UBA_button.h"
#include "UBA_battery_performance_test.h"
#include "UBA_6.h"

#define START_X (0)
#define START_Y (0)
#define UBA_COMP "LCD"
#define HEAD_LINE_SIZE (2)
#define DATA_LINE_SIZE (2)
#define STATUS_CHAR_SIZE (1)
#define BORDER_PADDING (5) /*5 pixel border padding*/
#define TEXT_COLOR BLACK
#define BACKGROUND_COLOR WHITE
#define LCD_DATA_FONT_SIZE (2)
#define LCD_BUTTON_BORADER_PAD (30)

#define STATUS_INDECATOR_RADIUS (CHAR_HEIGHT*2)

#define LINE_H 	(CHAR_HEIGHT + 2) /*2 pixel from line to line*/
#define LINE0_Y (START_Y+BORDER_PADDING)
#define LINE2_Y (LINE0_Y + LINE_H)
#define LINE3_Y (LINE2_Y + LINE_H)
#define DATA_FIRST_LINE (4)
// @formatter:off
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
// @formatter:on

#define UBA_LCD_MAX_DISPLAY_TEST_SELECT (7)

#define LINE(x) ((LINE_H*x) + LINE0_Y)

UBA_LCD UBA_LCD_g = UBA_LCD_DEFAULT;

/*State Machine function  */

static void LCD_init_enter(UBA_LCD *LCD);
static void LCD_init(UBA_LCD *LCD);
static void LCD_init_exit(UBA_LCD *LCD);

static void LCD_side_by_side_enter(UBA_LCD *LCD);
static void LCD_side_by_side(UBA_LCD *LCD);
static void LCD_side_by_side_exit(UBA_LCD *LCD);

static void LCD_full_screen_enter(UBA_LCD *LCD);
static void LCD_full_screen(UBA_LCD *LCD);
static void LCD_full_screen_exit(UBA_LCD *LCD);

static void LCD_off_enter(UBA_LCD *LCD);
static void LCD_off(UBA_LCD *LCD);
static void LCD_off_exit(UBA_LCD *LCD);

static void LCD_off_enter(UBA_LCD *LCD);
static void LCD_off(UBA_LCD *LCD);
static void LCD_off_exit(UBA_LCD *LCD);

typedef void (*step_cb_t)(UBA_LCD *LCD);
/***
 * UBA LCD State Machine Assigner Rule
 */
struct UBALCDSMA_rule {
	step_cb_t enter;
	step_cb_t run;
	step_cb_t exit;
};
/*BA LCD State Machine Assigner */
#define UBALCDSMA(step, cbe, cbr, cbx)[step] = {.enter = (step_cb_t)cbe, .run = (step_cb_t)cbr, .exit = (step_cb_t)cbx}

//@formatter:off

static const struct UBALCDSMA_rule rule_g[UBA_LCD_STATE_MAX] ={
		//				State
	UBALCDSMA(UBA_LCD_STATE_INIT,				LCD_init_enter,				LCD_init,			LCD_init_exit),
	UBALCDSMA(UBA_LCD_STATE_SIDE_BY_SIDE,		LCD_side_by_side_enter,		LCD_side_by_side,	LCD_side_by_side_exit),
	UBALCDSMA(UBA_LCD_STATE_FULL_SCREEN,		LCD_full_screen_enter,		LCD_full_screen,	LCD_full_screen_exit),
	UBALCDSMA(UBA_LCD_STATE_OFF,				LCD_off_enter,				LCD_off,			LCD_off_exit),


};
//@formatter:on

void LCD_run(UBA_LCD *LCD) {
	if (LCD->state.next == UBA_LCD_STATE_INVALID) { // if there the next state is not define , then run this state function
		if (rule_g[LCD->state.current].run) {
			rule_g[LCD->state.current].run(LCD); // run the main function of the state
		}
	} else {
		if (LCD->state.current < UBA_LCD_STATE_MAX) {
			if (rule_g[LCD->state.current].exit) {
				rule_g[LCD->state.current].exit(LCD); // run the status exit function
			}
		}
		if (rule_g[LCD->state.next].enter) {
			rule_g[LCD->state.next].enter(LCD); // run the next state enter function
		}
	}
}

static void LCD_update_state(UBA_LCD *LCD) {
	UART_LOG_INFO(UBA_COMP, "update state %u ---> %u", LCD->state.current, LCD->state.next);
	LCD->state.pre = LCD->state.current;
	LCD->state.current = LCD->state.next;
	LCD->state.next = UBA_LCD_STATE_INVALID;

}

void LCD_init_g(UBA_LCD *LCD) {
	ST7789_Init();
	ST7789_Set_Rotation(LCD->orientation);
}

static void LCD_init_enter(UBA_LCD *LCD) {
	UBA_GFX uba_title = { 0 };
	ST7789_Init();

	ST7789_Set_Rotation(LCD->orientation);
	LCD_update_state(LCD);
	LCD->main_frame.id = UBA_GFX_ELEMNET_FRAME;
	LCD->main_frame.effect = UBA_GFX_EFFECT_SOLID;
	LCD->main_frame.pos.x = START_X;
	LCD->main_frame.pos.y = START_Y;
	LCD->main_frame.elemnt.frame.heigth = LCD->height;
	LCD->main_frame.elemnt.frame.width = LCD->width;
	LCD->main_frame.elemnt.frame.color_border = BLACK;
	LCD->main_frame.elemnt.frame.color_fill = BACKGROUND_COLOR;
	UBA_GFX_draw_frame(&LCD->main_frame);

	LCD->screen_ch_A.bpt = &UBA_6_device_g.BPT_A;
	LCD->screen_ch_A.start_x = START_X;
	LCD->screen_ch_A.start_y = START_Y;
	LCD->screen_ch_A.width = LCD->width / 2;
	LCD->screen_ch_A.height = LCD->height;
	LCD->screen_ch_A.ch_control = UBA_CHANNLE_ID_A;

	LCD->screen_ch_B.bpt = &UBA_6_device_g.BPT_B;
	LCD->screen_ch_B.start_x = START_X + LCD->width / 2;
	LCD->screen_ch_B.start_y = START_Y;
	LCD->screen_ch_B.width = LCD->width / 2;
	LCD->screen_ch_B.height = LCD->height;
	LCD->screen_ch_B.ch_control = UBA_CHANNLE_ID_B;

	LCD->screen_ch_AB.bpt = &UBA_6_device_g.BPT_AB;
	LCD->screen_ch_AB.start_x = START_X;
	LCD->screen_ch_AB.start_y = START_Y;
	LCD->screen_ch_AB.width = LCD->width;
	LCD->screen_ch_AB.height = LCD->height;
	LCD->screen_ch_AB.ch_control = UBA_CHANNLE_ID_AB;

	sprintf(uba_title.elemnt.text.text, "Amicell - UBA 6");
	uba_title.id = UBA_GFX_ELEMNET_TEXT;
	uba_title.effect = UBA_GFX_EFFECT_SOLID;
	uba_title.pos.x = LCD->width / 2;
	uba_title.pos.y = LCD->height / 4;
	uba_title.elemnt.text.color_bg = WHITE;
	uba_title.elemnt.text.color_text = BLACK;
	uba_title.elemnt.text.size = 2;
	UBA_GFX_draw_text_center(&uba_title);
	snprintf(uba_title.elemnt.text.text,UBA_GFX_TEXT_MAX_LENGTH, "Ver:%02u.%02u.%02u.%02u", UBA_6_device_g.firmware.major, UBA_6_device_g.firmware.minor,
			UBA_6_device_g.firmware.patch, UBA_6_device_g.firmware.build);
	uba_title.id = UBA_GFX_ELEMNET_TEXT;
	uba_title.effect = UBA_GFX_EFFECT_SOLID;
	uba_title.pos.x = LCD->width / 2;
	uba_title.pos.y = (LCD->height / 4)*3;
	uba_title.elemnt.text.color_bg = WHITE;
	uba_title.elemnt.text.color_text = BLACK;
	uba_title.elemnt.text.size = 2;
	UBA_GFX_draw_text_center(&uba_title);
}

static void LCD_init(UBA_LCD *LCD) {
	LCD->state.next = UBA_LCD_STATE_SIDE_BY_SIDE;
}

static void LCD_init_exit(UBA_LCD *LCD) {
	UNUSED(LCD);
}

static void LCD_side_by_side_enter(UBA_LCD *LCD) {
	LCD_update_state(LCD);
	LCD->screen_ch_A.state.next = UBA_LCD_SCREEN_DISPLAY_INIT;
	LCD->screen_ch_B.state.next = UBA_LCD_SCREEN_DISPLAY_INIT;

}

static void LCD_side_by_side(UBA_LCD *LCD) {
	UBA_LCD_screen_run(&LCD->screen_ch_A);
	UBA_LCD_screen_run(&LCD->screen_ch_B);
	if ((LCD->screen_ch_A.state.current == UBA_LCD_SCREEN_DISPLAY_OFF) || (LCD->screen_ch_B.state.current == UBA_LCD_SCREEN_DISPLAY_OFF)) {
		LCD->state.next = UBA_LCD_STATE_FULL_SCREEN;
	}
}

static void LCD_side_by_side_exit(UBA_LCD *LCD) {
	LCD->screen_ch_A.state.next = UBA_LCD_SCREEN_DISPLAY_OFF;
	LCD->screen_ch_B.state.next = UBA_LCD_SCREEN_DISPLAY_OFF;
}

static void LCD_full_screen_enter(UBA_LCD *LCD) {
	LCD_update_state(LCD);
	LCD->screen_ch_AB.state.next = UBA_LCD_SCREEN_DISPLAY_INIT;
}
static void LCD_full_screen(UBA_LCD *LCD) {
	UBA_LCD_screen_run(&LCD->screen_ch_AB);
}

static void LCD_full_screen_exit(UBA_LCD *LCD) {
	LCD->screen_ch_AB.state.next = UBA_LCD_SCREEN_DISPLAY_OFF;
}

static void LCD_off_enter(UBA_LCD *LCD) {
	LCD_update_state(LCD);
}
static void LCD_off(UBA_LCD *LCD) {
	/*do noting*/
	UNUSED(LCD);
}
static void LCD_off_exit(UBA_LCD *LCD) {
	UNUSED(LCD);
}

