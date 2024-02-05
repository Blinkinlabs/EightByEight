#ifndef PATTERNDEMO_H
#define PATTERNDEMO_H

#include "Demo.h"

#include "patternfile.h"

class PatternDemo : public Demo
{
private:
    long lastTime;

    uint8_t frameData[LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL];

public:
    PatternFile patternFile;

    void begin();
    void tapped();
    bool step(float ax, float ay, float az);
    void draw(RGBMatrix &matrix);

    bool finished();

    bool open(String fileName);
};

#endif // PATTERNDEMO_H
