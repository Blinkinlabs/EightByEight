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

#pragma once
#include <stdbool.h>
#include <stdint.h>

#define RAM_FUNCTION __attribute__( ( long_call, section(".data#") ) ) 


typedef enum {
    appIDLE = 0,
    appDETACH,
    dfuIDLE,
    dfuDNLOAD_SYNC,
    dfuDNBUSY,
    dfuDNLOAD_IDLE,
    dfuMANIFEST_SYNC,
    dfuMANIFEST,
    dfuMANIFEST_WAIT_RESET,
    dfuUPLOAD_IDLE,
    dfuERROR
} dfu_state_t;

typedef enum {
    OK = 0,
    errTARGET,
    errFILE,
    errWRITE,
    errERASE,
    errCHECK_ERASED,
    errPROG,
    errVERIFY,
    errADDRESS,
    errNOTDONE,
    errFIRMWARE,
    errVENDOR,
    errUSBR,
    errPOR,
    errUNKNOWN,
    errSTALLEDPKT,
} dfu_status_t;

#define DFU_TRANSFER_SIZE         1024      // Flash sector size


#ifdef __cplusplus
extern "C" {
#endif

// USB entry points. True on success, false for stall.
bool dfu_getstatus(uint8_t *status);
bool dfu_clrstatus();
bool dfu_abort();
bool dfu_upload(unsigned offset, unsigned length, const uint8_t *data);
bool dfu_download(unsigned blockNum, unsigned blockLength,
    unsigned packetOffset, unsigned packetLength, const uint8_t *data);

// Sideways entrance?
void fl_state_poll();

#ifdef __cplusplus
}
#endif
