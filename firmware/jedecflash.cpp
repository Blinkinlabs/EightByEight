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


#include "Wprogram.h"
#include <errno.h>
#include "jedecflash.h"

//COMMANDS
#define W_EN    0x06    //write enable
#define W_DE    0x04    //write disable
#define R_SR1   0x05    //read status reg 1
#define W_SR    0x01    //write status reg
#define PAGE_PGM        0x02    //page program
#define BLK_E_64K       0xD8    //block erase 64KB
#define SECTOR_E        0x20    //sector erase 4KB
#define CHIP_ERASE      0xc7    //chip erase
#define CHIP_ERASE2     0x60    //=CHIP_ERASE
#define PDWN            0xb9    //power down
#define RELEASE         0xab    //release power down or HPM/Dev ID (deprecated)
#define R_JEDEC_ID      0x9f    //read JEDEC ID = Manuf+ID (suggested)
#define READ            0x03
#define FAST_READ       0x0b

#define SR1_BUSY_MASK   0x01

typedef struct {
    FlashClass::manufacturerId mfr;
    FlashClass::partNumber pn;
    uint16_t id;
    uint32_t bytes;
    uint32_t pages;
    uint16_t sectors;
    uint16_t blocks;
} flashListType;


// Note: it's possible we don't care about any of this, and can just read the size
// based on the JEDEC ID fields.
static const flashListType flashPnList[] = {
    { FlashClass::Winbond,  FlashClass::W25Q80,    0x4014, 1048576,  4096,  256,  16},
    { FlashClass::Winbond,  FlashClass::W25Q16,    0x4015, 2097152,  8192,  512,  32},
    { FlashClass::Winbond,  FlashClass::W25Q32,    0x4016, 4194304, 16384, 1024,  64},
    { FlashClass::Winbond,  FlashClass::W25Q64,    0x4017, 8388608, 32768, 2048, 128},
    { FlashClass::Winbond,  FlashClass::W25Q128,   0x4018,16777216, 65536, 4096, 256},
    { FlashClass::Spansion, FlashClass::S25FL208K, 0x4014, 1048576,  4096,  256,  16},
    { FlashClass::Spansion, FlashClass::S25FL216K, 0x4015, 2097152,  8192,  512,  32},
};
    
uint16_t FlashClass::readSR()
{
    uint8_t r1;
    select();
    send(R_SR1);
    r1 = receive(0xff, true);
    deselect();
    deselect();//some delay
    select();
    deselect();
    return r1;
}

uint8_t FlashClass::readManufacturer()
{
    uint8_t c;
    select();
    send(R_JEDEC_ID);
    c = receive(0x00);
    receive(0x00);
    receive(0x00, true);
    deselect();
    return c;
}

uint16_t FlashClass::readPartID()
{
    uint8_t a,b;
    select();
    send(R_JEDEC_ID);
    send(0x00);
    a = receive(0x00);
    b = receive(0x00, true);
    deselect();
    return (a<<8)|b;
}

bool FlashClass::checkPartNo(partNumber _partno)
{
    uint8_t manuf;
    uint16_t id;
    
    select();
    send(R_JEDEC_ID);
    manuf = receive(0x00);
    id = receive(0x00) << 8;
    id |= receive(0x00, true);
    deselect();

    // If it's autodetect mode, search the list of known parts to find a vendor/id match
    if(_partno == autoDetect)
    {
        for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
        {
            if((manuf == flashPnList[i].mfr) && (id == flashPnList[i].id))
            {
                partno = flashPnList[i].pn;
                return true;
            }
        }
    }
    else {
        // If a specific part id was given, test that the installed part matches vendor/id.
        for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
        {
            if(_partno == flashPnList[i].pn)
            {
                if((manuf == flashPnList[i].mfr) && (id == flashPnList[i].id)) {
                    partno = _partno;
                    return true;
                }

                return false;
            }
        }
    }

    return false;//partNo not found
}

bool FlashClass::busy()
{
    uint8_t r1;
    select();
    send(R_SR1);
    r1 = receive(0xff, true);
    deselect();
    if(r1 & SR1_BUSY_MASK)
        return true;
    return false;
}

void FlashClass::setWriteEnable(bool status)
{
    select();
    send( status ? W_EN : W_DE, true);
    deselect();
}

long FlashClass::bytes()
{
    for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
    {
        if(partno == flashPnList[i].pn)
        {
            return flashPnList[i].bytes;
        }
    }
    return 0;
}

uint16_t FlashClass::pages()
{
    for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
    {
        if(partno == flashPnList[i].pn)
        {
            return flashPnList[i].pages;
        }
    }
    return 0;
}

uint16_t FlashClass::sectors()
{
    for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
    {
        if(partno == flashPnList[i].pn)
        {
            return flashPnList[i].sectors;
        }
    }
    return 0;
}

uint16_t FlashClass::blocks()
{
    for(int i=0;i<sizeof(flashPnList)/sizeof(flashPnList[0]);i++)
    {
        if(partno == flashPnList[i].pn)
        {
            return flashPnList[i].blocks;
        }
    }
    return 0;
}

bool FlashClass::begin(partNumber _partno)
{
    select();
    send(RELEASE, true);
    deselect();
    delayMicroseconds(5);  //>3us
    
    if(!checkPartNo(_partno))
        return false;

    return true;
}

void FlashClass::end()
{
    select();
    send(PDWN, true);
    deselect();
    delayMicroseconds(5);  //>3us
}

uint16_t FlashClass::read(uint32_t address,uint8_t *buffer,uint16_t n)
{
    if(busy())
        return 0;
    
    select();
    send(READ);
    send(address>>16);
    send(address>>8);
    send(address);
    for(uint16_t i=0;i<n;i++)
    {
        buffer[i] = receive(0x00, i==n-1);
    }
    deselect();
    
    return n;
}

void FlashClass::writePage(uint32_t address_start,uint8_t *buffer)
{
    select();
    send(PAGE_PGM);
    send(address_start>>16);
    send(address_start>>8);
    send(0x00);
    uint8_t i=0;
    do {
        send(buffer[i], i==255);
        i++;
    }while(i!=0);
    deselect();
}

void FlashClass::eraseSector(uint32_t address_start)
{
    select();
    send(SECTOR_E);
    send(address_start>>16);
    send(address_start>>8);
    send(address_start, true);
    deselect();
}

void FlashClass::erase64kBlock(uint32_t address_start)
{
    select();
    send(BLK_E_64K);
    send(address_start>>16);
    send(address_start>>8);
    send(address_start, true);
    deselect();
}

void FlashClass::eraseAll()
{
    select();
    send(CHIP_ERASE, true);
    deselect();
}

//bool FlashSPI::begin(partNumber _partno, uint8_t _nss)
bool FlashSPI::begin(partNumber _partno)
{
//  nss = _nss;

    spi4teensy3::init();
//    spi4teensy3::init();

//  pinMode(nss, OUTPUT);
    deselect();

    return FlashClass::begin(_partno);
}

void FlashSPI::end()
{
    FlashClass::end();
}

