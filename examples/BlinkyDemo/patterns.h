#ifndef PATTERNS_H
#define PATTERNS_H

#include "patterndemo.h"

class Patterns
{
private:
    uint32_t count;

public:
    void begin();

    uint32_t getCount();

    PatternDemo open(int index);
};

#endif // PATTERNS_H
