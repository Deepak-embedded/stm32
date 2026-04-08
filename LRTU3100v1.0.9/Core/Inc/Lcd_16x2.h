/*
 * Lcd_16x2.h
 *
 *  Created on: Feb 21, 2024
 *      Author: maulin
 */

#ifndef INC_LCD_16X2_H_
#define INC_LCD_16X2_H_

#define LCD_I2C_ADDRESS 0x3C // change this according to ur setup
// Backlight configuration
#define BACKLIGHT_TIMEOUT_MS    25000   // 25 seconds (25000ms)

// Function prototypes
char lcd_write_command(uint8_t command);
void lcd_write_data(uint8_t data) ;
void lcd_clear();
void lcd_display_string(const char *string);
void lcd_initialize();
void lcd_set_cursor(uint8_t row, uint8_t  col);
void lcd_float_print(float value, int precision);

void Backlight_Init(void);
void Backlight_On(void);
void Backlight_Off(void);
void Backlight_Button_Pressed(void);
void Backlight_Timer_Callback(TimerHandle_t xTimer);
#endif /* INC_LCD_16X2_H_ */
