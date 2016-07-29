/*
 * Peripherals for the EightByEight Blinky Badge
 *
 * Copyright (c) 2016 Blinkinlabs, LLC
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

#include "WProgram.h"
#include "pins_arduino.h"
#include "eightbyeight.h"

void initBoard() {
    GPIOB_PDDR |= 0x03;
    GPIOB_PSOR  = 0x03;
    CORE_PIN5_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(1);
    CORE_PIN6_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(1);
/*
    // Software RTS and DTR signals
    // TODO: Test that I don't glitch the reset line.
    digitalWrite(ESP_RESET_PIN, HIGH);
    digitalWrite(ESP_GPIO0_PIN, HIGH);

    pinMode(ESP_RESET_PIN, OUTPUT);
    pinMode(ESP_GPIO0_PIN, OUTPUT);
*/
}
