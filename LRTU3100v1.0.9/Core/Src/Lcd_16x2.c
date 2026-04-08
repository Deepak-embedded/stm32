/*
 * Lcd_16x2.c
 *
 *  Created on: Feb 21, 2024
 *      Author: maulin
 */

#include "main.h"
#include "Lcd_16x2.h"

// Global variables
TimerHandle_t backlightTimer = NULL;
uint8_t backlight_state = 0; // 0 = off, 1 = on
char dispState;

//***************************************************** */
/**
 * @brief Initialize backlight control system
 */
void Backlight_Init(void)
{
    // GPIO should already be configured by CubeMX
    // Initialize backlight as OFF
    Backlight_Off();

    // Create backlight auto-off timer
    backlightTimer = xTimerCreate(
        "BacklightTimer",                    // Timer name
        pdMS_TO_TICKS(BACKLIGHT_TIMEOUT_MS), // Timer period (25 seconds)
        pdFALSE,                             // One-shot timer (not periodic)
        NULL,                                // Timer ID (not used)
        Backlight_Timer_Callback             // Callback function
    );

    if (backlightTimer == NULL) {
        // Handle timer creation failure - could add error logging here
    }
}

/**
 * @brief Turn backlight ON
 */
void Backlight_On(void)
{
    HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, GPIO_PIN_SET);
    backlight_state = 1;
}

/**
 * @brief Turn backlight OFF
 */
void Backlight_Off(void)
{
    HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin, GPIO_PIN_RESET);
    backlight_state = 0;
}

/**
 * @brief Handle button press - turn on backlight and start/restart timer
 */
void Backlight_Button_Pressed(void)
{
    // Turn on the backlight
    Backlight_On();
    // Stop timer if it's already running
    if (xTimerIsTimerActive(backlightTimer) != pdFALSE) {
        xTimerStop(backlightTimer, 0);
    }

    // Start the timer (25 second countdown)
    xTimerStart(backlightTimer, 0);

	if(Pro_Application_flag==0)
	{
		++dispState;
		if(EPROM_General.Modem_EC200_presence)
		{
			if(dispState > 6)
				dispState = 0;
		}
		else
		{
			if(dispState > 5)
				dispState = 0;
		}
	}
}

/**
 * @brief Timer callback - automatically turn off backlight after timeout
 */
void Backlight_Timer_Callback(TimerHandle_t xTimer)
{
    // Turn off backlight when timer expires
    Backlight_Off();
}

/**
 * @brief Get current backlight state
 * @return 1 if ON, 0 if OFF
 */
uint8_t Backlight_Get_State(void)
{
    return backlight_state;
}
//***************************************************** */


// Function to write a command to the Raystar LCD
char lcd_write_command(uint8_t command)
{
	char ret = 0;
	uint8_t data[2];
	data[0] = 0x00; // Co = 0, Rs = 0
	data[1] = command;
	ret = HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDRESS << 1, data, 2, 500);
    HAL_Delay(5);
    return ret;
}

// Function to write data to the Raystar LCD
void lcd_write_data(uint8_t data)
{

	uint8_t d[2];
	d[0] = 0x40;
	d[1]=data;
    HAL_I2C_Master_Transmit(&hi2c1, (LCD_I2C_ADDRESS << 1),	d, 2, 500);
    HAL_Delay(5);
}

// Function to display a string on the Raystar LCD
void lcd_display_string(const char *string)
{
    while (*string) {
        lcd_write_data(*string++);
    }
}

void lcd_clear()
{
    lcd_write_command(0x01);
    HAL_Delay(2); // Wait for the clear display command to complete
}

void lcd_initialize()
{
    // Initialize Raystar LCD

    // Step 1: Configure display for 8-bit data, 2-line display, 5x8 font
    if(lcd_write_command(0x38)==0)
    {
    	pro_I2C_State = 1;

    }
    else
    {
    	pro_I2C_State = 0;
    }
    HAL_Delay(2); // Wait for the command to be processed

    // Step 2: Turn on display, turn off cursor, disable blinking
    lcd_write_command(0x0C);
    HAL_Delay(2); // Wait for the command to be processed

    // Step 3: Clear the display
    lcd_write_command(0x01);
    HAL_Delay(2); // Wait for the clear display command to complete

    // Step 4: Set entry mode - Increment cursor, no display shift
    lcd_write_command(0x06);
    HAL_Delay(2); // Wait for the clear display command to complete
}

void lcd_set_cursor(uint8_t row, uint8_t  col)
{
	uint8_t pos;
	switch(row)
	{
		case 1:
			pos = 0x80 + col;
			break;
		case 2:
			pos = 0xC0 + col;
			break;
		default:
			pos = 0x80 + col; // Default to the first row if an invalid row is provided
			break;
	}
	lcd_write_command(pos);
}

void lcd_float_print(float value, int precision)
{
    char LCD_data[16] = {0};

    // %*.*f  -> first * = width, second * = precision
    sprintf(LCD_data, "%.*f ", precision, value);

    lcd_display_string((char *)LCD_data);
}
