#include "patterndemo.h"

void PatternDemo::begin()
{
    memset(frameData, 0, LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL);
}

bool PatternDemo::open(String fileName)
{
    lastTime = 0;

    return (patternFile.open(fileName) == ERROR_NO_ERROR);
}

void PatternDemo::tapped()
{

}

bool PatternDemo::done() {
  return patternFile.getDone();
}

bool PatternDemo::step(float ax, float ay, float az)
{
  unsigned long time = millis();

  if(time < (lastTime + patternFile.getFrameDelay())) {
      return false;
  }

  lastTime = time;

  return (patternFile.draw(frameData) == ERROR_NO_ERROR);
}

void PatternDemo::draw(RGBMatrix &matrix)
{
    matrix.set(frameData);
}
