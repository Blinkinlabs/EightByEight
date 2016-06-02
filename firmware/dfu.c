/*
 * Fadecandy DFU Bootloader
 * 
 * Copyright (c) 2013 Micah Elizabeth Scott
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

#include <stdbool.h>
#include "mk20dn64.h"
#include "usb_dev.h"
#include "dfu.h"
#include "eightbyeight.h"

// Internal flash-programming state machine
static unsigned fl_current_addr = 0;
static enum {
    flsIDLE = 0,
    flsERASING,
    flsPROGRAMMING
} fl_state;

static dfu_state_t dfu_state = dfuIDLE;
static dfu_status_t dfu_status = OK;
static unsigned dfu_poll_timeout = 1;
static unsigned dfu_program_index = 0;

static uint8_t dfu_buffer[DFU_TRANSFER_SIZE];

RAM_FUNCTION void *memcpy(void *dst, const void *src, size_t cnt) {
    uint8_t *dst8 = dst;
    const uint8_t *src8 = src;
    while (cnt > 0) {
        cnt--;
        *(dst8++) = *(src8++);
    }
    return dst;
}

RAM_FUNCTION bool ftfl_busy()
{
    // Is the flash memory controller busy?
    return 0 == (FTFL_FSTAT_CCIF & FTFL_FSTAT);
}

RAM_FUNCTION void ftfl_launch_command()
{
    // Begin a flash memory controller command
    FTFL_FSTAT = FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_RDCOLERR;
    FTFL_FSTAT = FTFL_FSTAT_CCIF;
}

RAM_FUNCTION void ftfl_begin_erase_sector(uint32_t address)
{
    FTFL_FCCOB0 = 0x09;
    FTFL_FCCOB1 = address >> 16;
    FTFL_FCCOB2 = address >> 8;
    FTFL_FCCOB3 = address;
    ftfl_launch_command();
}

RAM_FUNCTION void ftfl_begin_program_longword(uint32_t address, uint32_t* longword)
{
    FTFL_FCCOB0 = 0x06;
    FTFL_FCCOB1 = address >> 16;
    FTFL_FCCOB2 = address >> 8;
    FTFL_FCCOB3 = address;
    FTFL_FCCOB4 = (*longword) >> 24;
    FTFL_FCCOB5 = (*longword) >> 16;
    FTFL_FCCOB6 = (*longword) >> 8;
    FTFL_FCCOB7 = (*longword);
    ftfl_launch_command();
}

RAM_FUNCTION uint32_t address_for_block(unsigned blockNum)
{
    return (uint32_t)ANIMATION_DATA_START + (blockNum << 10);
}

bool dfu_upload(unsigned offset, unsigned length, const uint8_t *data)
{
    uint8_t* address = (uint8_t*)(address_for_block(0) + offset);
    memcpy(address, data, length);

    return true;
}

RAM_FUNCTION bool dfu_download(unsigned blockNum, unsigned blockLength,
    unsigned packetOffset, unsigned packetLength, const uint8_t *data)
{
    if (packetOffset + packetLength > DFU_TRANSFER_SIZE ||
        packetOffset + packetLength > blockLength) {

        // Overflow!
        dfu_state = dfuERROR;
        dfu_status = errADDRESS;
        return false;
    }

    // Store more data...
    memcpy(dfu_buffer + packetOffset, data, packetLength);

    if (packetOffset + packetLength != blockLength) {
        // Still waiting for more data.
        return true;
    }

    if (dfu_state != dfuIDLE && dfu_state != dfuDNLOAD_IDLE) {
        // Wrong state! Oops.
        dfu_state = dfuERROR;
        dfu_status = errSTALLEDPKT;
        return false;
    }

    if (ftfl_busy() || fl_state != flsIDLE) {
        // Flash controller shouldn't be busy now!
        dfu_state = dfuERROR;
        dfu_status = errUNKNOWN;
        return false;       
    }

    if (!blockLength) {
        // End of download
        dfu_state = dfuMANIFEST_SYNC;
        dfu_status = OK;
        return true;
    }

    // Start programming a block by erasing the corresponding flash sector
    fl_state = flsERASING;
    fl_current_addr = address_for_block(blockNum);
    ftfl_begin_erase_sector(fl_current_addr);

    dfu_state = dfuDNLOAD_SYNC;
    dfu_status = OK;
    return true;
}

RAM_FUNCTION bool fl_handle_status(uint8_t fstat, unsigned specificError)
{
    /*
     * Handle common errors from an FSTAT register value.
     * The indicated "specificError" is used for reporting a command-specific
     * error from MGSTAT0.
     *
     * Returns true if handled, false if not.
     */


    if (0 == (fstat & FTFL_FSTAT_CCIF)) {
        // Still working...
        return true;
    }

    if (fstat & FTFL_FSTAT_RDCOLERR) {
        // Bus collision. We did something wrong internally.
        dfu_state = dfuERROR;
        dfu_status = errUNKNOWN;
        fl_state = flsIDLE;
        return true;
    }

    if (fstat & (FTFL_FSTAT_FPVIOL | FTFL_FSTAT_ACCERR)) {
        // Address or protection error
        dfu_state = dfuERROR;
        dfu_status = errADDRESS;
        fl_state = flsIDLE;
        return true;
    }

    if (fstat & FTFL_FSTAT_MGSTAT0) {
        // Command-specifid error
        dfu_state = dfuERROR;
        dfu_status = specificError;
        fl_state = flsIDLE;
        return true;
    }

    return false;
}

RAM_FUNCTION void fl_state_poll()
{
    // Try to advance the state of our own flash programming state machine.

    uint8_t fstat = FTFL_FSTAT;
    switch (fl_state) {

        case flsIDLE:
            break;

        case flsERASING:
            if (!fl_handle_status(fstat, errERASE)) {
                // Done! Move on to programming the sector.
                fl_state = flsPROGRAMMING;
                // ftfl_begin_program_section(fl_current_addr, DFU_TRANSFER_SIZE/4);

                dfu_program_index = 0;
                ftfl_begin_program_longword(
                    fl_current_addr + dfu_program_index,
                    (uint32_t*)(dfu_buffer + dfu_program_index)
                    );
            }
            break;

        case flsPROGRAMMING:
            if (!fl_handle_status(fstat, errVERIFY)) {
                // // Done!
                // fl_state = flsIDLE;

                if (dfu_program_index < DFU_TRANSFER_SIZE) {
                    ftfl_begin_program_longword(
                        fl_current_addr + dfu_program_index,
                        (uint32_t*)(dfu_buffer + dfu_program_index)
                        );
                    dfu_program_index += 4;
                }
                else {
                    fl_state = flsIDLE;
                }
            }
            break;
    }
}

RAM_FUNCTION bool dfu_getstatus(uint8_t *status)
{
    switch (dfu_state) {

        case dfuDNLOAD_SYNC:
        case dfuDNBUSY:
            // Programming operation in progress. Advance our private flash state machine.
            fl_state_poll();

            if (dfu_state == dfuERROR) {
                // An error occurred inside fl_state_poll();
            } else if (fl_state == flsIDLE) {
                dfu_state = dfuDNLOAD_IDLE;
            } else {
                dfu_state = dfuDNBUSY;
            }
            break;

        case dfuMANIFEST_SYNC:
            // Ready to reboot. The main thread will take care of this. Also let the DFU tool
            // know to leave us alone until this happens.
            dfu_state = dfuMANIFEST;
            dfu_poll_timeout = 1000;
            break;

        default:
            break;
    }

    status[0] = dfu_status;
    status[1] = dfu_poll_timeout;
    status[2] = dfu_poll_timeout >> 8;
    status[3] = dfu_poll_timeout >> 16;
    status[4] = dfu_state;
    status[5] = 0;  // iString

    return true;
}

RAM_FUNCTION bool dfu_clrstatus()
{
    switch (dfu_state) {

        case dfuERROR:
            // Clear an error
            dfu_state = dfuIDLE;
            dfu_status = OK;
            return true;

        default:
            // Unexpected request
            dfu_state = dfuERROR;
            dfu_status = errSTALLEDPKT;
            return false;
    }
}

RAM_FUNCTION bool dfu_abort()
{
    dfu_state = dfuIDLE;
    dfu_status = OK;
    return true;
}
