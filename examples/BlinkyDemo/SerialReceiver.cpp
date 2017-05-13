#include "Arduino.h"
#include "SerialReceiver.h"

void SerialReceiver::begin() {
  reset();
}

void SerialReceiver::tapped() {
}

void SerialReceiver::reset() {
  serialMode = SERIAL_MODE_DATA;

  bufferIndex = 0;
  pixelIndex = 0;

  escapeRunCount = 0;

  controlBufferIndex = 0;
}

bool SerialReceiver::step(float ax, float ay, float az) {
  while(Serial.available()) {
  
    switch(serialMode) {
        case SERIAL_MODE_DATA:
            return readData();
            break;
  //      case SERIAL_MODE_COMMAND:
  //          commandLoop();
  //          break;
        default:
            reset();
    }
  }
}

bool SerialReceiver::readData() {
    bool canDraw = false;
  
    uint8_t c = Serial.read();

    // Pixel character
    if(c != 0xFF) {
        // Reset the control character state variables
        escapeRunCount = 0;

        // Buffer the color
        buffer[bufferIndex++] = c;

        // If this makes a complete pixel color, update the display and reset for the next color
        if(bufferIndex > 2) {
            bufferIndex = 0;

            // Prevent overflow by ignoring any pixel data beyond LED_COUNT
            if(pixelIndex < LED_ROWS*LED_COLS) {
//                dmxSetPixel(pixelIndex, buffer[2], buffer[1], buffer[0]);
                pixelIndex++;
            }
        }
    }

    // Control character
    else {
        // reset the pixel character state vairables
        bufferIndex = 0;
        pixelIndex = 0;

        escapeRunCount++;

        // If this is the first escape character, refresh the output
        if(escapeRunCount == 1) {
            canDraw = true;
        }
        
        if(escapeRunCount > 8) {
            serialMode = SERIAL_MODE_COMMAND;
            controlBufferIndex = 0;
        }
    }

    return canDraw;
}

void SerialReceiver::draw(RGBMatrix &matrix) {

}
