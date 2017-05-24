#include "patterndemo.h"

void PatternDemo::begin()
{
    patternFile.open("0");

    lastTime = 0;
}

void PatternDemo::tapped()
{

}

bool PatternDemo::step(float ax, float ay, float az)
{
  unsigned long time = millis();

  if(time < (lastTime + patternFile.getFrameDelay())) {
      return false;
  }

  lastTime = time;

//    Serial.print("encoding:");
//    Serial.print(patternFile.getEncoding());
//    Serial.print(", ledCount:");
//    Serial.print(patternFile.getLedCount());
//    Serial.print(", frameCount:");
//    Serial.println(patternFile.getFrameCount());
//    delay(5);
//    Serial.print("frameDelay:");
//    Serial.print(patternFile.getFrameDelay());
//    Serial.print(", frameIndex:");
//    Serial.println(patternFile.getFrameIndex());
//    delay(5);

    patternFile.draw(frameData);

    // TODO: Test for timeout before advancing
    return true;
}

void PatternDemo::draw(RGBMatrix &matrix)
{
    matrix.set(frameData);
}
