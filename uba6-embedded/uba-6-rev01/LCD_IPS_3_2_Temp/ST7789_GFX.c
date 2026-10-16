/*
 * ST7789_GFX.c
 *
 *  Created on: Apr 10, 2024
 *      Author: alexk
 */

//	MIT License
//
//	Copyright (c) 2017 Matej Artnak
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy
//	of this software and associated documentation files (the "Software"), to deal
//	in the Software without restriction, including without limitation the rights
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//	copies of the Software, and to permit persons to whom the Software is
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in all
//	copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//	SOFTWARE.
//
//
//
//-----------------------------------
//	ILI9341 GFX library for STM32
//-----------------------------------
//
//	Very simple GFX library built upon ILI9342_STM32_Driver library.
//	Adds basic shapes, image and font drawing capabilities to ILI9341
//
//	Library is written for STM32 HAL library and supports STM32CUBEMX. To use the library with Cube software
//	you need to tick the box that generates peripheral initialization code in their own respective .c and .h file
//
//
//-----------------------------------
//	How to use this library
//-----------------------------------
//
//	-If using MCUs other than STM32F7 you will have to change the #include "stm32f7xx_hal.h" in the ILI9341_GFX.h to your respective .h file
//
//	If using "ILI9341_STM32_Driver" then all other prequisites to use the library have allready been met
//	Simply include the library and it is ready to be used
//
//-----------------------------------
#include "ST7789_GFX.h"
#include "ST7789_STM32_Driver.h"
#include "5x5_font.h"
#include "5x7_font.h"
#include "spi.h"
#include "stdlib.h"
#include "uart_log.h"
#include "math.h"

#define PI 3.14159

typedef enum {
	Octant0 = 0x01,
	Octant1 = 0x02,
	Octant2 = 0x04,
	Octant3 = 0x08,
	Octant4 = 0x10,
	Octant5 = 0x20,
	Octant6 = 0x40,
	Octant7 = 0x80,
	Q0 = Octant0|Octant1,
	Q1 = Octant2|Octant3,
	Q2 = Octant4|Octant5,
	Q3 = Octant6|Octant7,
	UPPER_HALF = Q0|Q1,
	LOWER_HALF = Q2|Q3,

} Octant;
/*Draw hollow circle at X,Y location with specified radius and colour. X and Y represent circles center
 * (x, y) 	→ Octant 0
 * (-x, y) 	→ Octant 3
 * (x, -y) 	→ Octant 7
 * (-x, -y)	→ Octant 4
 * (y, x) 	→ Octant 1
 * (-y, x) 	→ Octant 2
 * (y, -x) 	→ Octant 6
 * (-y, -x) → Octant 5
 *        |                Octant 1         | Octant 0
       |                       \        /
       |                        \      /
   -y  |                         \    /
       |                          \  /
-------+---------------------------(0,0)------------------------- +x
       |                          /  \
       |                         /    \
   +y  |                        /      \
       |                Octant 2         | Octant 7
       |
 *
 */
void ST7789_Draw_Hollow_Octant_Circle(uint16_t X, uint16_t Y, uint16_t Radius, Octant o, uint16_t Colour)
{
	int x = Radius;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (Radius << 1);

	while (x >= y)
	{
		if (o & Octant7)
			ST7789_Draw_Pixel(X + x, Y + y, Colour); // Octant 7
		if (o & Octant6)
			ST7789_Draw_Pixel(X + y, Y + x, Colour); // Octant 6
		if (o & Octant5)
			ST7789_Draw_Pixel(X - y, Y + x, Colour); // Octant 5
		if (o & Octant4)
			ST7789_Draw_Pixel(X - x, Y + y, Colour); // Octant 4
		if (o & Octant3)
			ST7789_Draw_Pixel(X - x, Y - y, Colour); // Octant 3
		if (o & Octant2)
			ST7789_Draw_Pixel(X - y, Y - x, Colour); // Octant 2
		if (o & Octant1)
			ST7789_Draw_Pixel(X + y, Y - x, Colour); // Octant 6
		if (o & Octant0)
			ST7789_Draw_Pixel(X + x, Y - y, Colour); // Octant 0

		if (err <= 0)
				{
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0)
				{
			x--;
			dx += 2;
			err += (-Radius << 1) + dx;
		}
	}
}
/*Draw hollow circle at X,Y location with specified radius and colour. X and Y represent circles center */
void ST7789_Draw_Hollow_Circle(uint16_t X, uint16_t Y, uint16_t Radius, uint16_t Colour)
{
	int x = Radius;
	int y = 0;
	int dx = 1;
	int dy = 1;
	int err = dx - (Radius << 1);

	while (x >= y)
	{

		ST7789_Draw_Pixel(X + x, Y + y, Colour); // Octant 1
		ST7789_Draw_Pixel(X - x, Y + y, Colour); // Octant 2
		ST7789_Draw_Pixel(X + x, Y - y, Colour); // Octant 3
		ST7789_Draw_Pixel(X - x, Y - y, Colour); // Octant 4
		ST7789_Draw_Pixel(X + y, Y + x, Colour); // Octant 5
		ST7789_Draw_Pixel(X - y, Y + x, Colour); // Octant 6
		ST7789_Draw_Pixel(X + y, Y - x, Colour); // Octant 7
		ST7789_Draw_Pixel(X - y, Y - x, Colour); // Octant 8

		if (err <= 0)
				{
			y++;
			err += dy;
			dy += 2;
		}
		if (err > 0)
				{
			x--;
			dx += 2;
			err += (-Radius << 1) + dx;
		}
	}
}

void ST7789_Draw_Filled_Ellipse(uint16_t x_center, uint16_t y_center, uint16_t a, uint16_t b, uint16_t Colour, uint16_t Background_Colour) {
	int x, y;
	long a2 = a * a;  // a^2
	long b2 = b * b;  // b^2
	long two_a2 = 2 * a2;
	long two_b2 = 2 * b2;
	long x_error = b2 * (1 - 2 * a);
	long y_error = a2;
	long ellipse_error = 0;
	long stopping_x = two_b2 * a;
	long stopping_y = 0;
	if (a == b) {
		return ST7789_Draw_Filled_Circle(x_center, y_center, a, Colour, Background_Colour);
	}

	// First region of the ellipse (0 to 45 degrees)
	x = a;
	y = 0;
	while (stopping_x >= stopping_y) {
		// Draw horizontal lines from x_center - x to x_center + x for each y
		ST7789_DrawLine(x_center - x, y_center + y, x_center + x, y_center + y, Colour);
		ST7789_DrawLine(x_center - x, y_center - y, x_center + x, y_center - y, Colour);
		ST7789_Draw_Pixel(x_center + x, y_center + y, BLACK);
		ST7789_Draw_Pixel(x_center - x, y_center + y, BLACK);
		ST7789_Draw_Pixel(x_center + x, y_center - y, BLACK);
		ST7789_Draw_Pixel(x_center - x, y_center - y, BLACK);

		y++;
		stopping_y += two_a2;
		ellipse_error += y_error;
		y_error += two_a2;

		if ((2 * ellipse_error + x_error) > 0) {
			x--;
			stopping_x -= two_b2;
			ellipse_error += x_error;
			x_error += two_b2;
		}
	}

	// Second region of the ellipse (45 to 90 degrees)
	x = 0;
	y = b;
	x_error = b2;
	y_error = a2 * (1 - 2 * b);
	ellipse_error = 0;
	stopping_x = 0;
	stopping_y = two_a2 * b;

	while (stopping_x <= stopping_y) {
		// Draw horizontal lines from x_center - x to x_center + x for each y
		ST7789_DrawLine(x_center - x, y_center + y, x_center + x, y_center + y, Colour);
		ST7789_DrawLine(x_center - x, y_center - y, x_center + x, y_center - y, Colour);
		ST7789_Draw_Pixel(x_center + x, y_center + y, BLACK);
		ST7789_Draw_Pixel(x_center - x, y_center + y, BLACK);
		ST7789_Draw_Pixel(x_center + x, y_center - y, BLACK);
		ST7789_Draw_Pixel(x_center - x, y_center - y, BLACK);

		x++;
		stopping_x += two_b2;
		ellipse_error += x_error;
		x_error += two_b2;

		if ((2 * ellipse_error + y_error) > 0) {
			y--;
			stopping_y -= two_a2;
			ellipse_error += y_error;
			y_error += two_a2;
		}
	}

}

/*Draw filled circle at X,Y location with specified radius and colour. X and Y represent circles center */

void ST7789_Draw_Filled_Circle(uint16_t centerX, uint16_t centerY, uint16_t Radius, uint16_t Colour, uint16_t Background_Colour)
{
	int x = Radius;
	int y = 0;
	int decisionOver2 = 1 - x; // Decision variable
	int j, k;
	int arr_size = (2 * (Radius)) + 1;
	uint16_t (*p_arr)[arr_size] = calloc((arr_size), sizeof(*p_arr));
	for (j = 0; j < arr_size; j++) {
		for (k = 0; k < arr_size; k++) {
			p_arr[j][k] = Background_Colour;
		}
	}

	while (x >= y) {
		// Draw horizontal lines (scanline fill)
		for (int i = centerX - x; i <= centerX + x; i++) {
//			ST7789_Draw_Pixel(i, centerY + y, Colour); // Lower half
//			ST7789_Draw_Pixel(i, centerY - y, Colour); // Upper half
			j = ((i + Radius) - centerX);
			if (j < 0) {
				UART_LOG_CRITICAL("c", "j = %d", j);
			}
			k = Radius + y;
			p_arr[j][k] = Colour;
			k = Radius - y;
			p_arr[j][k] = Colour;
		}

		for (int i = centerX - y; i <= centerX + y; i++) {
//			ST7789_Draw_Pixel(i, centerY + x, Colour); // Right half
//			ST7789_Draw_Pixel(i, centerY - x, Colour); // Left half
			j = ((i + Radius) - centerX);
			if (j < 0) {
				UART_LOG_CRITICAL("c", "j = %d", j);
			}
			k = Radius + x;
			p_arr[j][k] = Colour;
			k = Radius - x;
			p_arr[j][k] = Colour;
		}

		y++;
		if (decisionOver2 <= 0) {
			decisionOver2 += 2 * y + 1;
		} else {
			x--;
			decisionOver2 += 2 * (y - x) + 1;
		}
	}
	ST7789_Draw_Array(centerX - Radius, centerY - Radius, arr_size, arr_size, &p_arr[0][0], arr_size * arr_size);
	ST7789_Draw_Hollow_Circle(centerX, centerY, Radius, BLACK);
	free(p_arr);
}
void ST7789_Draw_Filled_Quarter_Circle(uint16_t centerX, uint16_t centerY, uint16_t Radius, uint8_t Q, uint16_t Colour, uint16_t Background_Colour) {
	int x = Radius;
	int y = 0;
	int decisionOver2 = 1 - x; // Decision variable
	int j, k;
	int arr_size = (2 * (Radius)) + 1;
	int q_arr_size = (Radius + 1);
	uint16_t (*p_arr)[arr_size] = calloc((arr_size), sizeof(*p_arr));
	uint16_t (*p_q_arr)[q_arr_size] = calloc((q_arr_size), sizeof(*p_q_arr));
	for (j = 0; j < arr_size; j++) {
		for (k = 0; k < arr_size; k++) {
			p_arr[j][k] = Background_Colour;
		}
	}
	while (x >= y) {
		// Draw horizontal lines (scanline fill)
		for (int i = centerX - x; i <= centerX + x; i++) {
			j = ((i + Radius) - centerX);
			if (j < 0) {
				UART_LOG_CRITICAL("c", "j = %d", j);
			}
			k = Radius + y;
			p_arr[j][k] = Colour;
			k = Radius - y;
			p_arr[j][k] = Colour;
		}

		for (int i = centerX - y; i <= centerX + y; i++) {
			j = ((i + Radius) - centerX);
			if (j < 0) {
				UART_LOG_CRITICAL("c", "j = %d", j);
			}
			k = Radius + x;
			p_arr[j][k] = Colour;
			k = Radius - x;
			p_arr[j][k] = Colour;
		}

		y++;
		if (decisionOver2 <= 0) {
			decisionOver2 += 2 * y + 1;
		} else {
			x--;
			decisionOver2 += 2 * (y - x) + 1;
		}
	}
	switch (Q) {
		case 0:
			for (j = 0; j < Radius + 1; j++) {
				for (k = 0; k < Radius + 1; k++) {
					p_q_arr[j][k] = p_arr[j][k+ Radius];
				}
			}
			ST7789_Draw_Array(centerX , centerY - Radius, q_arr_size, q_arr_size, &p_q_arr[0][0], q_arr_size * q_arr_size);
			ST7789_Draw_Hollow_Octant_Circle(centerX, centerY ,Radius,Q0,BLACK);
			break;
		case 1:
			for (j = 0; j < q_arr_size; j++) {
				for (k = 0; k < q_arr_size; k++) {
					p_q_arr[j][k] = p_arr[j][k ];
				}
			}
			ST7789_Draw_Array(centerX- Radius, centerY - Radius, q_arr_size, q_arr_size, &p_q_arr[0][0], q_arr_size * q_arr_size);
			ST7789_Draw_Hollow_Octant_Circle(centerX , centerY,Radius,Q1,BLACK);
			break;
		case 2:
			for (j = 0; j < Radius + 1; j++) {
				for (k = 0; k < Radius + 1; k++) {
					p_q_arr[j][k] = p_arr[j + Radius][k];
				}
			}
			ST7789_Draw_Array(centerX - Radius, centerY, q_arr_size, q_arr_size, &p_q_arr[0][0], q_arr_size * q_arr_size);
			ST7789_Draw_Hollow_Octant_Circle(centerX, centerY ,Radius,Q2,BLACK);
			break;
		case 3:
			for (j = 0; j < Radius + 1; j++) {
				for (k = 0; k < Radius + 1; k++) {
					p_q_arr[j][k] = p_arr[j + Radius][k + Radius];
				}
			}
			ST7789_Draw_Array(centerX, centerY, q_arr_size, q_arr_size, &p_q_arr[0][0], q_arr_size * q_arr_size);
			ST7789_Draw_Hollow_Octant_Circle(centerX , centerY ,Radius,Q3,BLACK);
			break;
	}
 	/*for(int o=1;o<0x100;o=o<<1){
		ST7789_Draw_Hollow_Octant_Circle(centerX, centerY, Radius,o, BLUE);
	}*/
	free(p_arr);
	free(p_q_arr);

}

/*Draw a hollow rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void ST7789_Draw_Hollow_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t Colour)
{
	uint16_t X_length = 0;
	uint16_t Y_length = 0;
	uint8_t Negative_X = 0;
	uint8_t Negative_Y = 0;
	float Calc_Negative = 0;

	Calc_Negative = X1 - X0;
	if (Calc_Negative < 0)
		Negative_X = 1;
	Calc_Negative = 0;

	Calc_Negative = Y1 - Y0;
	if (Calc_Negative < 0)
		Negative_Y = 1;

	//DRAW HORIZONTAL!
	if (!Negative_X)
	{
		X_length = X1 - X0;
	}
	else
	{
		X_length = X0 - X1;
	}
	ST7789_Draw_Horizontal_Line(X0, Y0, X_length, Colour);
	ST7789_Draw_Horizontal_Line(X0, Y1, X_length, Colour);

	//DRAW VERTICAL!
	if (!Negative_Y)
	{
		Y_length = Y1 - Y0;
	}
	else
	{
		Y_length = Y0 - Y1;
	}
	ST7789_Draw_Vertical_Line(X0, Y0, Y_length, Colour);
	ST7789_Draw_Vertical_Line(X1, Y0, Y_length, Colour);

	if ((X_length > 0) || (Y_length > 0))
			{
		ST7789_Draw_Pixel(X1, Y1, Colour);
	}

}

/*Draw a filled rectangle between positions X0,Y0 and X1,Y1 with specified colour*/
void ST7789_Draw_Filled_Rectangle_Coord(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1, uint16_t Colour)
{
	uint16_t X_length = 0;
	uint16_t Y_length = 0;
	uint8_t Negative_X = 0;
	uint8_t Negative_Y = 0;
	int32_t Calc_Negative = 0;

	uint16_t X0_true = 0;
	uint16_t Y0_true = 0;

	Calc_Negative = X1 - X0;
	if (Calc_Negative < 0)
		Negative_X = 1;
	Calc_Negative = 0;

	Calc_Negative = Y1 - Y0;
	if (Calc_Negative < 0)
		Negative_Y = 1;

	//DRAW HORIZONTAL!
	if (!Negative_X)
	{
		X_length = X1 - X0;
		X0_true = X0;
	}
	else
	{
		X_length = X0 - X1;
		X0_true = X1;
	}

	//DRAW VERTICAL!
	if (!Negative_Y)
	{
		Y_length = Y1 - Y0;
		Y0_true = Y0;
	}
	else
	{
		Y_length = Y0 - Y1;
		Y0_true = Y1;
	}

	ST7789_Draw_Rectangle(X0_true, Y0_true, X_length, Y_length, Colour);
}

/*Draws a character (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void ST7789_Draw_Char(char Character, uint16_t X, uint16_t Y, uint16_t Colour, uint16_t Size, uint16_t Background_Colour)
{
	uint8_t function_char;
	uint8_t i, j, k, l;
	uint16_t (*p_arr)[CHAR_WIDTH * Size] = calloc((CHAR_HEIGHT * Size), sizeof(*p_arr));
	uint16_t buf1[CHAR_HEIGHT][CHAR_WIDTH] = { 0 };
	uint16_t buf[CHAR_HEIGHT * 2][CHAR_WIDTH * 2] = { 0 };
	UNUSED(buf);
	UNUSED(buf1);
	function_char = Character;

	if (function_char < ' ') {
		Character = 0;
	} else {
//		function_char -= 32;
	}

	char temp[CHAR_WIDTH];
	for (uint8_t k = 0; k < CHAR_WIDTH; k++)
			{
		temp[k] = font5x7[function_char][k];
	}
	//TODO: Create a function that TX an array, create the char as array
	// Draw pixels
	//ST7789_Draw_Rectangle(X, Y, CHAR_WIDTH * Size, CHAR_HEIGHT * Size, Background_Colour);
	for (j = 0; j < CHAR_WIDTH; j++) {
		for (i = 0; i < CHAR_HEIGHT; i++) {
			if (temp[j] & (1 << i)) {
				if (Size == 1) {
					//ST7789_Draw_Pixel(X + j, Y + i, Colour);
					p_arr[i][j] = Colour;
				} else {
					for (k = 0; k < Size; k++) {
						for (l = 0; l < Size; l++) {
							p_arr[(i * Size) + k][(j * Size) + l] = Colour;
						}
					}
					if (Size == 2) {
						buf[(i * Size)][(j * Size)] = Colour;
						buf[(i * Size) + 1][(j * Size)] = Colour;
						buf[(i * Size)][(j * Size) + 1] = Colour;
						buf[(i * Size) + 1][(j * Size) + 1] = Colour;
					} else {
						ST7789_Draw_Rectangle(X + (j * Size), Y + (i * Size), Size, Size, Colour);
					}
				}
			} else {
				if (Size == 1) {
					//ST7789_Draw_Pixel(X + j, Y + i, Background_Colour);
					buf1[i][j] = Background_Colour;
					p_arr[i][j] = Background_Colour;
				} else {
					for (k = 0; k < Size; k++) {
						for (l = 0; l < Size; l++) {
							p_arr[(i * Size) + k][(j * Size) + l] = Background_Colour;
						}
					}
					if (Size == 2) {
						buf[i * Size][j * Size] = Background_Colour;
						buf[i * Size + 1][j * Size] = Background_Colour;
						buf[i * Size][j * Size + 1] = Background_Colour;
						buf[i * Size + 1][j * Size + 1] = Background_Colour;

					} else {

						ST7789_Draw_Rectangle(X + (j * Size), Y + (i * Size), Size, Size, Background_Colour);
					}
				}
			}
		}
	}
	ST7789_Draw_Array(X, Y, CHAR_WIDTH * Size, CHAR_HEIGHT * Size, &p_arr[0][0], (CHAR_WIDTH * Size) * (CHAR_HEIGHT * Size));
	free(p_arr);
	/*if (Size == 1)
	 ST7789_Draw_Array(X, Y, CHAR_WIDTH, CHAR_HEIGHT, &buf1[0][0], (CHAR_WIDTH) * (CHAR_HEIGHT));
	 if (Size == 2)
	 ST7789_Draw_Array(X, Y, CHAR_WIDTH * Size, CHAR_HEIGHT * Size, &buf[0][0], (CHAR_WIDTH * Size) * (CHAR_HEIGHT * Size));*/
}

/*Draws an array of characters (fonts imported from fonts.h) at X,Y location with specified font colour, size and Background colour*/
/*See fonts.h implementation of font on what is required for changing to a different font when switching fonts libraries*/
void ST7789_Draw_Text(uint16_t X, uint16_t Y, const char *Text, uint16_t Size, uint16_t Colour, uint16_t Background_Colour)
{
	while (*Text) {
		ST7789_Draw_Char(*Text++, X, Y, Colour, Size, Background_Colour);
		X += CHAR_WIDTH * Size;
	}
}

/*Draws a full screen picture from flash. Image converted from RGB .jpeg/other to C array using online converter*/
//USING CONVERTER: http://www.digole.com/tools/PicturetoC_Hex_converter.php
//65K colour (2Bytes / Pixel)
void ST7789_Draw_Image(const char *Image_Array, uint8_t Orientation)
{
	if (Orientation == SCREEN_HORIZONTAL_1)
	{
		ST7789_Set_Rotation(SCREEN_HORIZONTAL_1);
		ST7789_Set_Address(0, 0, ST7789_SCREEN_WIDTH, ST7789_SCREEN_HEIGHT);

		HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for (uint32_t i = 0; i < ST7789_SCREEN_WIDTH * ST7789_SCREEN_HEIGHT * 2 / BURST_MAX_SIZE; i++)
				{
			for (uint32_t k = 0; k < BURST_MAX_SIZE; k++)
					{
				Temp_small_buffer[k] = Image_Array[counter + k];
			}
			HAL_SPI_Transmit(&hspi2, (unsigned char*) Temp_small_buffer, BURST_MAX_SIZE, 10);
			counter += BURST_MAX_SIZE;
		}
	}
	else if (Orientation == SCREEN_HORIZONTAL_2)
	{
		ST7789_Set_Rotation(SCREEN_HORIZONTAL_2);
		ST7789_Set_Address(0, 0, ST7789_SCREEN_WIDTH, ST7789_SCREEN_HEIGHT);

		HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for (uint32_t i = 0; i < ST7789_SCREEN_WIDTH * ST7789_SCREEN_HEIGHT * 2 / BURST_MAX_SIZE; i++)
				{
			for (uint32_t k = 0; k < BURST_MAX_SIZE; k++)
					{
				Temp_small_buffer[k] = Image_Array[counter + k];
			}
			HAL_SPI_Transmit(&hspi2, (unsigned char*) Temp_small_buffer, BURST_MAX_SIZE, 10);
			counter += BURST_MAX_SIZE;
		}
	}
	else if (Orientation == SCREEN_VERTICAL_2)
	{
		ST7789_Set_Rotation(SCREEN_VERTICAL_2);
		ST7789_Set_Address(0, 0, ST7789_SCREEN_HEIGHT, ST7789_SCREEN_WIDTH);

		HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for (uint32_t i = 0; i < ST7789_SCREEN_WIDTH * ST7789_SCREEN_HEIGHT * 2 / BURST_MAX_SIZE; i++)
				{
			for (uint32_t k = 0; k < BURST_MAX_SIZE; k++)
					{
				Temp_small_buffer[k] = Image_Array[counter + k];
			}
			HAL_SPI_Transmit(&hspi2, (unsigned char*) Temp_small_buffer, BURST_MAX_SIZE, 10);
			counter += BURST_MAX_SIZE;
		}
	}
	else if (Orientation == SCREEN_VERTICAL_1)
	{
		ST7789_Set_Rotation(SCREEN_VERTICAL_1);
		ST7789_Set_Address(0, 0, ST7789_SCREEN_HEIGHT, ST7789_SCREEN_WIDTH);

		HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

		unsigned char Temp_small_buffer[BURST_MAX_SIZE];
		uint32_t counter = 0;
		for (uint32_t i = 0; i < ST7789_SCREEN_WIDTH * ST7789_SCREEN_HEIGHT * 2 / BURST_MAX_SIZE; i++)
				{
			for (uint32_t k = 0; k < BURST_MAX_SIZE; k++)
					{
				Temp_small_buffer[k] = Image_Array[counter + k];
			}
			HAL_SPI_Transmit(&hspi2, (unsigned char*) Temp_small_buffer, BURST_MAX_SIZE, 10);
			counter += BURST_MAX_SIZE;
		}
	}
}
/**
 * @brief Draw a Triangle with single color
 * @param  xi&yi -> 3 coordinates of 3 top points.
 * @param color ->color of the lines
 * @return  none
 */
void ST7789_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
	/* Draw lines */
	ST7789_DrawLine(x1, y1, x2, y2, color);
	ST7789_DrawLine(x2, y2, x3, y3, color);
	ST7789_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief Draw a filled Triangle with single color
 * @param  xi&yi -> 3 coordinates of 3 top points.
 * @param color ->color of the triangle
 * @return  none
 */
void ST7789_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{

	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
			yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
			curpixel = 0;

	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);
	x = x1;
	y = y1;

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	}
	else {
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	}
	else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;
		yinc2 = 0;
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;
	}
	else {
		xinc2 = 0;
		yinc1 = 0;
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		ST7789_DrawLine(x, y, x3, y3, color);

		num += numadd;
		if (num >= den) {
			num -= den;
			x += xinc1;
			y += yinc1;
		}
		x += xinc2;
		y += yinc2;
	}
}

// Function to draw a curved line (arc) from point A to B with a specified radius
void drawCurvedLine(int x1, int y1, int x2, int y2, float radius, uint16_t color) {
	// Midpoint between A and B
	float midX = (x1 + x2) / 2.0;
	float midY = (y1 + y2) / 2.0;

	// Distance between points A and B
	float d = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

	// Check if the radius is valid (it must be larger than half the distance between A and B)
	if (radius < d / 2) {
		// Invalid radius, radius must be larger than d/2
		return;
	}

	// Find the perpendicular direction to the line AB
	float dx = (x2 - x1) / d;
	float dy = (y2 - y1) / d;

	// Perpendicular vector (normalized)
	float perpX = -dy;
	float perpY = dx;

	// Distance from the midpoint to the center of the circle (using Pythagoras)
	float h = sqrt(radius * radius - (d / 2) * (d / 2));

	// Calculate the two possible centers of the circle (above or below the line)
	float centerX1 = midX + h * perpX;
	float centerY1 = midY + h * perpY;
	float centerX2 = midX - h * perpX;
	float centerY2 = midY - h * perpY;

	// Choose one of the two centers (based on your preference for which direction the arc should curve)
	float centerX = centerX1;
	float centerY = centerY1;

	// Calculate angles from the center to A and B
	float startAngle = atan2(y1 - centerY, x1 - centerX);
	float endAngle = atan2(y2 - centerY, x2 - centerX);

	// If the arc crosses more than 180 degrees, adjust the angles
	if (endAngle < startAngle) {
		endAngle += 2 * PI;
	}

	// Step through the angles and draw the arc
	for (float theta = startAngle; theta <= endAngle; theta += 0.01) {  // Step through the arc
		int x = (int) (centerX + radius * cos(theta));
		int y = (int) (centerY + radius * sin(theta));
		ST7789_Draw_Pixel(x, y, color);  // Draw the pixel on the TFT
	}
}

void ST7789_Draw_Rounded_Rectangle(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t R, uint16_t Colour, uint16_t Background_Colour) {
	// @formatter:off

	ST7789_Draw_Filled_Quarter_Circle(X + W - R-1	, Y + R			, R, 0,Colour, Background_Colour);
	ST7789_Draw_Filled_Quarter_Circle(X + R			, Y + R			, R, 1,Colour, Background_Colour);
	ST7789_Draw_Filled_Quarter_Circle(X + R			, Y + H - R-1	, R, 2,Colour, Background_Colour);
	ST7789_Draw_Filled_Quarter_Circle(X + W - R	-1	, Y + H - R	-1	, R, 3,Colour, Background_Colour);
												// @formatter:on
	ST7789_Draw_Rectangle(X + R, Y, (W - (R * 2)), H, Colour);
	ST7789_Draw_Horizontal_Line(X + R, Y,(W - (R * 2)),BLACK);
	ST7789_Draw_Horizontal_Line(X + R, Y+H-1,(W - (R * 2)),BLACK);
	ST7789_Draw_Rectangle(X, Y + R, W, H - (2 * R), Colour);
	ST7789_Draw_Vertical_Line(X, Y+ R, H - (2 * R), BLACK);
	ST7789_Draw_Vertical_Line(X+W-1, Y+ R, H - (2 * R), BLACK);
}

