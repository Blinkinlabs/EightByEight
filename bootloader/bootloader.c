/*
 * Fadecandy DFU Bootloader
 *
 * Modified by Matt Mets to work with parts that do not contain flexram
 * 
 * Copyright (c) 2013 Micah Elizabeth Scott
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

#include <stdbool.h>
#include "usb_dev.h"
#include "dfu.h"
#include "serial.h"
//#include "mk20dx128.h"
#include "mk20dn64.h"

extern uint32_t boot_token;
static __attribute__ ((section(".appvectors"))) uint32_t appVectors[64];

//const uint32_t led_bit =      1 << 6;  // LED is on Port D6

static void led_init()
{
//    // Set the status LED on PD6, as an indication that we're in bootloading mode.
//    PORTD_PCR6 = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE;
//    GPIOD_PDDR |= led_bit;
//    GPIOD_PDOR |= led_bit;
}

static void led_toggle()
{
//    GPIOD_PTOR = led_bit;
}

static bool test_boot_token()
{
    /*
     * If we find a valid boot token in RAM, the application is asking us explicitly
     * to enter DFU mode. This is used to implement the DFU_DETACH command when the app
     * is running.
     */

    return boot_token == 0x74624346;
}

static bool test_app_missing()
{
    /*
     * If there doesn't seem to be a valid application installed, we always go to
     * bootloader mode.
     */

    uint32_t entry = appVectors[1];
    return entry < 0x00001000 || entry >= 128 * 1024;
}

static bool test_user_buttons() {
    /*
     * At startup we check the status of a GPIO input to see if we should enter DFU
     * mode directly. This is intended to be a recovery mechanism for a misbehaving
     * application firmware that prevents DFU mode.
     */

    // Recovery pin is on port C, pin 4
    PORTC_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_PFE;

    const uint32_t recovery_pin_bit = (1 << 4);

    GPIOC_PDDR = GPIOC_PDDR & (~recovery_pin_bit);
    uint32_t status = GPIOC_PDIR & recovery_pin_bit;

    return status == 0;
}

static void app_launch()
{
    // Relocate IVT to application flash
    __disable_irq();
    SCB_VTOR = (uint32_t) &appVectors[0];

    // Refresh watchdog right before launching app
    watchdog_refresh();

    // Clear the boot token, so we don't repeatedly enter DFU mode.
    boot_token = 0;

    asm volatile (
        "mov lr, %0 \n\t"
        "mov sp, %1 \n\t"
        "bx %2 \n\t"
        : : "r" (0xFFFFFFFF),
            "r" (appVectors[0]),
            "r" (appVectors[1]) );
}

int main()
{
    if (test_user_buttons() || test_app_missing() || test_boot_token()) {
        unsigned i, j;

        // We're doing DFU mode!
        led_init();
        dfu_init();
        usb_init();

        // Wait for firmware download
        while (dfu_getstate() != dfuMANIFEST) {

            __disable_irq();
            fl_state_poll();  // Help push programming forward, since it's dependent on polling
            __enable_irq();
            // delay(2)

            watchdog_refresh();
        }

        // Clear boot token, to enter the new application
        boot_token = 0;

        // Wait a little bit longer, while flashing the LED.
        for (i = 11; i; --i) {
            led_toggle();
            for (j = 100000; j; --j) {
                watchdog_refresh();
            }
        }

        // USB disconnect and reboot, using watchdog to time 10ms.
        watchdog_refresh();
        __disable_irq();
        USB0_CONTROL = 0;
        while (1);
    }

    app_launch();
    return 0;
}
