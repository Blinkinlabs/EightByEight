/*
 * Blinky Controller
 *
* Copyright (c) 2014, 2016 Blinkinlabs, LLC
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

#include "eightbyeight.h"
#include "patterns.h"

//#include "protocol.h"
#include "serialloop.h"
#include "usb_serial.h"

#include "matrix.h"

// Serial programming receiver
//Protocol serialReceiver;

// Reserved RAM area for signalling entry to bootloader
extern uint32_t boot_token;

static void dfu_reboot()
{
    // Reboot to the Bootloader
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
    
            if(dataSize != 1 + LED_ROWS*LED_COLS*BYTES_PER_PIXEL) {
                return;
            }
            memcpy(getPixels(), &data[1], LED_ROWS*LED_COLS*BYTES_PER_PIXEL);
            show();
        }
        default:
            break;
    }
}


extern "C" int main()
{
    setupWatchdog();

    // Delay here for some time, in case we mess up the firmware.
    for(uint64_t timeout = 0; timeout < 5000000; timeout++) {
        watchdog_refresh();

        if (usb_dfu_state != DFU_appIDLE) {
            dfu_reboot();
        }
    }
        

    initBoard();

//    serialReceiver.reset();
    serialReset();

    matrixSetup();

    #define BRIGHTNESS_COUNT 8
    const int brightnessLevels[BRIGHTNESS_COUNT] = {255,204,153,102,51,102,153,204};

    int brightnessStep = 5;
    bool streaming_mode = false;

    // Application main loop
    while (usb_dfu_state == DFU_appIDLE) {

        watchdog_refresh();

        // TODO: Don't do this so often
        setBrightness(brightnessLevels[brightnessStep]/255.0);

        if(!streaming_mode) {
            setPixel(5,0,255,0,0);
            setPixel(7,0,0,255,0);
            setPixel(6,1,255,0,0);
            setPixel(5,2,255,0,0);
            setPixel(7,2,0,255,0);
            setPixel(6,3,255,0,0);
            setPixel(5,4,255,0,0);
            setPixel(7,4,0,255,0);
            setPixel(6,5,255,0,0);
            setPixel(5,6,255,0,0);
            setPixel(7,6,0,255,0);
            setPixel(6,7,255,0,0);
            show();
        }

        if(usb_serial_available() > 0) {
            streaming_mode = true;
            serialLoop();
        }
    }

    // Reboot into DFU bootloader
    dfu_reboot();
}
