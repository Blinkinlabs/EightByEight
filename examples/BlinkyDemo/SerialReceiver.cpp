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
  while (Serial.available()) {

    switch (serialMode) {
      case SERIAL_MODE_DATA:
        if(readData()) {
          return true;
        }
        break;
      //      case SERIAL_MODE_COMMAND:
      //          commandLoop();
      //          break;
      default:
        reset();
    }
  }

  return false;
}

bool SerialReceiver::readData() {
  bool canDraw = false;

  uint8_t c = Serial.read();

  // Pixel character
  if (c != 0xFF) {
    // Reset the control character state variables
    escapeRunCount = 0;

    // Copy this byte into the pixel array
    // TODO: Copy directly into the buffer
    // Buffer the color
    buffer[bufferIndex++] = c;

    // If this makes a complete pixel color, update the display and reset for the next color
    if (bufferIndex > 2) {
      bufferIndex = 0;

      // Prevent overflow by ignoring any pixel data beyond LED_COUNT
      if (pixelIndex < LED_ROWS * LED_COLS) {
        data[pixelIndex * LED_BYTES_PER_PIXEL + 0] = buffer[0];
        data[pixelIndex * LED_BYTES_PER_PIXEL + 1] = buffer[1];
        data[pixelIndex * LED_BYTES_PER_PIXEL + 2] = buffer[2];

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
    if (escapeRunCount == 1) {
      canDraw = true;
    }

    if (escapeRunCount > 8) {
      serialMode = SERIAL_MODE_COMMAND;
      controlBufferIndex = 0;
    }
  }

  return canDraw;
}

void SerialReceiver::draw(RGBMatrix &matrix) {
//  for(int x = 0 ; x < LED_ROWS ; x++)
//  {
//    for(int y = 0 ; y < LED_COLS ; y++)
//    {
//      matrix.blend(x, y, 255, 30, 255, 152);
//    }
//  }
  matrix.set(data);
}

