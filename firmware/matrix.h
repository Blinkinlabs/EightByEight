/*
 * DMA Matrix Driver
 * 
 * Copyright (c) 2014 Matt Mets
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

#ifndef MATRIX_H
#define MATRIX_H

#include "WProgram.h"
#include "pins_arduino.h"
#include "eightbyeight.h"

//Display Geometry
// Note the bits per channels is equal to BIT_DEPTH + PAGED_BITS
#define BIT_DEPTH 11       // Color bits per channel (Note: input is always 8 bit)
#define PAGED_BITS 2       // Number of pages (simulate higher bit depths)

#define PAGES 4

#if BYTES_PER_PIXEL == 3
// RGB pixel type
struct Pixel {
  uint8_t R;
  uint8_t G;
  uint8_t B;
};
#elif BYTES_PER_PIXEL == 1
// Grayscale pixel type
typedef uint8_t Pixel;
#endif

// Set up the matrix and start running it's display loop
extern void matrixSetup();

// Re-start a paused matrix
extern void matrixStart();

// Change the system brightness
// @param brightness Display brightness scale, from 0 (off) to 1 (fully on)
extern void setBrightness(float brightness);

#if BYTES_PER_PIXEL == 3
// Update a single pixel in the array
// @param column int pixel column (0 to led_cols - 1)
// @param row int pixel row (0 to led_rows - 1)
// @param r uint8_t new red value for the pixel (0 - 255)
// @param g uint8_t new green value for the pixel (0 - 255)
// @param b uint8_t new blue value for the pixel (0 - 255)
extern void setPixel(int column, int row, uint8_t r, uint8_t g, uint8_t b);

#elif BYTES_PER_PIXEL == 1
// Update a single pixel in the array
// @param column int pixel column (0 to led_cols - 1)
// @param row int pixel row (0 to led_rows - 1)
// @param value uint8_t new value for the pixel (0 - 255)
extern void setPixel(int column, int row, uint8_t value);

#endif

// Update the matrix using the data in the Pixels[] array
extern void show();

// Get the display pixel buffer
// @return Pointer to the pixel display buffer, a uint8_t array of size
// LED_ROWS*LED_COLS
extern Pixel* getPixels();

// The display is double-buffered internally. This function returns
// true if there is already an update waiting.
extern bool bufferWaiting();

#endif
