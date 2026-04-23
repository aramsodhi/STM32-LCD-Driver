/*
 * LCD.h
 *
 *  Created on: Apr 21, 2026
 *      Author: aramsodhi
 */

#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "stm32l4xx_hal.h"


/*
 * Describes one GPIO pin used by the LCD (which bank and pin #)
 * */
typedef struct {
	GPIO_TypeDef *port;
	uint16_t pinMask;
} LCD_Pin;


/*
 * Groups all LCD control and data pins.
 * D4-D7 must match the LCD's DB4-DB7 pins
 * */
typedef struct {
	LCD_Pin RS;
	LCD_Pin EN;
	LCD_Pin D4;
	LCD_Pin D5;
	LCD_Pin D6;
	LCD_Pin D7;
} LCD_Config;


/*
 * Initializes the LCD using the provided pin mapping.
 * This function assumes the GPIO pins are already configured as outputs
 * */
void LCD_Init(const LCD_Config *config);

/*
 * Sends the clear display command.
 * */
void LCD_Clear(void);

/*
 * Moves the cursor to the given row and column
 * For a 16x2 display (like we are using), valid rows are 0-1
 * */
void LCD_SetCursor(uint8_t row, uint8_t col);

/*
 * Writes one character at the current cursor position.
 * */
void LCD_WriteChar(char c);


/*
 * Sends a single command byte to the LCD.
 * */
void LCD_SendCommand(uint8_t command);

/*
 * Write a null-terminated string at the current cursor position.
 * */
void LCD_WriteString(const char *str);

/*
 * Returns the cursor to the home position.
 * This sets the cursor back to the top-left corner.
 */
void LCD_Home(void);


/*
 * Turns cursor blinking on.
 */
void LCD_CursorBlinkOn(void);

/*
 * Turns cursor blinking off.
 */
void LCD_CursorBlinkOff(void);



#endif /* LCD_H */
