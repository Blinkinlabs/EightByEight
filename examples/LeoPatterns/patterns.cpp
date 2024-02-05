#include "patterns.h"
#include <Arduino.h>
#include "FS.h"

// TODO: Handle this at a higher level?
extern volatile bool fileAccessLocked;
extern volatile bool reloadAnimations;

void Patterns::begin() {
    count = 0;

    if(fileAccessLocked) {
        return;
    }

    Dir dir = SPIFFS.openDir("/p/");
    while (dir.next()) {
        count++;
    }
}

uint32_t Patterns::getCount() {
    return count;
}

PatternDemo Patterns::open(int index) {
    Dir dir = SPIFFS.openDir("/p/");
    while (dir.next() && (index > 0)) {
        index--;
    }

    PatternDemo patternDemo;
    patternDemo.begin();
    patternDemo.open(dir.fileName());

    return patternDemo;
}
