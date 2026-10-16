/*
 * ST7789_STM32_Driver.c
 *
 *  Created on: Apr 10, 2024
 *      Author: alexk
 */

/* Includes ------------------------------------------------------------------*/
#include "ST7789_STM32_Driver.h"
#include "stm32g4xx_hal.h"
#include "spi.h"
#include "gpio.h"
#include "stdlib.h"

/* Global Variables ------------------------------------------------------------------*/
volatile uint16_t LCD_HEIGHT = ST7789_SCREEN_HEIGHT;
volatile uint16_t LCD_WIDTH = ST7789_SCREEN_WIDTH;
////////////////////////////////////ST7789 LCD Driver Functions  //////////////////////
/* Initialize SPI */
void ILI9341_SPI_Init(void)
{
	MX_SPI2_Init(); //SPI INIT
	MX_GPIO_Init();												//GPIO INIT

}
/*Enable LCD display*/
void ST7789_Enable(void)
{

	HAL_GPIO_WritePin(LCD_Reset_GPIO_Port, LCD_Reset_Pin, GPIO_PIN_SET);
}
/*HARDWARE RESET*/
void ST7789_Reset(void)
{
	HAL_GPIO_WritePin(LCD_Reset_GPIO_Port, LCD_Reset_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_Reset_GPIO_Port, LCD_Reset_Pin, GPIO_PIN_SET);
	HAL_Delay(200);
}
/*Send data (char) to LCD*/
void ST7789_SPI_Send(unsigned char SPI_Data)
{
	HAL_SPI_Transmit(HSPI_INSTANCE, &SPI_Data, 1, 1);
}

/* Send command (char) to LCD */
void ST7789_Write_Command(uint8_t Command)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	ST7789_SPI_Send(Command);
}

/* Send Data (char) to LCD */
void ST7789_Write_Data(uint8_t Data)
{
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	ST7789_SPI_Send(Data);
}

/*Ser rotation of the screen - changes x0 and y0*/
void ST7789_Set_Rotation(uint8_t m)
{
	ST7789_Write_Command(ST7789_MADCTL);	// MADCTL
	switch (m) {
		case 0:
			ST7789_Write_Data(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
			break;
		case 1:
			ST7789_Write_Data(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
			break;
		case 2:
			ST7789_Write_Data(ST7789_MADCTL_RGB);
			break;
		case 3:
			ST7789_Write_Data(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
			break;
		default:
			break;
	}
}

//INTERNAL FUNCTION OF LIBRARY
/*Sends block colour information to LCD*/
void ST7789_Draw_Colour_Burst(uint16_t Colour, uint32_t Size)
{
	//SENDS COLOUR
	uint32_t Buffer_Size = 0;
	if ((Size * 2) < BURST_MAX_SIZE)
	{
		Buffer_Size = Size;
	}
	else
	{
		Buffer_Size = BURST_MAX_SIZE;
	}

	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

	unsigned char chifted = Colour >> 8;
	;
	unsigned char burst_buffer[Buffer_Size];
	for (uint32_t j = 0; j < Buffer_Size; j += 2)
			{
		burst_buffer[j] = chifted;
		burst_buffer[j + 1] = Colour;
	}

	uint32_t Sending_Size = Size * 2;
	uint32_t Sending_in_Block = Sending_Size / Buffer_Size;
	uint32_t Remainder_from_block = Sending_Size % Buffer_Size;

	if (Sending_in_Block != 0)
			{
		for (uint32_t j = 0; j < (Sending_in_Block); j++)
				{
			HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char*) burst_buffer, Buffer_Size, 100);
		}
	}

	//REMAINDER!
	HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char*) burst_buffer, Remainder_from_block, 100);

}
//DRAW PIXEL AT XY POSITION WITH SELECTED COLOUR
//
//Location is dependant on screen orientation. x0 and y0 locations change with orientations.
//Using pixels to draw big simple structures is not recommended as it is really slow
//Try using either rectangles or lines if possible
//
void ST7789_Draw_Pixel(uint16_t X, uint16_t Y, uint16_t Colour)
{
	if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
		return;	//OUT OF BOUNDS!

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	ST7789_SPI_Send(0x2A);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

	//XDATA
	unsigned char Temp_Buffer[4] = { X >> 8, X, (X + 1) >> 8, (X + 1) };
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer, 4, 1);

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	ST7789_SPI_Send(0x2B);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

	//YDATA
	unsigned char Temp_Buffer1[4] = { Y >> 8, Y, (Y + 1) >> 8, (Y + 1) };
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer1, 4, 1);

	//ADDRESS
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_RESET);
	ST7789_SPI_Send(0x2C);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);

	//COLOUR
	unsigned char Temp_Buffer2[2] = { Colour >> 8, Colour };
	HAL_SPI_Transmit(HSPI_INSTANCE, Temp_Buffer2, 2, 1);

}
//DRAW RECTANGLE OF SET SIZE AND HEIGTH AT X and Y POSITION WITH CUSTOM COLOUR
//
//Rectangle is hollow. X and Y positions mark the upper left corner of rectangle
//As with all other draw calls x0 and y0 locations dependant on screen orientation
//

void ST7789_Draw_Rectangle(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t Colour)
{
	if ((X >= LCD_WIDTH) || (Y >= LCD_HEIGHT))
		return;
	if ((X + Width - 1) >= LCD_WIDTH)
			{
		Width = LCD_WIDTH - X;
	}
	if ((Y + Height - 1) >= LCD_HEIGHT)
			{
		Height = LCD_HEIGHT - Y;
	}
	ST7789_Set_Address(X, Y, X + Width - 1, Y + Height - 1);
	ST7789_Draw_Colour_Burst(Colour, Height * Width);
}

void ST7789_Draw_Array(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, uint16_t *ColourArr, uint16_t Size) {
	uint16_t i, t_color;
	ST7789_Set_Address(X, Y, X + Width - 1, Y + Height - 1);
	HAL_GPIO_WritePin(LCD_DC_PORT, LCD_DC_PIN, GPIO_PIN_SET);
	for (i = 0; i < Size; i++) {
		t_color = ColourArr[i];
		ColourArr[i] = (ColourArr[i] << 8) | (t_color >> 8);
	}
	HAL_SPI_Transmit(HSPI_INSTANCE, (unsigned char*) ColourArr, Size * 2, 100);
}

/*Initialize LCD display*/
void ST7789_Init(void)
{
	//MX_SPI2_Init();
	ST7789_Enable();
	HAL_Delay(1);
	ST7789_Reset();
	HAL_Delay(10);
	ST7789_Enable();
	HAL_Delay(120);

	//SOFTWARE RESET
	ST7789_Write_Command(0x3A);
	ST7789_Write_Data(0x55);

	ST7789_Write_Command(0xB2);
	ST7789_Write_Data(0x0C);
	ST7789_Write_Data(0x0C);
	ST7789_Write_Data(0x00);
	ST7789_Write_Data(0x33);
	ST7789_Write_Data(0x33);

	ST7789_Write_Command(0xB7);
	ST7789_Write_Data(0x35);

	ST7789_Write_Command(0xBB);
	ST7789_Write_Data(0x19);

	ST7789_Write_Command(0xC0);
	ST7789_Write_Data(0x2C);

	ST7789_Write_Command(0xC2);
	ST7789_Write_Data(0x01);

	ST7789_Write_Command(0xC3);
	ST7789_Write_Data(0x12);

	ST7789_Write_Command(0xC4);
	ST7789_Write_Data(0x20);

	ST7789_Write_Command(0xC6);
	ST7789_Write_Data(0x0F);

	ST7789_Write_Command(0xD0);
	ST7789_Write_Data(0xA4);
	ST7789_Write_Data(0xA1);

	ST7789_Write_Command(0xE0);
	ST7789_Write_Data(0xD0);
	ST7789_Write_Data(0x04);
	ST7789_Write_Data(0x0D);
	ST7789_Write_Data(0x11);
	ST7789_Write_Data(0x13);
	ST7789_Write_Data(0x2B);
	ST7789_Write_Data(0x3F);
	ST7789_Write_Data(0x54);
	ST7789_Write_Data(0x4C);
	ST7789_Write_Data(0x18);
	ST7789_Write_Data(0x0D);
	ST7789_Write_Data(0x0B);
	ST7789_Write_Data(0x1F);
	ST7789_Write_Data(0x23);

	ST7789_Write_Command(0xE1);
	ST7789_Write_Data(0xF0);
	ST7789_Write_Data(0x07);
	ST7789_Write_Data(0x0A);
	ST7789_Write_Data(0x0D);
	ST7789_Write_Data(0x0B);
	ST7789_Write_Data(0x07);
	ST7789_Write_Data(0x28);
	ST7789_Write_Data(0x33);
	ST7789_Write_Data(0x3E);
	ST7789_Write_Data(0x36);
	ST7789_Write_Data(0x14);
	ST7789_Write_Data(0x14);
	ST7789_Write_Data(0x29);
	ST7789_Write_Data(0x32);

	ST7789_Write_Command(0x21);
	ST7789_Write_Command(0x11);
	HAL_Delay(120);
	ST7789_Write_Command(0x13);

	//TURN ON DISPLAY
	ST7789_Write_Command(0x29);
}

/* Set Address - Location block - to draw into */
void ST7789_Set_Address(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2)
{
	ST7789_Write_Command(0x2A);
	ST7789_Write_Data(X1 >> 8);
	ST7789_Write_Data(X1);
	ST7789_Write_Data(X2 >> 8);
	ST7789_Write_Data(X2);

	ST7789_Write_Command(0x2B);
	ST7789_Write_Data(Y1 >> 8);
	ST7789_Write_Data(Y1);
	ST7789_Write_Data(Y2 >> 8);
	ST7789_Write_Data(Y2);

	ST7789_Write_Command(0x2C);


}

// all display one colour
void ST7789_Fill_Screen(uint16_t bColor)
{
	uint16_t i, j;
	ST7789_Set_Address(0, 0, 319, 239);	//320x240
	for (i = 0; i < 320; i++)
			{
		for (j = 0; j < 240; j++)
				{
			ST7789_Write_Data(bColor >> 8);
			ST7789_Write_Data(bColor);
		}
	}
}

/**
 * @brief Draw a line with single color
 * @param x1&y1 -> coordinate of the start point
 * @param x2&y2 -> coordinate of the end point
 * @param color -> color of the line to Draw
 * @return none
 */
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
		uint16_t color) {
	uint16_t swap;
	uint16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (x0 == x1) {
		if (abs(y1 - y0) > 1) {
			return ST7789_Draw_Vertical_Line(x0, y0 <= y1 ? y0 : y1, abs(y1 - y0), color);
		} else if (abs(y1 - y0) == 1) {
			return ST7789_Draw_Pixel(x0, y0, color);
		} else {
			return;
		}

	} else if (y0 == y1) {
		if (abs(x1 - x0) > 1) {
			return ST7789_Draw_Horizontal_Line(x0 <= x1 ? x0 : x1, y0, abs(x1 - x0), color);
		} else if (abs(x1 - x0) == 1) {
			return ST7789_Draw_Pixel(x0, y0, color);
		} else {
			return;
		}
	}
	if (steep) {
		swap = x0;
		x0 = y0;
		y0 = swap;

		swap = x1;
		x1 = y1;
		y1 = swap;
		//_swap_int16_t(x0, y0);
		//_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		swap = x0;
		x0 = x1;
		x1 = swap;

		swap = y0;
		y0 = y1;
		y1 = swap;
		//_swap_int16_t(x0, x1);
		//_swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			ST7789_Draw_Pixel(y0, x0, color);
		} else {
			ST7789_Draw_Pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void ST7789_Draw_Horizontal_Line(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Colour) {
	uint16_t ColorArr[320];
	uint16_t i = 0;
	for (i = 0; i < 320; i++) {
		ColorArr[i] = Colour;
	}
	ST7789_Draw_Array(X, Y, Width, 1, ColorArr, Width);
}
void ST7789_Draw_Vertical_Line(uint16_t X, uint16_t Y, uint16_t Height, uint16_t Colour) {
	uint16_t ColorArr[320];
	uint16_t i = 0;
	for (i = 0; i < Height; i++) {
		ColorArr[i] = Colour;
	}
	ST7789_Draw_Array(X, Y, 1, Height, ColorArr, Height);

}
