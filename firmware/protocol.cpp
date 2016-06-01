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

#include "protocol.h"

void Protocol::init() {
  reset();
}

void Protocol::reset() {
  m_crc = 0;
  m_packetLength = 0;
  m_mode = MODE_HEADER0;
}

void Protocol::addByte(uint8_t data) {
  m_packetData[m_packetLength] = data;
  m_packetLength +=1;

  updateCRC(data);
}

boolean Protocol::parseByte(uint8_t data) {
  switch(m_mode) {
  case MODE_HEADER0:
  case MODE_PAYLOAD_READY: // If we get new data before a payload is handled, just start over.
    if(data == HEADER0) {
      reset();
      m_mode = MODE_HEADER1;
      updateCRC(data);
    }
    else {
    }
    break;
  case MODE_HEADER1:
    if(data == HEADER1) {
      m_mode = MODE_LENGTH0;
      updateCRC(data);
    }
    else {
      reset();
    }
    break;
  case MODE_LENGTH0:
    m_mode = MODE_LENGTH1;
    m_expectedLength = data << 8;
    updateCRC(data);
    break;
  case MODE_LENGTH1:
    m_mode = MODE_PAYLOAD;
    m_expectedLength |= data;
    updateCRC(data);
    if (m_expectedLength >= MAX_DATA_LENGTH) {
      reset();
    }
    break;
  case MODE_PAYLOAD:
    addByte(data);
    if(m_packetLength >= m_expectedLength) {  // TODO: why >= and not just ==?
      m_mode = MODE_CRC;
    } 
    break;
  case MODE_CRC:
    //if(data == m_crc) {
      m_mode = MODE_PAYLOAD_READY;
    //}
    //else {
    //  reset();
    //}
    break;
  default:
    reset();
  }

  return (m_mode == MODE_PAYLOAD_READY);
}

uint16_t Protocol::getPacketSize() {
  if(m_mode == MODE_PAYLOAD_READY) {
    return m_packetLength;  // first byte of payload data
  }
  else {
    return 0x0;
  }
}
  
uint8_t* Protocol::getPacket() {
  if(m_mode == MODE_PAYLOAD_READY) {
    m_mode = MODE_HEADER0;
    return m_packetData;  // first byte of payload data
  }
  else {
    return 0x0;
  }
}

uint16_t* Protocol::getPacket16() {
  if(m_mode == MODE_PAYLOAD_READY) {
    m_mode = MODE_HEADER0;
    return reinterpret_cast<uint16_t*>(m_packetData);  // first byte of payload data
  }
  else {
    return 0x0;
  }
}

void Protocol::updateCRC(uint8_t data) {
  uint8_t i;

  m_crc = m_crc ^ data;
  for (i = 0; i < 8; i++)
  {
    if(m_crc & 0x01) {
      m_crc = (m_crc >> 1) ^ 0x8C;
    }
    else {
      m_crc >>= 1;
    }
  }
}


