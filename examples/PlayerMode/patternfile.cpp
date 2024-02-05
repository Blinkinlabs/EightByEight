#include "patternfile.h"

// TODO: Handle this at a higher level?
extern volatile bool fileAccessLocked;
extern volatile bool reloadAnimations;


uint32_t fileToUInt32(File* file)
{
    uint32_t value = 0;

    uint8_t byte;
    for(int i = 3; i > -1; i--) {
        file->read(&byte, 1);
        value += byte << (i*8);
    }

    return value;
}

int PatternFile::open(String filename)
{
    if(fileAccessLocked) {
        return ERROR_READ_ERROR;
    }

    if(!SPIFFS.exists(filename)) {
        return ERROR_BAD_FILENAME;
    }
    file = SPIFFS.open(filename, "r");
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
    uint32_t fileVersion = fileToUInt32(&file);
    if(fileVersion != SUPPORTED_VERSION) {
        return ERROR_UNSUPPORTED_VERSION;
    }

    // Test encoding
    encoding = fileToUInt32(&file);
//    if((encoding != ENCODING_RGB24)
//            && (encoding != ENCODING_RGB565_RLE)) {
    if((encoding != ENCODING_RGB24)) {
        return ERROR_UNSUPPORTED_ENCODING;
    }

    // Read in led count
    ledCount = fileToUInt32(&file);

    // Test that we have at least one frame
    frameCount = fileToUInt32(&file);
    if(frameCount == 0) {
        return ERROR_NO_FRAMES;
    }

    // Read in frame delay
    frameDelay = fileToUInt32(&file);

    // Test frameDataSize
    uint32_t frameDataSize = fileToUInt32(&file);
    if(file.size() != (PATTERN_HEADER_LENGTH + frameDataSize)) {
        return ERROR_BAD_FRAME_DATA_SIZE;
    }

    reset();

    return ERROR_NO_ERROR;
}

void PatternFile::reset() {
    frameIndex = 0;
    file.seek(PATTERN_HEADER_LENGTH, SeekMode::SeekSet);
    done = false;
}

bool PatternFile::getDone() {
  const bool isDone = done;
  done = false;
  return isDone;
}

int PatternFile::draw(uint8_t *frame)
{
    if(fileAccessLocked) {
        return ERROR_READ_ERROR;
    }

    if(encoding == ENCODING_RGB24) {
        const uint32_t frameSize = ledCount*3;
        if (frameSize != file.read(frame, frameSize)) {
            return ERROR_READ_ERROR;
        }

        frameIndex++;
        if(frameIndex == frameCount) {
            reset();
            done = true;
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
