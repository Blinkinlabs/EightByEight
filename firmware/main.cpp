/*
 * Blinky Controller
 *
* Copyright (c) 2014 Matt Mets
 *
 * based on Fadecandy Firmware, Copyright (c) 2013 Micah Elizabeth Scott
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

#include <math.h>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include "WProgram.h"
#include "usb_serial.h"
#include "usb_dev.h"
#include "jedecflash.h"

#include "leoblinky.h"
#include "animation.h"
#include "patterns.h"
#include "jedecflash.h"

#include "protocol.h"
#include "usb_serial.h"

#include "buttons.h"
#include "matrix.h"

// External flash chip
FlashSPI flash;

// Flash storage class, works on top of the base flash chip
//NoFatStorage flashStorage;

// Animations class
Animations animations;

// Button inputs
Buttons userButtons;

// Serial programming receiver
Protocol serialReceiver;

// Reserved RAM area for signalling entry to bootloader
extern uint32_t boot_token;

// Token to signal that the animation loop should be restarted
volatile bool reloadAnimations;

static void dfu_reboot()
{
    // Reboot to the Fadecandy Bootloader
    boot_token = 0x74624346;

    // Short delay to allow the host to receive the response to DFU_DETACH.
    uint32_t deadline = millis() + 10;
    while (millis() < deadline) {
        watchdog_refresh();
    }

    // Detach from USB, and use the watchdog to time out a 10ms USB disconnect.
    __disable_irq();
    USB0_CONTROL = 0;
    while (1);
}


void setupWatchdog() {
    // Change the watchdog timeout because the SPI access is too slow.
    const uint32_t watchdog_timeout = F_BUS / 2;  // 500ms

    WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
    WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
    asm volatile ("nop");
    asm volatile ("nop");
    WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN |
        WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN | WDOG_STCTRLH_CLKSRC;
    WDOG_PRESC = 0;
    WDOG_TOVALH = (watchdog_timeout >> 16) & 0xFFFF;
    WDOG_TOVALL = (watchdog_timeout)       & 0xFFFF;
}


// Handle full messages here
void handleData(uint16_t dataSize, uint8_t* data) {
    switch(data[0]) {
        case 0x00:  // Display some data on the LEDs
        {
    
            if(dataSize != 1 + LED_ROWS*LED_COLS) {
                return;
            }
            memcpy(getPixels(), &data[1], LED_ROWS*LED_COLS);
            show();
        }
        case 0x01:  // Clear the flash
        {
            if(dataSize != 1) {
                return;
            }
            flash.setWriteEnable(true);
            flash.eraseAll();
            while(flash.busy()) {
                watchdog_refresh();
                delay(100);
            }
            flash.setWriteEnable(false);

            // send an OK to the pc
            usb_serial_putchar('!');
        }
            break;

        case 0x02:  // Program a page of memory
        {
            if(dataSize != (1 + 4 + 256)) {
                return;
            }

            uint32_t address =
                  (data[1] << 24)
                + (data[2] << 16)
                + (data[3] <<  8)
                + (data[4]      );

            flash.setWriteEnable(true); 
            flash.writePage(address, (uint8_t*) &data[5]);
            while(flash.busy()) {
                delay(10);
                watchdog_refresh();
            }
            flash.setWriteEnable(false); 

            // send an OK to the pc
            usb_serial_putchar('!');
        }
            break;

        default:
            break;
    }
}


extern "C" int main()
{
    setupWatchdog();

    initBoard();

//    userButtons.setup();

    serialReceiver.reset();

    flash.begin(FlashClass::autoDetect);

    reloadAnimations = true;

    matrixSetup();

    // TODO: Fix pinMode()!
    PORTB_PCR0 = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;
    PORTB_PCR1 = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;
    GPIOB_PDDR |= 0x01 | 0x02;
    GPIOB_PDOR |= 0x01 | 0x02;
    PORTD_PCR5 = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;
    PORTD_PCR6 = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;
    GPIOD_PDDR |= 0x05 | 0x06;
    GPIOD_PDOR |= 0x05 | 0x06;


    // Application main loop
    while (usb_dfu_state == DFU_appIDLE) {
        #define BRIGHTNESS_COUNT 8
        static int brightnessLevels[BRIGHTNESS_COUNT] = {255,204,153,102,51,102,153,204};
        static int brightnessStep = 5;

        static bool streaming_mode;

        static int animation;          // Flash animation to show
        static int frame;              // current frame to display
        static uint32_t nextTime;           // Time to display next frame

        watchdog_refresh();

        // TODO: put this in an ISR? Make the buttons do pin change interrupts?
        userButtons.buttonTask();

        // TODO: Don't do this so often
        setBrightness(brightnessLevels[brightnessStep]/255.0);

        if(reloadAnimations) {
            reloadAnimations = false;
            animations.begin(flash);

            streaming_mode = false;
            animation = 0;
            frame = 0;
            nextTime = 0;
        }

#if 0
        if(!streaming_mode) {
            // If the flash wasn't initialized, show a default flashing pattern
            if(animations.getCount() == 0) {
                if(digitalRead(BUTTON_A_PIN) == LOW) {
                    for(int row = 0; row < LED_ROWS; row++) {
                        for(int col = 0; col < LED_COLS; col++) {
                            setPixel(col, row, 255);
                        }
                    }
                }
                else {
                    count_up_loop();
                }
                show();
            }
            else {

                // Flash-based
                if(millis() > nextTime) {
                    animations.getAnimation(animation)->getFrame(frame, getPixels());
                    frame++;
                    if(frame >= animations.getAnimation(animation)->frameCount) {
                        frame = 0;
                    }
    
                    nextTime += animations.getAnimation(animation)->speed;
    
                    // If we've gotten too far ahead of ourselves, reset the counter
                    if(millis() > nextTime) {
                        nextTime = millis() + animations.getAnimation(animation)->speed;
                    }
    
                    show();
                }
            }
        }
#endif

        if(!streaming_mode && millis() > nextTime) {
            if(animations.getAnimationCount() == 0) {
                    for(int row = 0; row < LED_ROWS; row++) {
                        for(int col = 0; col < LED_COLS; col++) {
                            setPixel(col, row, 100);
                            show();
                        }
                    }
            }
            else {
                animations.getAnimation(animation)->getFrame(frame, getPixels());
                show();
            }

            frame++;
            if(frame >= animations.getAnimation(animation)->frameCount) {
                frame = 0;
#if 1
                // increment through
                animation++;
                if(animation >= animations.getAnimationCount()) {
                    animation = 0;
                }
#else
                animation = floor(animations.getAnimationCount()*rand());
#endif
            }

            nextTime += animations.getAnimation(animation)->speed;
        }

        if(usb_serial_available() > 0) {
            streaming_mode = true;

            if(serialReceiver.parseByte(usb_serial_getchar())) {


              uint16_t dataSize = serialReceiver.getPacketSize();
              uint8_t* data = serialReceiver.getPacket();
              handleData(dataSize, data);
            }
        }

        if(userButtons.isPressed()) {
            uint8_t button = userButtons.getPressed();
    
            if(button == BUTTON_A) {
                brightnessStep = (brightnessStep + 1) % BRIGHTNESS_COUNT;
                setBrightness(brightnessLevels[brightnessStep]/255.0);
          }
        }

    }

    // Reboot into DFU bootloader
    dfu_reboot();
}
