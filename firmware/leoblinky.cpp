#include "WProgram.h"
#include "pins_arduino.h"
#include "leoblinky.h"

void initBoard() {
//    pinMode(BUTTON_A_PIN, INPUT);

    // Read the status pin.
    PORTC_PCR3 = PORT_PCR_MUX(1) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_SRE;
    GPIOC_PDDR &= ~(1<<3);
}

bool readButton() {
//    return digitalRead(BUTTON_A_PIN) == LOW;
    uint32_t status = GPIOC_PDIR & (1<<3);
    return status == 0;
}
