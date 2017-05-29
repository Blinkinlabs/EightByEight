/** \file
 * RGB LED matrix on the blinky badges.
 *
 * RGB values are 0-255, blend values are 0-255.
 */
#ifndef _rgbmatrix_h_
#define _rgbmatrix_h_

#include <stdint.h>

#define LED_ROWS 8
#define LED_COLS 8
#define LED_BYTES_PER_PIXEL 3

#define DATA_SIZE (LED_ROWS * LED_COLS * LED_BYTES_PER_PIXEL)

class RGBMatrix
{
public:
	RGBMatrix();
	void begin();
	void show();

	void clear();
	void set(int row, int col, int r, int g, int b);
	void blend(int row, int col, int blend, int r, int g, int b);

	void set(int row, int col, uint32_t color);
	void blend(int row, int col, int blend, uint32_t color);

  void set(uint8_t *frame);

  void setBrightness(uint8_t brightness);

private:
    uint8_t data[DATA_SIZE];
};


#endif


