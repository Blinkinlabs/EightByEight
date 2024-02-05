/** \file
 * Driver for the Blinky Badge RGB matrix.
 *
 */
#include <Arduino.h>
#include "RGBMatrix.h"

RGBMatrix::RGBMatrix()
{
	this->clear();
}


void RGBMatrix::clear()
{
//    for(int i = 0 ; i < LED_ROWS; i++)
//        for(int j = 0 ; j < LED_COLS ; j++)
//            set(i, j, 0, 0, 0);

    memset(data, 0, DATA_SIZE);
}

void RGBMatrix::begin()
{
	// Serial communication to RGB matrix
	Serial1.begin( 230400 );
	this->clear();
	this->show();
	this->show();

  setBrightness(100);
}

void RGBMatrix::setBrightness(uint8_t brightness)
{
  // Escape to command mode
  for(int i = 0; i < 9; i++) {
    Serial1.print(char(255));
  }
  Serial1.print(char(1)); // Set brightness command
  Serial1.print(char(brightness)); // And the brightness data
}

void RGBMatrix::set(
	int row,
	int col,
	int r,
	int g,
	int b
)
{
	if (row < 0 || row >= LED_ROWS)
		return;

	if (col < 0 || col >= LED_COLS)
		return;

	uint8_t * const p = &data[3*(row*LED_COLS + col)];
	p[0] = r;
	p[1] = g;
	p[2] = b;
}

void RGBMatrix::blend(
	int row,
	int col,
	int blend,
	int r,
	int g,
	int b
)
{
	if (row < 0 || row >= LED_ROWS)
		return;

	if (col < 0 || col >= LED_COLS)
		return;

	if (blend < 0) blend = 0;
 	if (blend > 255) blend = 255;
	
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	uint8_t * const p = &data[3*(row*LED_COLS + col)];
	r = (p[0] * (255 - blend) + r * blend) / 256;
	g = (p[1] * (255 - blend) + g * blend) / 256;
	b = (p[2] * (255 - blend) + b * blend) / 256;

    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;

	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;

	p[0] = r;
	p[1] = g;
	p[2] = b;
}

void RGBMatrix::blend(
	int row,
	int col,
	int blend,
	uint32_t rgb
)
{
	this->blend(
		row,
		col,
		blend,
		(rgb >> 16) & 0xFF,
		(rgb >>  8) & 0xFF,
		(rgb >>  0) & 0xFF
	);
}

void RGBMatrix::set(
	int row,
	int col,
	uint32_t rgb
)
{
	this->set(
		row,
		col,
		(rgb >> 16) & 0xFF,
		(rgb >>  8) & 0xFF,
		(rgb >>  0) & 0xFF
	);
}

void RGBMatrix::set(uint8_t* frame)
{
  memcpy(data, frame, LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL);
}

void RGBMatrix::show()
{
  for(int i = 0; i < LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL; i++)
  {
    uint8_t c = data[i];
    if(c == 255) {
        c = 254;
    }

    Serial1.print(char(c));
  }

  Serial1.print(char(255));
}
