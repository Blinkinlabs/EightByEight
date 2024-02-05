#ifndef PATTERN_FILE_H
#define PATTERN_FILE_H

#include "Arduino.h"
#include "FS.h"

// Pattern file format (version 0):
// Offset 0-2: char[3] 'PAT' (magic header)
// Offset 3: uint32_t fileVersion (1)
// Offset 7: uint32_t encoding
// Offset 11: uint32_t ledCount (leds)
// Offset 15: uint32_t frameCount (frames)
// Offset 19: uint32_t frameDelay (ms)
// Offset 23: uint32_t frameDataSize (bytes)
// Offset 27+: frame data

const char PATTERN_DIRECTORY[] = "/p/";
const uint32_t SUPPORTED_VERSION = 1;

const uint32_t PATTERN_HEADER_LENGTH = 27;

#define ENCODING_RGB24 0           /// RGB24 mode (uncompressed 24 bit)
#define ENCODING_RGB565_RLE 1               /// RGB545_RLE


#define ERROR_NO_ERROR 0
#define ERROR_BAD_FILENAME -1
#define ERROR_READ_ERROR -2
#define ERROR_BAD_MAGIC -3
#define ERROR_UNSUPPORTED_VERSION -4
#define ERROR_UNSUPPORTED_ENCODING -5
#define ERROR_NO_FRAMES -6
#define ERROR_BAD_FRAME_DATA_SIZE -7

uint32_t fileToUInt32(File* file);

class PatternFile
{
public:
    int open(String filename);
    const char *getName() const;

    uint32_t getEncoding() const;
    uint32_t getLedCount() const;
    uint32_t getFrameCount() const;
    uint32_t getFrameDelay() const;

    uint32_t getFrameIndex() const;

    void reset();

    // Draw data into the specified frame
    int draw(uint8_t* frame);

    bool getDone();

private:
    File file;

    uint32_t encoding;
    uint32_t ledCount;
    uint32_t frameCount;
    uint32_t frameDelay;

    uint32_t frameIndex;

    bool done;
};

#endif // PATTERN_FILE_H
