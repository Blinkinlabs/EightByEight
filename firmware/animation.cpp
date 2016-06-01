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

#include "animation.h"
#include "matrix.h"

void Animation::getFrame(uint32_t frame, uint8_t* buffer) {
    flash->read(startingAddress + frame*LED_ROWS*LED_COLS,
                buffer,
                LED_ROWS*LED_COLS
        );
}


bool Animations::isInitialized() {
    return initialized;
}

void Animations::begin(FlashSPI& _flash) {
    initialized = false;
    flash = &_flash;

    // Test if the magic number is present
    uint32_t magicNumber;
    flash->read(ANIMATIONS_TABLE_ADDRESS,
                (uint8_t*) &magicNumber,
                sizeof(uint32_t)
        );

    if (magicNumber != ANIMATIONS_MAGIC_NUMBER) {
        return;
    }


    flash->read(ANIMATIONS_TABLE_ADDRESS + sizeof(uint32_t),
                (uint8_t*) &animationCount,
                sizeof(uint32_t)
        );

    if (animationCount > MAX_ANIMATIONS_COUNT) {
        animationCount = MAX_ANIMATIONS_COUNT;
    }

    for(uint32_t animation = 0; animation < animationCount; animation++) {
        uint32_t animationData[ANIMATIONS_TABLE_ENTRY_SIZE];

        flash->read(ANIMATIONS_TABLE_ADDRESS + (2 + animation*ANIMATIONS_TABLE_ENTRY_SIZE)*sizeof(uint32_t),
                    (uint8_t*) animationData,
                    ANIMATIONS_TABLE_ENTRY_SIZE*sizeof(uint32_t)
            );

        animations[animation].flash           = flash;
        animations[animation].frameCount      = animationData[0];
        animations[animation].speed           = animationData[1];
        animations[animation].startingAddress = animationData[2];
    }

    initialized = true;
}

uint32_t Animations::getAnimationCount() {
    return animationCount;
}

Animation* Animations::getAnimation(uint32_t animation) {
    return &(animations[animation]);
}
