/*
 * Peripherals for the EightByEight Blinky Badge
 *
 * Copyright (c) 2016 Blinkinlabs, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef EIGHTBYEIGHT_H
#define EIGHTBYEIGHT_H

#include "mk20dn64.h"

#define LED_COLS 8          // Number of columns that the LED matrix has
#define LED_ROWS 8          // Number of rows that the LED matrix has
#define BYTES_PER_PIXEL     3

#define LED_COUNT           (LED_COLS*LED_ROWS)  // Number of LEDs we are controlling

// Output assignments
// Note: These can't be changed arbitrarily- the GPIOs are actually
// referred to in the library by their port assignments.
#define LED_HS_EN_PIN   7   // PC1
//#define LED_CLOCK_PIN   11  // SPI0 CLK / PC6
//#define LED_DATA_PIN    12  // SPI0 MOSI / PC5
//#define LED_STROBE_PIN  14  // FTM0 channel 4 / PD4
//#define LED_OE_PIN      4   // FTM0 channel 1 / PA4
#define S0              15  // PD5
#define S1              16  // PD6
#define S2              17  // PD7

// Initialize the board hardware
extern void initBoard();

// Refresh the watchdog, so that the board doesn't reset
static inline void watchdog_refresh(void)
{
    __disable_irq();
    WDOG_REFRESH = 0xA602;
    WDOG_REFRESH = 0xB480;
    __enable_irq();
}

#endif

