/*
 * Sample algorithmic patterns
 *
 * Copyright (c) 2014, 2016 Blinkinlabs, LLC
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

#include "patterns.h"
#include "matrix.h"

void bleedTest() {

    memset(matrix.getPixels(), 0, LED_ROWS*LED_COLS*BYTES_PER_PIXEL);
    static float i = 0;

    for(int row = 0; row < LED_ROWS; row++) {
        for(int col = 0; col < LED_COLS; col++) {
            //uint8_t val = row*LED_COLS + col + i;
            uint8_t val = 0;
            if(col == row) {
                val = 254;
            }

            matrix.setPixelColor(row,col, val, val, val);
        }
    }

    i += .1;
}

void countUpLoop() {
    static int pixel = 0;
    #define slowdown 5000
   
    for (uint16_t row = 0; row < LED_ROWS; row+=1) {
        for (uint16_t col = 0; col < LED_COLS; col+=1) {
            if(col == pixel/slowdown) {
                matrix.setPixelColor(col, row, Pixel(1,0,200));
            }
            else {
                matrix.setPixelColor(col, row, Pixel(0,0,0));
            }
        }
    }
    
    pixel = (pixel+1)%(LED_COLS*slowdown);
}

void colorSwirl() {  
    static float j = 0;
    static float f = 0;
    static float k = 0;
    
    for (uint8_t row = 0; row < LED_ROWS; row++) {
    	for (uint8_t col = 0; col < LED_COLS; col++) {
            uint8_t r = 1;//64*(1+sin(row/2.0 + -col/3.0 + j/4.0       ));
            uint8_t g = 1;//64*(1+sin(-row/1.0 + col/4.0 + f/9.0  + 2.1));
            uint8_t b = 1;//64*(1+sin(row/3.0 + -col/2.0 + k/14.0 + 4.2));
            matrix.setPixelColor(row, col, r, g, b);
        }
    }
    matrix.show();
    
    j = j + .3;
    f = f + .2;
    k = k + .4;
}

