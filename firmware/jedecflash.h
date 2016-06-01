/*
 * Generic JEDEC-compatible SPI flash library
 *
 * Copyright (c) 2014 Matt Mets
 *
 * based on Winbond spi flash memory chip operating library for Arduino
 * by WarMonkey (luoshumymail@gmail.com)
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

#ifndef _JEDECFLASH_H__
#define _JEDECFLASH_H__

#include <inttypes.h>
#include "spi4teensy3.h"

class FlashClass {
public:  
    enum manufacturerId {
        Spansion = 0x01,
        Winbond = 0xEF,
    };

    enum partNumber {
        autoDetect,
        W25Q80,
        W25Q16,
        W25Q32,
        W25Q64,
        W25Q128,
        S25FL208K,
        S25FL216K,
    };

    // Attempt to initialize and identify the flash memory
    // @param _partno Part Number (leave unspecified to autodetect)
    // @return True if the specified flash memory was detected, false otherwise.
    bool begin(partNumber _partno = autoDetect);

    // Power down the flash chip
    void end();

    // Get the size of this flash memory, in bytes
    long bytes();

    // Get the number of 256-byte pages in this flash memory
    uint16_t pages();

    // Get the number of 4096-byte sectors in this flash memory
    uint16_t sectors();

    // Get the number of blocks in this flash memory
    uint16_t blocks();

    // Read some data from the flash
    // @param address Address to read from
    // @param buffer Buffer to store data in
    // @param n Number of bytes to read
    // @return Number of bytes read
    uint16_t read(uint32_t address, uint8_t *buffer, uint16_t n);

    // Change the flash write enable status
    // Flash write must be enabled before any write or erase operations.
    // @param status If true, enable flash write, otherwise disable
    void setWriteEnable(bool status);
    
    // Write a 256 byte page to flash
    // @param address Starting address, must be 8-bit aligned (0x00ffff00)
    // @param buffer 256 byte data buffer to write
    // Note: The sector containing this page must be erased separately before writing.
    void writePage(uint32_t address,uint8_t *buffer);

    // Erase a sector (4096 bytes)
    // @param address Address of the block, must be 12bit-aligned (0x00fff000)
    void eraseSector(uint32_t address);

    // Erase a 64k block (65536 bytes)
    // @param address Address of the block, must be 16bit-aligned (0x00ff0000)
    // Note: This operation might take multiple seconds to complete
    void erase64kBlock(uint32_t address);

    // Erase the entire flash memory
    // Note: This operation might take multiple seconds to complete
    void eraseAll();

    // Poll the chip to see if an operation is in progress
    // @return true if busy, false otherwise
    bool busy();
    
    uint8_t  readManufacturer();
    uint16_t readPartID();
    uint16_t readSR();

private:
    partNumber partno;
    bool checkPartNo(partNumber _partno);

protected:
    virtual void select() = 0;
    virtual void send(uint8_t x, bool deselect = false) = 0;
    virtual uint8_t receive(uint8_t x, bool deselect = false) = 0;
    virtual void deselect() = 0;
    
};

class FlashSPI: public FlashClass {
private:
//  uint8_t nss;
    inline void select() {
//    digitalWrite(nss,LOW);
    }

    inline void send(uint8_t x, bool deselect = false) {
        spi4teensy3::send(x, deselect);
    }

    inline uint8_t receive(uint8_t x, bool deselect = false) {
        return spi4teensy3::receive(deselect);
    }

    inline void deselect() {
//    digitalWrite(nss,HIGH);
    }

public:
//bool begin(partNumber _partno = autoDetect,uint8_t _nss = SS);
    bool begin(partNumber _partno = autoDetect);
    void end();
};

#endif
