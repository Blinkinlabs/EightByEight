/** \file
 * UDP Pixel data receiver.
 */

#include "Demo.h"

#define SERIAL_MODE_DATA     0x01
#define SERIAL_MODE_COMMAND  0x02

#define MAX_DATA_LENGTH 255

#define FILEMODE_READ  0
#define FILEMODE_WRITE 1

class SerialReceiver : public Demo
{
private:
  void reset();

  bool readData();
  void readCommand();

  int serialMode;         // Serial protocol we are speaking

  ///// Defines for the data mode
  void dataLoop();

  int escapeRunCount;     // Count of how many escape characters we've received
  uint8_t buffer[3];      // Buffer for one pixel of data
  int bufferIndex;        // Current location in the buffer
  int pixelIndex;         // Pixel we are currently writing to

  int receiveState;

  uint8_t commandBuffer[MAX_DATA_LENGTH+2];
  int commandBufferIndex;

  uint8_t data[LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL];


public:
  virtual void begin();
  virtual void tapped();

  virtual bool step(float ax, float ay, float az);
  virtual void draw(RGBMatrix &matrix);
};
