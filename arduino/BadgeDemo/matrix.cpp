/*
 * Copyright (c) 2016, Blinkinlabs, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "matrix.h"

#include <Arduino.h>
//#include <Serial.h>

uint8_t colorData[LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL];

void Matrix::setup() {
  Serial1.begin( 230400 );
}

void Matrix::setPixelColor(int row, int col, uint8_t r, uint8_t g, uint8_t b) {
  if(r == 255) {
    r = 254;
  }
  if(g == 255) {
    g = 254;
  }
  if(b == 255) {
    b = 254;
  }
  
  colorData[(row*LED_COLS + col)*3 + 0] = r;
  colorData[(row*LED_COLS + col)*3 + 1] = g;
  colorData[(row*LED_COLS + col)*3 + 2] = b;
}

void Matrix::show() {
  // LED control
  for(int i = 0; i < LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL; i++) {
    Serial1.print(char(colorData[i]));
  }
  Serial1.print(char(255));
}

uint8_t* Matrix::getPixels() {
  return colorData;
}

