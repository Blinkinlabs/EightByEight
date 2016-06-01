/*
 * CRC-checked serial packet protocol
 * 
 * Copyright (c) 2014 Matt Mets
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

#ifndef _PROTOCOL_h
#define _PROTOCOL_h

#include "WProgram.h"

#define MAX_DATA_LENGTH     500  // Maximum length of the data packet we can receive (up to 65534)

#define MODE_HEADER0        0x0     // Expecting the first header byte
#define MODE_HEADER1        0x1     // Expecting the second header byte
#define MODE_LENGTH0        0x2     // Expecting the high byte of the packet length
#define MODE_LENGTH1        0x3     // Expecting the low byte of the packet length
#define MODE_PAYLOAD        0x4
#define MODE_CRC            0x5
#define MODE_PAYLOAD_READY  0x6

#define HEADER0             0xde
#define HEADER1             0xad

// AnalogLED protocol decoder. Expects frames of data that look like this:
// [header (2 bytes)][length (2 bytes)][payload (length bytes)][ibutton crc (1 byte)]
class Protocol {
private:
  uint16_t m_expectedLength;                ///< Number of bytes expected in the current packet  
  uint8_t m_mode;                           ///< Current operational mode
  uint8_t m_packetData[MAX_DATA_LENGTH];    ///< Data received in this packet
  uint16_t m_packetLength;                  ///< Number of bytes received for this packet
  uint8_t m_crc;                           ///< CRC calculated for this packet

  // add a new byte to the CRC
  void updateCRC(uint8_t data);

  // store a new byte in the packet
  void addByte(uint8_t data);
public:
  // Initialize the packet processor
  void init();
  
  // Reset the packet processor state machine
  void reset();
  
  // Read a byte into the packet processor, and handle it as possible.
  // @param data Data byte to read in
  // @return true if a payload is ready to read
  boolean parseByte(uint8_t data);
  
  // Get the packet size (in bytes)
  uint16_t getPacketSize();
  
  // If a full packet was received, returns the address to the data and resets the
  // packet state machine. Otherwise, returns null
  uint8_t* getPacket();

  // If a full packet was received, returns the address to the data as a 16-bit array,
  // and resets the packet state machine. Otherwise, returns null
  uint16_t* getPacket16();
};

#endif

