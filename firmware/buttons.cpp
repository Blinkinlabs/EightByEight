#include "WProgram.h"
#include "pins_arduino.h"
#include "buttons.h"
#include "leoblinky.h"

// Button definitions
button buttons[BUTTON_COUNT] = {
    {BUTTON_A_PIN,     0},  // Button A, PD4
};

void Buttons::setup() {
    for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
        pinMode(buttons[b].pin, INPUT_PULLUP);
    }
    
    pressedButton = BUTTON_COUNT;
    lastPressed = BUTTON_COUNT;
}


// Scan for new button presses
void Buttons::buttonTask() {
    // If a button is currently pressed, don't bother looking for a new one
    if (lastPressed != BUTTON_COUNT) {
        if(digitalRead(buttons[lastPressed].pin) == buttons[lastPressed].inverted) {
            if(debounceCount < DEBOUNCE_INTERVAL) {
                debounceCount++;
            }
            return;
        }
        lastPressed = BUTTON_COUNT;
    }
    
    for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
        // TODO: Use port access here for speed?
        if (digitalRead(buttons[b].pin) == buttons[b].inverted) {
            lastPressed = b;
            pressedButton = b;
            debounceCount = 0;
            return;
        }
    }
}


bool Buttons::isPressed() {
    return (pressedButton != BUTTON_COUNT && debounceCount == DEBOUNCE_INTERVAL);
}

// If a button was pressed, return it!
int Buttons::getPressed() {
    int pressed;

// TODO: if this goes in an isr, we need to implement a lock here
//    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
//    {
            pressed = pressedButton;
            pressedButton = BUTTON_COUNT;
//        }

    return pressed;
}
