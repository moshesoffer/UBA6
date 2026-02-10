/*
 * UBA_GFX.h
 *
 *  Created on: Sep 12, 2024
 *      Author: ORA
 */

#ifndef UBA_GFX_H_
#define UBA_GFX_H_

#include "stdint.h"
#define UBA_GFX_TEXT_MAX_LENGTH (32)

#define UBA_GFX_STATUS_RADIUS_A (23)
#define UBA_GFX_STATUS_RADIUS_B (13)

#define UBA_GFX_COLOR_WHITE       	0xFFFF
#define UBA_GFX_COLOR_BLACK       	0x0000
#define UBA_GFX_COLOR_BLUE        	0x001F
#define UBA_GFX_COLOR_RED         	0xF800
#define UBA_GFX_COLOR_MAGENTA     	0xF81F
#define UBA_GFX_COLOR_GREEN       0x07E0
#define UBA_GFX_COLOR_CYAN        0x7FFF
#define UBA_GFX_COLOR_YELLOW      0xFFE0
#define UBA_GFX_COLOR_GRAY        0x8430
#define UBA_GFX_COLOR_BRED        0xF81F
#define UBA_GFX_COLOR_GRED        0xFFE0
#define UBA_GFX_COLOR_GBLUE       0x07FF
#define UBA_GFX_COLOR_BROWN       0xBC40
#define UBA_GFX_COLOR_BRRED       0xFC07
#define UBA_GFX_COLOR_DARKBLUE    0x01CF
#define UBA_GFX_COLOR_LIGHTBLUE   0x7D7C
#define UBA_GFX_COLOR_GRAYBLUE    0x5458


#define UBA_GFX_COLOR_RUN		  	0x966a
#define UBA_GFX_COLOR_DELAY		   	0x057d
#define UBA_GFX_COLOR_STANDBY		0xffff
#define UBA_GFX_COLOR_OFF 			0xf9e7

#define UBA_GFX_COLOR_INFO 			0xffff
#define UBA_GFX_COLOR_WARNNING 		0xffe0
#define UBA_GFX_COLOR_ERROR 		0xf9e7
#define UBA_GFX_COLOR_CRITICAL 		UBA_GFX_COLOR_ERROR

#define UBA_GFX_COLOR_LIGHTGREEN  0X841F
#define UBA_GFX_COLOR_LGRAY       0XC618
#define UBA_GFX_COLOR_LGRAYBLUE   0XA651
#define UBA_GFX_COLOR_LBBLUE      0X2B12

typedef enum UBA_GFX_ELEMNET_ID {
	UBA_GFX_ELEMNET_BUTTON,
	UBA_GFX_ELEMNET_DATA,
	UBA_GFX_ELEMNET_HEADLINE,
	UBA_GFX_ELEMNET_TEXT,
	UBA_GFX_ELEMNET_STATUS,
	UBA_GFX_ELEMNET_FRAME,
	UBA_GFX_ELEMNET_MAX,

} UBA_GFX_ELEMNET_ID;

typedef enum UBA_GFX_STATE {
	UBA_GFX_STATE_INVISIBLE,
	UBA_GFX_STATE_VISIBLE,
	UBA_GFX_STATE_BLINK_OFF,
	UBA_GFX_STATE_BLINK_ON,
	UBA_GFX_STATE_SELECTED,

} UBA_GFX_STATE;

typedef enum UBA_GFX_EFFECT {
	UBA_GFX_EFFECT_INVISIBLE = 0x00,
	UBA_GFX_EFFECT_VISIBLE = 0x01, // if the button is current on the display
	UBA_GFX_EFFECT_SOLID = 0x02 | UBA_GFX_EFFECT_VISIBLE, // the button  in solid mode
	UBA_GFX_EFFECT_BLINK = 0x04 | UBA_GFX_EFFECT_VISIBLE, // the button is slow blinking
	UBA_GFX_EFFECT_BLINK_SLOW = 0x08 | UBA_GFX_EFFECT_BLINK, // the button is slow blinking
	UBA_GFX_EFFECT_BLINK_FAST = 0x10 | UBA_GFX_EFFECT_BLINK, // the button is fast blinking
	UBA_GFX_EFFECT_SELECTED = 0x20 | UBA_GFX_EFFECT_VISIBLE,
	UBA_GFX_EFFECT_MAX,
} UBA_GFX_EFFECT;

typedef enum UBA_GFX_ERROR {
	UBA_GFX_ERROR_NO_ERROR = 0x00,
	UBA_GFX_ERROR_ID = 0x01,
} UBA_GFX_ERROR;

typedef struct UBA_GFX_frame {
	uint16_t heigth;
	uint16_t width;
	uint16_t color_fill;
	uint16_t color_border;
} UBA_GFX_frame;

typedef struct UBA_GFX_button {
	char text[10];
	uint8_t size;
	uint16_t color_text;
	uint16_t color_bg;
} UBA_GFX_button;

typedef struct UBA_GFX_text {
	char text[UBA_GFX_TEXT_MAX_LENGTH];
	uint8_t size;
	uint16_t color_text;
	uint16_t color_bg;
} UBA_GFX_text;

typedef struct UBA_GFX_data {
	char text[10];
	uint8_t size;
	uint16_t color_text;
	uint16_t color_bg;
} UBA_GFX_data;

typedef struct UBA_GFX_headline {
	char text[10];
	uint8_t size;
	uint16_t color_text;
	uint16_t color_bg;
} UBA_GFX_headline;

typedef struct UBA_GFX_status {
	struct{
		uint8_t a;
		uint8_t b;
	}raduios;
	char text[11];
	uint16_t color_fill;
	uint16_t color_border;
	uint16_t color_text;
	uint16_t color_bg;
} UBA_GFX_status;

typedef struct UBA_GFX {
	UBA_GFX_ELEMNET_ID id;
	UBA_GFX_STATE state;
	UBA_GFX_EFFECT effect;
	struct {
		uint16_t x;
		uint16_t y;
	} pos;
	union {
		UBA_GFX_button button;
		UBA_GFX_data data;
		UBA_GFX_headline headline;
		UBA_GFX_status status;
		UBA_GFX_frame frame;
		UBA_GFX_text text;
	} elemnt;
} UBA_GFX;

//@formatter:off

#define UBA_GFX_BUTTON_DEFALT 	\
{								\
	UBA_GFX_ELEMNET_BUTTON,		\
	UBA_GFX_STATE_INVISIBLE,	\
	UBA_GFX_EFFECT_INVISIBLE,	\
	{0,0},						\
	.elemnt ={.button ={"",1,UBA_GFX_COLOR_BLACK,UBA_GFX_COLOR_WHITE}} \
}

#define UBA_GFX_FRAME_DEFALT 	\
{								\
	UBA_GFX_ELEMNET_FRAME,		\
	UBA_GFX_STATE_INVISIBLE,	\
	UBA_GFX_EFFECT_INVISIBLE,	\
	{0,0},						\
	.elemnt ={.frame ={0,0,UBA_GFX_COLOR_WHITE,UBA_GFX_COLOR_BLACK}} \
}

#define UBA_GFX_TEXT_DEFALT 	\
{								\
	UBA_GFX_ELEMNET_TEXT,		\
	UBA_GFX_STATE_INVISIBLE,	\
	UBA_GFX_EFFECT_INVISIBLE,	\
	{0,0},						\
	.elemnt ={.text  ={"",1,UBA_GFX_COLOR_BLACK,UBA_GFX_COLOR_WHITE}} \
}

#define UBA_GFX_STATUS_DEFALT 	\
{								\
	UBA_GFX_ELEMNET_STATUS,		\
	UBA_GFX_STATE_INVISIBLE,	\
	UBA_GFX_EFFECT_INVISIBLE,	\
	{0,0},						\
	.elemnt ={.status={{UBA_GFX_STATUS_RADIUS_A,UBA_GFX_STATUS_RADIUS_B},"status?",UBA_GFX_COLOR_LIGHTBLUE,UBA_GFX_COLOR_BLACK,UBA_GFX_COLOR_BLACK,UBA_GFX_COLOR_WHITE}} \
}

//@formatter:on

UBA_GFX_ERROR UBA_GFX_draw_frame(UBA_GFX *uba_gfx);
UBA_GFX_ERROR UBA_GFX_draw_button(UBA_GFX *uba_gfx);
UBA_GFX_ERROR UBA_GFX_draw_status(UBA_GFX *uba_gfx);
UBA_GFX_ERROR UBA_GFX_draw_text(UBA_GFX *uba_gfx);
UBA_GFX_ERROR UBA_GFX_draw_text_center(UBA_GFX *uba_gfx);
#endif /* UBA_GFX_H_ */

