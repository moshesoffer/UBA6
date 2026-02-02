/*
 * UBA_GFX.c
 *
 *  Created on: Sep 12, 2024
 *      Author: ORA
 */

#include "UBA_GFX.h"
#include "ST7789_GFX.h"
#include "string.h"
#include "strings.h"
#include "5x5_font.h"
#include "uart_log.h"

#define UBA_CHAR_HEIGHT CHAR_HEIGHT
#define UBA_CHAR_WIDTH CHAR_WIDTH

#define UBA_GFX_MIN(a,b) (((a)<(b))?(a):(b))
#define UBA_GFX_MAX(a,b) (((a)>(b))?(a):(b))
#define UBA_GFX_BUTTON_FRAME_OFFSET (6)

#define UBA_COMP "UBA_GFX"

UBA_GFX_ERROR UBA_GFX_draw_frame(UBA_GFX *uba_gfx) {
	static uint16_t x = 0;
	static uint16_t y = 0;
	if (uba_gfx->id != UBA_GFX_ELEMNET_FRAME) {
		UART_LOG_ERROR(UBA_COMP, "ID is not Frame");
		return UBA_GFX_ERROR_ID;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_VISIBLE) == 0) {
		return UBA_GFX_ERROR_NO_ERROR;
	}
	x = uba_gfx->pos.x;
	y = uba_gfx->pos.y;

	ST7789_Draw_Filled_Rectangle_Coord(x, y, x + uba_gfx->elemnt.frame.width - 1, y + uba_gfx->elemnt.frame.heigth - 1,
			uba_gfx->elemnt.frame.color_fill);
	ST7789_Draw_Hollow_Rectangle_Coord(x, y, x + uba_gfx->elemnt.frame.width - 1, y + uba_gfx->elemnt.frame.heigth - 1,
			uba_gfx->elemnt.frame.color_border);

	return UBA_GFX_ERROR_NO_ERROR;
}

UBA_GFX_ERROR UBA_GFX_draw_button(UBA_GFX *uba_gfx) {
	uint16_t x = 0;
	uint16_t y = 0;
	UBA_GFX frame = { 0 };
	frame.id = UBA_GFX_ELEMNET_FRAME;
	frame.effect = uba_gfx->effect;
	if (uba_gfx->id != UBA_GFX_ELEMNET_BUTTON) {
		return UBA_GFX_ERROR_ID;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_VISIBLE) == 0) {
		return UBA_GFX_ERROR_NO_ERROR;
	}
	x = uba_gfx->pos.x - ((strlen(uba_gfx->elemnt.button.text) * (uba_gfx->elemnt.button.size * CHAR_WIDTH)) / 2);
	y = uba_gfx->pos.y - (uba_gfx->elemnt.button.size * CHAR_HEIGHT);
	frame.pos.x = x - (UBA_GFX_BUTTON_FRAME_OFFSET / 2);
	frame.pos.y = y - (UBA_GFX_BUTTON_FRAME_OFFSET / 2);
	frame.elemnt.frame.heigth = (uba_gfx->elemnt.button.size * CHAR_HEIGHT) + UBA_GFX_BUTTON_FRAME_OFFSET;
	frame.elemnt.frame.width = (strlen(uba_gfx->elemnt.button.text) * (uba_gfx->elemnt.button.size * CHAR_WIDTH)) + UBA_GFX_BUTTON_FRAME_OFFSET;
	frame.elemnt.frame.color_border = UBA_GFX_COLOR_WHITE;
	frame.elemnt.frame.color_fill = uba_gfx->elemnt.button.color_bg;
	if ((uba_gfx->effect & UBA_GFX_EFFECT_SELECTED) == UBA_GFX_EFFECT_SELECTED) {
		frame.elemnt.frame.color_fill = uba_gfx->elemnt.button.color_text;
		UBA_GFX_draw_frame(&frame);
		ST7789_Draw_Text(x, y, uba_gfx->elemnt.button.text, uba_gfx->elemnt.button.size, uba_gfx->elemnt.button.color_bg,
				uba_gfx->elemnt.button.color_text);
		uba_gfx->state = UBA_GFX_STATE_SELECTED;

	} else if ((uba_gfx->effect & UBA_GFX_EFFECT_BLINK) == UBA_GFX_EFFECT_BLINK) {
		if (uba_gfx->state != UBA_GFX_STATE_BLINK_OFF) {
			frame.elemnt.frame.color_fill = uba_gfx->elemnt.button.color_bg;
			frame.elemnt.frame.color_border = uba_gfx->elemnt.button.color_bg;
			UBA_GFX_draw_frame(&frame);
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.button.text, uba_gfx->elemnt.button.size, uba_gfx->elemnt.button.color_text,
					uba_gfx->elemnt.button.color_bg);
			uba_gfx->state = UBA_GFX_STATE_BLINK_OFF;
		} else {
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.button.text, uba_gfx->elemnt.button.size, uba_gfx->elemnt.button.color_bg,
					uba_gfx->elemnt.button.color_text);
			uba_gfx->state = UBA_GFX_STATE_BLINK_ON;
		}
	} else {
		UBA_GFX_draw_frame(&frame);
		ST7789_Draw_Text(x, y, uba_gfx->elemnt.button.text, uba_gfx->elemnt.button.size, uba_gfx->elemnt.button.color_text,
				uba_gfx->elemnt.button.color_bg);
		uba_gfx->state = UBA_GFX_STATE_VISIBLE;
	}
	return 0;
}

UBA_GFX_ERROR UBA_GFX_draw_status(UBA_GFX *uba_gfx) {
	uint16_t x = uba_gfx->pos.x; // the center of the status circle
	uint16_t y = uba_gfx->pos.y; // the center of the status circle
	uint16_t fill_color;
	uint16_t bg_color;
	if (uba_gfx->id != UBA_GFX_ELEMNET_STATUS) {
		return UBA_GFX_ERROR_ID;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_VISIBLE) == 0) {
		return UBA_GFX_ERROR_NO_ERROR;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_BLINK) == UBA_GFX_EFFECT_BLINK) {
		if (uba_gfx->state == UBA_GFX_STATE_BLINK_ON) {
			fill_color = uba_gfx->elemnt.status.color_bg;
			bg_color = uba_gfx->elemnt.status.color_bg;
			uba_gfx->state = UBA_GFX_STATE_BLINK_OFF;
		} else {
			fill_color = uba_gfx->elemnt.status.color_fill;
			bg_color = uba_gfx->elemnt.status.color_bg;
			uba_gfx->state = UBA_GFX_STATE_BLINK_ON;
		}
	} else {
		fill_color = uba_gfx->elemnt.status.color_fill;
		bg_color = uba_gfx->elemnt.status.color_bg;
		//(x, y, uba_gfx->elemnt.status.raduios + 1, uba_gfx->elemnt.status.color_border);
	}
	//x = x - (((strlen(uba_gfx->elemnt.status.text) - 1) / 2) * CHAR_WIDTH) ; // center the text blow the circle
	ST7789_Draw_Rounded_Rectangle(x - 32, y - 10, 64, 20, 5, fill_color, bg_color);
	y -= 4; //two pixel blow the status
	x -= (((strlen(uba_gfx->elemnt.status.text) * CHAR_WIDTH) / 2) + 1);
	ST7789_Draw_Text(x, y, uba_gfx->elemnt.status.text, 1, uba_gfx->elemnt.status.color_text, fill_color);
	return UBA_GFX_ERROR_NO_ERROR;
}

UBA_GFX_ERROR UBA_GFX_draw_text(UBA_GFX *uba_gfx) {
	uint16_t x = uba_gfx->pos.x; // the center of the status circle
	uint16_t y = uba_gfx->pos.y; // the center of the status circle
//	UART_LOG_INFO(UBA_COMP,"Draw Text At (%u,%u)",x,y);

	if (uba_gfx->id != UBA_GFX_ELEMNET_TEXT) {
		return UBA_GFX_ERROR_ID;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_VISIBLE) == 0) {
		return UBA_GFX_ERROR_NO_ERROR;
	}
	if ((uba_gfx->effect & UBA_GFX_EFFECT_BLINK) == UBA_GFX_EFFECT_BLINK) {
		if (uba_gfx->state == UBA_GFX_STATE_BLINK_ON) {
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.text.text, uba_gfx->elemnt.text.size, uba_gfx->elemnt.text.color_bg,
					uba_gfx->elemnt.text.color_bg);
			uba_gfx->state = UBA_GFX_STATE_BLINK_OFF;
		} else {
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.text.text, uba_gfx->elemnt.text.size, uba_gfx->elemnt.text.color_text,
					uba_gfx->elemnt.text.color_bg);
			uba_gfx->state = UBA_GFX_STATE_BLINK_ON;
		}
	} else if ((uba_gfx->effect & UBA_GFX_EFFECT_SELECTED) == UBA_GFX_EFFECT_SELECTED) {
		if (uba_gfx->state != UBA_GFX_STATE_SELECTED) {
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.text.text, uba_gfx->elemnt.text.size, uba_gfx->elemnt.text.color_bg,
					uba_gfx->elemnt.text.color_text);
			uba_gfx->state = UBA_GFX_STATE_SELECTED;
		} else {
			ST7789_Draw_Text(x, y, uba_gfx->elemnt.text.text, uba_gfx->elemnt.text.size, uba_gfx->elemnt.text.color_bg,
					uba_gfx->elemnt.text.color_text);
		}
	} else {
		ST7789_Draw_Text(x, y, uba_gfx->elemnt.text.text, uba_gfx->elemnt.text.size, uba_gfx->elemnt.text.color_text,
				uba_gfx->elemnt.text.color_bg);
		uba_gfx->state = UBA_GFX_STATE_VISIBLE;
	}
	return UBA_GFX_ERROR_NO_ERROR;
}

UBA_GFX_ERROR UBA_GFX_draw_text_center(UBA_GFX *uba_gfx) {
	uint16_t x = uba_gfx->pos.x; // the center of the text
	uint16_t y = uba_gfx->pos.y; // the center of  the text
	uba_gfx->pos.x -= (((strlen(uba_gfx->elemnt.text.text) * uba_gfx->elemnt.text.size) * CHAR_WIDTH) / 2);
	UBA_GFX_draw_text(uba_gfx);
	uba_gfx->pos.x = x;
	uba_gfx->pos.y = y;
	return UBA_GFX_ERROR_NO_ERROR;
}
