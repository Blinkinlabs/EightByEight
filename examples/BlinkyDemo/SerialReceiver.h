/** \file
 * UDP Pixel data receiver.
 */

#include "Demo.h"

#define SERIAL_MODE_DATA     0x01
#define SERIAL_MODE_COMMAND  0x02

#define CONTROL_BUFFER_SIZE 300

class SerialReceiver : public Demo
{
private:
  void reset();

  bool readData();
//  bool readCommand();

  int serialMode;         // Serial protocol we are speaking

  ///// Defines for the data mode
  void dataLoop();

  int escapeRunCount;     // Count of how many escape characters we've received
  uint8_t buffer[3];      // Buffer for one pixel of data
  int bufferIndex;        // Current location in the buffer
  int pixelIndex;         // Pixel we are currently writing to


//  uint8_t controlBuffer[CONTROL_BUFFER_SIZE];     // Buffer for receiving command data
  int controlBufferIndex;     // Current location in the buffer

public:
  virtual void begin();
  virtual void tapped();

  virtual bool step(float ax, float ay, float az);
  virtual void draw(RGBMatrix &matrix);
};


