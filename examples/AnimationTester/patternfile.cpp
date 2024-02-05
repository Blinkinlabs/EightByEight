#include "patternfile.h"

int PatternFile::open(String filename)
{
    if(!SPIFFS.exists(PATTERN_DIRECTORY + filename)) {
        return ERROR_BAD_FILENAME;
    }
    file = SPIFFS.open(PATTERN_DIRECTORY + filename, "r");
    if(!file) {
        return ERROR_READ_ERROR;
    }

    // Test header
    char magic[3];
    if(file.read((uint8_t*)magic, 3) != 3) {
        return ERROR_READ_ERROR;
    }
    if(magic[0] != 'P'
            || magic[1] != 'A'
            || magic[2] != 'T') {
        return ERROR_BAD_MAGIC;
    }

    // Test version
    uint32_t fileVersion;
    if(file.read((uint8_t*)(&fileVersion), 4) != 4) {
        return ERROR_READ_ERROR;
    }
    if(fileVersion != SUPPORTED_VERSION) {
        return ERROR_UNSUPPORTED_VERSION;
    }

    // Test encoding
    if(file.read((uint8_t*)(&encoding), 4) != 4) {
        return ERROR_READ_ERROR;
    }
//    if((encoding != ENCODING_RGB24)
//            && (encoding != ENCODING_RGB565_RLE)) {
    if((encoding != ENCODING_RGB24)) {
        return ERROR_UNSUPPORTED_ENCODING;
    }

    // Read in led count
    if(file.read((uint8_t*)(&ledCount), 4) != 4) {
        return ERROR_READ_ERROR;
    }

    // Test that we have at least one frame
    if(file.read((uint8_t*)(&frameCount), 4) != 4) {
        return ERROR_READ_ERROR;
    }
    if(frameCount == 0) {
        return ERROR_NO_FRAMES;
    }

    // Read in frame delay
    if(file.read((uint8_t*)(&frameDelay), 4) != 4) {
        return ERROR_READ_ERROR;
    }

    // Test frameDataSize
    uint32_t frameDataSize;
    if(file.read((uint8_t*)(&frameDataSize), 4) != 4) {
        return ERROR_READ_ERROR;
    }
    if(file.size() != (PATTERN_HEADER_LENGTH + frameDataSize)) {
        return ERROR_BAD_FRAME_DATA_SIZE;
    }

    reset();

    return ERROR_NO_ERROR;
}

void PatternFile::reset() {
    frameIndex = 0;
}

int PatternFile::draw(uint8_t *frame)
{
    if(encoding == ENCODING_RGB24) {
        const uint32_t frameSize = ledCount*3;
        if (frameSize != file.read(frame, frameSize)) {
            return ERROR_READ_ERROR;
        }

        frameIndex++;
        if(frameIndex == frameCount) {
            frameIndex = 0;
            file.seek(PATTERN_HEADER_LENGTH, SeekMode::SeekSet);
        }
        return ERROR_NO_ERROR;
    }

    return ERROR_UNSUPPORTED_ENCODING;
}

const char* PatternFile::getName() const {
    return file.name() + (sizeof(PATTERN_DIRECTORY) -1);
}


uint32_t PatternFile::getEncoding() const {
    return encoding;
}

uint32_t PatternFile::getLedCount() const {
    return ledCount;
}

uint32_t PatternFile::getFrameCount() const {
    return frameCount;
}

uint32_t PatternFile::getFrameDelay() const {
    return frameDelay;
}

uint32_t PatternFile::getFrameIndex() const {
    return frameIndex;
}
