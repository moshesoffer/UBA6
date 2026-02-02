/*
 * LCD.h
 *
 *  Created on: Sep 9, 2024
 *      Author: ORA
 */

#ifndef LCD_H_
#define LCD_H_

#include "stdint.h"
#include "UBA_channel.h"
#include "stdbool.h"
#include "UBA_GFX.h"
#include "UBA_LCD_screen.h"
#include "UBA_button.h"

typedef enum UBA_LCD_STATE {
	UBA_LCD_STATE_INIT, // show splash screen
	UBA_LCD_STATE_SIDE_BY_SIDE, // show the channel data and info
	UBA_LCD_STATE_FULL_SCREEN, // focus on ch1 and display the test to be selected
	UBA_LCD_STATE_OFF, // the screen is off
	UBA_LCD_STATE_MAX,
	UBA_LCD_STATE_INVALID,
} UBA_LCD_STATE;



typedef enum UBA_LCD_TEST_SELECT {
	UBA_LCD_TEST_SELECT_NONE = 0x0000,
	UBA_LCD_TEST_SELECT_VISIBLE = 0x0001,
	UBA_LCD_TEST_SELECT_SELECTED = 0x0001,
	UBA_LCD_TEST_SELECT_BLINK = 0x0,
	UBA_LCD_TEST_SELECT_TEST,
} UBA_LCD_TEST_SELECT;


typedef struct UBA_LCD {
	struct {
		UBA_LCD_STATE pre;
		UBA_LCD_STATE current;
		UBA_LCD_STATE next;
	} state;
	uint16_t width;
	uint16_t height;
	uint8_t orientation;
	UBA_GFX main_frame;
	UBA_LCD_screen screen_ch_A;
	UBA_LCD_screen screen_ch_B;
	UBA_LCD_screen screen_ch_AB;
} UBA_LCD;

// @formatter:off
#define UBA_LCD_DEFAULT	 	\
{							\
	{UBA_LCD_STATE_INVALID,UBA_LCD_STATE_INIT, UBA_LCD_STATE_INIT},\
	ST7789_SCREEN_WIDTH,  	\
	ST7789_SCREEN_HEIGHT, 	\
	SCREEN_HORIZONTAL_2, 	\
	UBA_GFX_FRAME_DEFALT,	\
	UBA_LCD_SCREEN_DEFUALT,\
	UBA_LCD_SCREEN_DEFUALT,\
	UBA_LCD_SCREEN_DEFUALT\
}

// @formatter:on

extern UBA_LCD UBA_LCD_g; // the all screen
void LCD_run(UBA_LCD *LCD);
//void LCD_init(UBA_LCD *LCD);
void LCD_dispay_ch(UBA_LCD *LCD, UBA_channel *ch);

#endif /* LCD_H_ */

