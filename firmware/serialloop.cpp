#include "WProgram.h"
#include "eightbyeight.h"
#include "serialloop.h"
#include "usb_serial.h"
#include "matrix.h"
#include "dfu.h"
#include <stdlib.h>
#include <stdio.h>

#define SERIAL_MODE_DATA     0x01
#define SERIAL_MODE_COMMAND  0x02

extern Matrix matrix;

//extern bool reloadAnimations;

// We start in BlinkyTape mode, but if we get the magic escape sequence, we transition
// to BlinkyTile mode.
// Escape sequence is 10 0xFF characters in a row.

int serialMode;         // Serial protocol we are speaking

///// Defines for the data mode
void dataLoop();

int escapeRunCount;     // Count of how many escape characters we've received
uint8_t buffer[3];      // Buffer for one pixel of data
int bufferIndex;        // Current location in the buffer
int pixelIndex;         // Pixel we are currently writing to

///// Defines for the control mode
void commandLoop();

#define CONTROL_BUFFER_SIZE 100
uint8_t controlBuffer[CONTROL_BUFFER_SIZE];     // Buffer for receiving command data
int controlBufferIndex;     // Current location in the buffer

void serialReset() {
    serialMode = SERIAL_MODE_DATA;

    bufferIndex = 0;
    pixelIndex = 0;

    escapeRunCount = 0;

    controlBufferIndex = 0;
}

void serialLoop() {
    switch(serialMode) {
        case SERIAL_MODE_DATA:
            dataLoop();
            break;
        case SERIAL_MODE_COMMAND:
            commandLoop();
            break;
        default:
            serialReset();
    }
}

void dataLoop() {
    uint8_t c = usb_serial_getchar();

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
            if(pixelIndex < LED_COUNT) {
                matrix.setPixelColor(pixelIndex%LED_COLS, pixelIndex/LED_COLS,
                         buffer[0], buffer[1], buffer[2]);
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
            //memcpy(getPixels(), frameData, LED_ROWS*LED_COLS*BYTES_PER_PIXEL);
            matrix.show();
        }
        
        if(escapeRunCount > 8) {
            serialMode = SERIAL_MODE_COMMAND;
            controlBufferIndex = 0;
        }
    }
}

bool commandStartWrite(uint8_t* buffer);
bool commandWrite(uint8_t* buffer);
bool commandStopWrite(uint8_t* buffer);
//bool commandStartRead(uint8_t* buffer);
//bool commandRead(uint8_t* buffer);
//bool commandStopRead(uint8_t* buffer);

struct Command {
    uint8_t name;   // Command identifier
    int length;     // Command length (number of bytes to read)
    bool (*function)(uint8_t*);
};

Command commands[] = {
    {0x01,   1,   commandStartWrite},   // Start writing an animation
    {0x02,   65,  commandWrite},        // Write 64 bytes of data
    {0x03,   1,   commandStopWrite},    // Stop writing
//    {0x04,   1,   commandStartRead},    // Start reading back the animation
//    {0x05,   1,   commandRead},         // Read 64 bytes of data
//    {0x06,   1,   commandStopRead},     // Stop reading
    {0xFF,   0,   NULL}
};


void commandLoop() {
    uint8_t c = usb_serial_getchar();

    // If we get extra 0xFF bytes before the command byte, ignore them
    if((controlBufferIndex == 0) && (c == 0xFF))
        return;

    controlBuffer[controlBufferIndex++] = c;

    for(Command *command = commands; 1; command++) {
        // If the command isn't found in the list, bail
        if(command->name == 0xFF) {
            serialReset();
            break;
        }

        // If this iteration isn't the correct one, keep looking
        if(command->name != controlBuffer[0])
            continue;

        // Now we're on to something- have we gotten enough data though?
        if(controlBufferIndex >= command->length) {
            if(command->function(controlBuffer + 1)) {
                usb_serial_putchar('P');
                usb_serial_putchar((char)controlBuffer[1]);
                usb_serial_write(controlBuffer + 2, controlBuffer[1] + 1);
            }
            else {
                usb_serial_putchar('F');
                usb_serial_putchar((char)controlBuffer[1]);
                usb_serial_write(controlBuffer + 2, controlBuffer[1] + 1);
            }

            serialReset();
        }
        break;
    }
}

static bool writing = false;
static int packetCount;         // Count of packets we have written so far


#define BYTES_PER_PACKET 64
#define PACKETS_PER_BLOCK (DFU_TRANSFER_SIZE / BYTES_PER_PACKET)

bool commandStartWrite(uint8_t* buffer) {
    // Reset the write state machine
    writing = true;
    packetCount = 0;

    buffer[0] = 0;
    return true;
}

// TODO: Cut down the critical sections here.
RAM_FUNCTION bool doWrite(uint8_t* buffer, int blockNum, int blockLength, int packetOffset, int packetLength) {
    bool result = false;
    buffer[0] = 0;

    __disable_irq();

    if(!dfu_download(blockNum,
                    blockLength,
                    packetOffset,
                    packetLength,
                    buffer)) {

        writing = false;
        goto FAIL;
    }

    packetCount++;

    uint8_t status[6];
    do {
        dfu_getstatus(status);

        if(status[0] != OK) {
            buffer[0] = 6-1;
            buffer[1] = status[0];
            buffer[2] = FTFL_FPROT3;
            buffer[3] = FTFL_FPROT2;
            buffer[4] = FTFL_FPROT1;
            buffer[5] = FTFL_FPROT0;
            buffer[6] = FTFL_FDPROT;
            goto FAIL;
        }
    }
    while((status[4] != dfuDNLOAD_IDLE) &&
          (status[4] != dfuIDLE));

    result = true;

FAIL:
    __enable_irq();
    return result;
}

bool commandWrite(uint8_t* buffer) {
    if(!writing) {
        buffer[0] = 0;
        return false;
    }

    int blockNum = packetCount / PACKETS_PER_BLOCK;
    int blockLength = DFU_TRANSFER_SIZE;
    int packetOffset = ((packetCount % PACKETS_PER_BLOCK) * BYTES_PER_PACKET);
    int packetLength = BYTES_PER_PACKET;

    bool result = doWrite(buffer, blockNum, blockLength, packetOffset, packetLength);

    return result;
}

bool commandStopWrite(uint8_t* buffer) {
    writing = false;

    //reloadAnimations = true;

    buffer[0] = 0;
    return true;
}

/*

static bool reading = false;
static int readPacketCount;         // Count of packets we have written so far

bool commandStartRead(uint8_t* buffer) {
    // Reset the write state machine
    reading = true;
    readPacketCount = 0;

    buffer[0] = 0;
    return true;
}

bool commandRead(uint8_t* buffer) {
    if(!reading) {
        buffer[0] = 0;
        return false;
    }

    // TODO: Fixme
    //dfu_upload(readPacketCount*BYTES_PER_PACKET, BYTES_PER_PACKET, buffer+1);
    //buffer[0] = BYTES_PER_PACKET;

    buffer[0]=0;
    return true;
}

bool commandStopRead(uint8_t* buffer) {
    reading = false;

    buffer[0] = 0;
    return true;
}
*/
