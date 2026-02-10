/*
 * UBA_LCD_screen.h
 *
 *  Created on: Sep 22, 2024
 *      Author: ORA
 */

#ifndef UBA_LCD_SCREEN_H_
#define UBA_LCD_SCREEN_H_

#include "stdint.h"
#include "stdbool.h"
#include "UBA_GFX.h"
#include "UBA_channel.h"
#include "UBA_button.h"
#include "UBA_battery_performance_test.h"
#include "UBA_test_routine.h"

#define TEST_SELECT_LIST_SIZE (7)

typedef enum UBA_LCD_REFRESH_TYPE {
	UBA_LCD_REFRESH_TYPE_NONE = 0x00,
	UBA_LCD_REFRESH_TYPE_FRAME = 0x01,
	UBA_LCD_REFRESH_TYPE_INFO = 0x02,
	UBA_LCD_REFRESH_TYPE_STATE = 0x04,
	UBA_LCD_REFRESH_TYPE_STATUS = 0x08 | UBA_LCD_REFRESH_TYPE_STATE, /*status always include state*/
	UBA_LCD_REFRESH_TYPE_DATA = 0x10,
	UBA_LCD_REFRESH_TYPE_UI = 0x20,
	UBA_LCD_REFRESH_TYPE_EWI = 0x40,
	UBA_LCD_REFRESH_TYPE_ALL = 0xff

} UBA_LCD_REFRESH_TYPE;

typedef enum UBA_LCD_SCREEN_DISPLAY_STATE {
	UBA_LCD_SCREEN_DISPLAY_INIT,
	UBA_LCD_SCREEN_DISPLAY_CHANNEL, // show the channel data and info
	UBA_LCD_SCREEN_DISPLAY_BPT, // show the BPT data and info
	UBA_LCD_SCREEN_DISPLAY_TEST_SELECT, // show the list of test
	UBA_LCD_SCREEN_DISPLAY_TEST_INFO, // show the test info
	UBA_LCD_SCREEN_DISPLAY_SETTING, // show the device settings
	UBA_LCD_SCREEN_DISPLAY_OFF, // turn off the display
	UBA_LCD_SCREEN_STATE_MAX,
	UBA_LCD_SCREEN_STATE_INVALID,

} UBA_LCD_SCREEN_DISPLAY_STATE;

typedef struct UBA_LCD_channel_shadow
{
	float volt_vlaue;
	float current_vlaue;
	float capacity_vlaue;
	float temp_value;

	bool volt_vlaue_changed;
	bool current_vlaue_changed;
	bool capacity_vlaue_changed;
	bool temp_value_changed;
} UBA_LCD_channel_shadow;

typedef struct UBA_LCD_channel {
	UBA_GFX ch_name;
	UBA_GFX status;
	UBA_GFX volt;
	UBA_GFX current;
	UBA_GFX capacity;
	UBA_GFX temp;

	UBA_LCD_channel_shadow shadow;
} UBA_LCD_channel;


typedef struct UBA_LCD_page_channel {

	UBA_GFX frame;
	UBA_LCD_channel channel;
	UBA_GFX EWI_msg;
	UBA_GFX bnt_select;
} UBA_LCD_page_channel;

// @formatter:off
#define UBA_LCD_PAGE_CHANNEL_DEFUALT 					\
{													\
	UBA_GFX_FRAME_DEFALT, 	/*frame*/				\
	UBA_GFX_TEXT_DEFALT, 	/*channel name*/		\
	UBA_GFX_STATUS_DEFALT, 	/*status*/				\
	UBA_GFX_TEXT_DEFALT, 	/**Channel Volt*/		\
	UBA_GFX_TEXT_DEFALT, 	/*Channel capacity*/	\
	UBA_GFX_TEXT_DEFALT, 	/*channel Temp*/		\
	UBA_GFX_TEXT_DEFALT, 	/*channel EWI Message*/	\
	UBA_GFX_TEXT_DEFALT, 	/*EWI message*/			\
	UBA_GFX_BUTTON_DEFALT, 	/*Test Back/Stop Bnt*/	\
}

// @formatter:on

typedef struct UBA_LCD_page_BPT {
	UBA_GFX frame;
	UBA_GFX test_name;
	UBA_GFX test_step;
	UBA_GFX time;
	UBA_LCD_channel channel;
	UBA_GFX EWI_msg;
	UBA_GFX bnt_back_stop;
	UBA_GFX bnt_pause_start;
} UBA_LCD_page_BPT;

// @formatter:off
#define UBA_LCD_PAGE_BPT_DEFUALT 					\
{													\
	UBA_GFX_FRAME_DEFALT, 	/*frame*/				\
	UBA_GFX_TEXT_DEFALT, 	/*channel name*/		\
	UBA_GFX_STATUS_DEFALT, 	/*status*/				\
	UBA_GFX_TEXT_DEFALT, 	/*Test Name*/			\
	UBA_GFX_TEXT_DEFALT, 	/*Test Step counter*/	\
	UBA_GFX_TEXT_DEFALT, 	/*Test Time*/			\
	UBA_GFX_TEXT_DEFALT, 	/**Channel Volt*/		\
	UBA_GFX_TEXT_DEFALT, 	/*Channel capacity*/	\
	UBA_GFX_TEXT_DEFALT, 	/*channel Temp*/		\
	UBA_GFX_TEXT_DEFALT, 	/*channel EWI Message*/	\
	UBA_GFX_TEXT_DEFALT, 	/*EWI message*/			\
	UBA_GFX_BUTTON_DEFALT, 	/*Test Back/Stop Bnt*/	\
	UBA_GFX_BUTTON_DEFALT  	/*Test pause/start Bnt*/\
}
// @formatter:on
typedef struct UBA_LCD_page_bpt_channel {
	UBA_GFX frame;
	UBA_GFX ch_name;
	UBA_GFX status;
	UBA_GFX volt;
	UBA_GFX current;
	UBA_GFX capacity;
	UBA_GFX temp;
	UBA_GFX EWI_msg;
} UBA_LCD_page_bpt_channel;

typedef struct UBA_LCD_page_test_list_select {
	UBA_GFX frame;
	UBA_GFX title;
	UBA_GFX test_name_list[TEST_SELECT_LIST_SIZE];
	UBA_GFX bnt_cancel;
	int8_t select_index;
	int8_t list_select_index;

} UBA_LCD_page_test_list_select;

// @formatter:off
#define UBA_LCD_SCREEN_TEST_SELECT_DEFUALT \
{									\
	UBA_GFX_FRAME_DEFALT, 			\
	UBA_GFX_TEXT_DEFALT, 			\
	{								\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT,		\
		UBA_GFX_TEXT_DEFALT			\
	}, 								\
	0,								\
	0								\
}

// @formatter:on
typedef struct UBA_LCD_page_test_info {
	UBA_GFX frame;
	UBA_GFX title;
	UBA_GFX test_info[6];
	UBA_GFX EWI_msg;
	UBA_GFX bnt_back;
	UBA_GFX bnt_select;
} UBA_LCD_page_test_info;
// @formatter:off

#define UBA_LCD_SCREEN_TEST_INFO_DEFUALT 	\
{											\
	UBA_GFX_FRAME_DEFALT, 					\
	UBA_GFX_TEXT_DEFALT, 					\
	{										\
		UBA_GFX_TEXT_DEFALT,				\
		UBA_GFX_TEXT_DEFALT,				\
		UBA_GFX_TEXT_DEFALT,				\
		UBA_GFX_TEXT_DEFALT,				\
		UBA_GFX_TEXT_DEFALT,				\
		UBA_GFX_TEXT_DEFALT,				\
	}, 										\
	UBA_GFX_TEXT_DEFALT,					\
	UBA_GFX_BUTTON_DEFALT, 	/*Test Back/Stop Bnt*/	\
	UBA_GFX_BUTTON_DEFALT  	/*Test pause/start Bnt*/\
}
// @formatter:on

typedef struct UBA_LCD_screen_shadow {
	char test_name[UBA_BPT_NAME_MAX_SIZE];
	UBA_BPT_STATE current_state;
	UBA_PROTO_UBA6_ERROR error;
	struct {
		UBA_GFX_EFFECT effect;
		uint16_t color_text;
		uint16_t color_bg;
		char text[11];
	} bnt_back_stop;
	struct {
		UBA_GFX_EFFECT effect;
		uint16_t color_text;
		uint16_t color_bg;
		char text[11];
	} bnt_pause_start;
} UBA_LCD_screen_shadow;

typedef struct UBA_LCD_screen {
	void *LCD_handler;
	struct {
		UBA_LCD_SCREEN_DISPLAY_STATE pre;
		UBA_LCD_SCREEN_DISPLAY_STATE current;
		UBA_LCD_SCREEN_DISPLAY_STATE next;
	} state;
	uint32_t start_tick;
	struct {
		UBA_LCD_page_channel channel;
		UBA_LCD_page_BPT screen_bpt;
		UBA_LCD_page_test_list_select test_list;
		UBA_LCD_page_test_info test_info;
	} pages;
	struct {
		UBA_button *bnt_up_p;
		UBA_button *bnt_down_p;
		UBA_button *bnt_select_p;
	} main_buttons;
	struct {
		UBA_button *bnt_up_p;
		UBA_button *bnt_down_p;
		UBA_button *bnt_select_p;
	} secondery_buttons;
	TR_Test_Routine *tr;
	UBA_BPT *bpt;
	UBA_CHANNLE_ID ch_control; // the control id of the screen
	uint8_t tr_list_select_index;
} UBA_LCD_screen;

// @formatter:off
#define UBA_LCD_SCREEN_DEFUALT 		\
{									\
	{								\
		UBA_LCD_SCREEN_DISPLAY_INIT,\
		UBA_LCD_SCREEN_DISPLAY_INIT,\
		UBA_LCD_SCREEN_DISPLAY_INIT	\
	},								\
	0,								\
	0,								\
	0,								\
	0,								\
	0,								\
	.pages = {.test_info=UBA_LCD_SCREEN_TEST_INFO_DEFUALT}, \
	{NULL,NULL,NULL},				\
	{NULL,NULL,NULL},				\
	NULL,							\
	NULL,							\
	UBA_CHANNLE_ID_NONE,			\
	0,								\
}

typedef struct UBA_LCD_STATIC_PAGE {
	struct {
		UBA_GFX_EFFECT effect;
		uint16_t color_fill;
	} frame;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
		uint16_t text_color_bg;
		uint16_t text_color_text;
	} ch_name;
	struct {
		UBA_GFX_EFFECT effect;
	} status;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
		uint16_t text_color_bg;
		uint16_t text_color_text;
	} volt;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
		uint16_t text_color_bg;
		uint16_t text_color_text;
	} current;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
		uint16_t text_color_bg;
		uint16_t text_color_text;
	} capacity;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
		uint16_t text_color_bg;
		uint16_t text_color_text;
	} temp;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t button_size;
		uint16_t button_color_bg;
		uint16_t button_color_text;
		char button_text[10];
	} bnt_select;
	struct {
		UBA_GFX_EFFECT effect;
		uint8_t text_size;
	} EWI_msg;
} UBA_LCD_STATIC_PAGE;

// @formatter:on
void UBA_LCD_screen_run(UBA_LCD_screen *screen);

#endif /* UBA_LCD_SCREEN_H_ */

