/*
 * LED Animation loader/player
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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "jedecflash.h"


class Animation {
  public:
    FlashSPI* flash;    // Flash chip to read from
    uint32_t frameCount;         // Number of frames in this animation
    uint32_t speed;              // Speed, in ms between frames
    uint32_t startingAddress;    // Address in flash of the first animation frame.

    void getFrame(uint32_t frame, uint8_t* buffer);
};

// Max. number of animations that can be read from the flash (arbitrary)
#define MAX_ANIMATIONS_COUNT 20

// Address in flash memory of the Animations table
#define ANIMATIONS_TABLE_ADDRESS    0x00000000

// Number of uint32_t entries per animation in the animation table
#define ANIMATIONS_TABLE_ENTRY_SIZE 3

#define ANIMATIONS_MAGIC_NUMBER 0x12345679

// Animations table is an array of uint32_t integers, and looks like this:
// Note that entries are 4 bytes each.
// 0: magic number (0x12345678)
// 1: animation count (number of animations in table)
// 2 + n*3: animation n: frameCount
// 2 + n*3: animation n: speed
// 2 + n*3: animation n: flashAddress

class Animations {
  private:
    FlashSPI* flash;

    Animation animations[MAX_ANIMATIONS_COUNT];
    uint32_t animationCount;    // Number of animations in this class

    bool initialized;

  public:
    void begin(FlashSPI& _flash);

    // True if the animations table was read correctly from flash
    bool isInitialized();

    // Read the number of animations stored in the flash
    uint32_t getAnimationCount();

    // Get the requested animation
    Animation* getAnimation(uint32_t animation);
};

#endif
