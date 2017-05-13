#include <Arduino.h>
#include "ColorSwirl.h"


void ColorSwirl::begin() {
  reset();
}

void ColorSwirl::tapped()
{
}

void ColorSwirl::reset() {
  j = 0;
  f = 0;
  k = 0;  
}

bool ColorSwirl::step(float ax, float ay, float az) {  
  j += .03*12;
  f += .02*12;
  k += .04*12;

  return true;
}

void ColorSwirl::draw(RGBMatrix &matrix) {  
  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      uint8_t r = 127*(1+sin(row/2.0 + -col/3.0 + j/4.0       ));
      uint8_t g = 127*(1+sin(-row/1.0 + col/4.0 + f/9.0  + 2.1));
      uint8_t b = 127*(1+sin(row/3.0 + -col/2.0 + k/14.0 + 4.2));
      matrix.blend(row, col, 8, r, g, b);
    }
  }
  matrix.show();
}

