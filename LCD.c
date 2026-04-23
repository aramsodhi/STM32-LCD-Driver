/*
 * LCD.c
 *
 * Simple 16x2 LCD driver for 4-bit operation mode.
 * This implementation assumes a few things:
 * - R/W is tied to ground (write only)
 * - GPIO pins are already configured as outputs
 * - The display is used in standard character mode
 *
 *  Created on: Apr 21, 2026
 *      Author: aramsodhi
 */

#include "LCD.h"

static LCD_Config lcd;

/*
 * Common LCD command bytes used by this driver
 * */
#define LCD_CMD_FUNCTION_SET		0x28
#define LCD_CMD_ENTRY_MODE			0x06
#define LCD_CMD_CLEAR_DISPLAY		0x01
#define LCD_CMD_RETURN_HOME			0x02
#define LCD_CMD_DISPLAY_BLINK_ON	0x0F
#define LCD_CMD_DISPLAY_BLINK_OFF	0x0E


/*
 * Writes one GPIO pin using the STM32 BSRR reg.
 * The lower 16 bits set a pin, the upper 16 bits clear a pin
 * SET: state = 1
 * CLEAR: state = 0
 */
static void LCD_WritePin(const LCD_Pin *pin, uint8_t state) {
	if (state) {
		pin->port->BSRR = pin->pinMask;
	} else {
		pin->port->BSRR = ((uint32_t)pin->pinMask << 16);
	}
}


/*
 * Generate the enable pulse that tells the LCD to latch the current data
 * */
static void LCD_PulseEnable(void) {
	LCD_WritePin(&lcd.EN, 1); // enable high
	HAL_Delay(1);
	LCD_WritePin(&lcd.EN, 0); // enable low
	HAL_Delay(1);
}


/*
 * Writes one 4-bit nibble to the LCD data pins D4-D7
 * */
static void LCD_WriteNibble(uint8_t nibble) {
	LCD_WritePin(&lcd.D4, (nibble & 0x01) ? 1 : 0);
	LCD_WritePin(&lcd.D5, (nibble & 0x02) ? 1 : 0);
	LCD_WritePin(&lcd.D6, (nibble & 0x04) ? 1 : 0);
	LCD_WritePin(&lcd.D7, (nibble & 0x08) ? 1 : 0);

	LCD_PulseEnable();
}


/*
 * Sends one byte in two 4-bit/nibble transfers
 * rs = 0 (command)
 * rs = 1 (char data)
 * */
static void LCD_SendByte(uint8_t value, uint8_t rs) {
	LCD_WritePin(&lcd.RS, rs);

	LCD_WriteNibble(value >> 4); 	// write upper nibble
	LCD_WriteNibble(value & 0x0F); 	// write lower nibble

	HAL_Delay(2); // small delay
}


/*
 * Sends a command byte to the LCD
 * Just a little more readable
 * */
void LCD_SendCommand(uint8_t command) {
	LCD_SendByte(command, 0);
}


void LCD_Init(const LCD_Config *config) {
	lcd = *config;

	HAL_Delay(50); // wait for LCD to wake up/stabilize

	LCD_WritePin(&lcd.RS, 0); // reg select low
	LCD_WritePin(&lcd.EN, 0); // enable low


	/*
	 * Startup sequence from the Hitachi HD44780 LCD datasheet
	 * The LCD may power up expecting 8-bit transfers, so we first send
	 * a few setup nibbles to force it into a known 4-bit state.
	 * After that, normal LCD commands configure the display.
	 * */
	LCD_WriteNibble(0x03);
	HAL_Delay(5);

	LCD_WriteNibble(0x03);
	HAL_Delay(1);

	LCD_WriteNibble(0x03);
	HAL_Delay(1);

	LCD_WriteNibble(0x02);

	LCD_SendCommand(LCD_CMD_FUNCTION_SET);
	LCD_SendCommand(LCD_CMD_DISPLAY_BLINK_ON);
	LCD_SendCommand(LCD_CMD_ENTRY_MODE);
	LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);

	HAL_Delay(2);
}


void LCD_Clear(void) {
	LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);
	HAL_Delay(2);
}


/*
 * Moves the cursor to a row/column position.
 * On a standard 16x2 LCD, line 1 starts at DDRAM address 0x00
 * and line 2 starts at DDRAM address 0x40.
 * The command 0x80 | address tells the LCD where the next
 * character will be written.
 */
void LCD_SetCursor(uint8_t row, uint8_t col) {
	uint8_t address;
	if (row == 0) {
		address = col;
	} else {
		address = 0x40 + col;
	}

	/*
	 * 0x80 is the command prefix for "set DDRAM address".
	 * OR-ing with the address builds the full command byte.
	 */
	LCD_SendCommand(0x80 | address);
}


void LCD_WriteChar(char c) {
	/*
	 * LCD_SendByte expects an unsigned 8-bit value
	 * The cast makes it explicit that we are sending the
	 * raw character code stored in c.
	 * */
	LCD_SendByte((uint8_t)c, 1);
}


void LCD_WriteString(const char *str) {
	while (*str != '\0') {
		LCD_WriteChar(*str);
		str++;
	}
}

void LCD_Home(void) {
	LCD_SendCommand(LCD_CMD_RETURN_HOME);
	HAL_Delay(2);
}

void LCD_CursorBlinkOn(void) {
	LCD_SendCommand(LCD_CMD_DISPLAY_BLINK_ON);
}

void LCD_CursorBlinkOff(void) {
	LCD_SendCommand(LCD_CMD_DISPLAY_BLINK_OFF);
}
