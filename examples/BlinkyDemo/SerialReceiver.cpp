#include "Arduino.h"
#include "SerialReceiver.h"

#include "FS.h"


#define FIRMWARE_VERSION 0x00000001

#define COMMAND_FORMAT_FILESYTEM 0x10
#define COMMAND_OPEN_FILE 0x11
#define COMMAND_WRITE 0x12
#define COMMAND_READ 0x13
#define COMMAND_CLOSE_FILE 0x14
#define COMMAND_LOCK_FILE_ACCESS 0x20
#define COMMAND_UNLOCK_FILE_ACCESS 0x21
#define COMMAND_GET_FIRMWARE_VERSION 0x30

extern bool fileAccessLocked;
extern bool reloadAnimations;

File file;

void SerialReceiver::begin() {
    reset();
}

void SerialReceiver::tapped() {
}

void SerialReceiver::reset() {
    serialMode = SERIAL_MODE_DATA;

    bufferIndex = 0;
    pixelIndex = 0;

    escapeRunCount = 0;

    commandBufferIndex = 0;
}

bool SerialReceiver::step(float ax, float ay, float az) {
    while (Serial.available()) {
        switch (serialMode) {
        case SERIAL_MODE_DATA:
            if(readData()) {
                return true;
            }
            break;
        case SERIAL_MODE_COMMAND:
            readCommand();
            break;
        default:
            reset();
        }
    }

    return false;
}

void SerialReceiver::draw(RGBMatrix &matrix) {
    matrix.set(data);
}

bool SerialReceiver::readData() {
    bool canDraw = false;

    uint8_t c = Serial.read();

    // Pixel character
    if (c != 0xFF) {
        // Reset the control character state variables
        escapeRunCount = 0;

        // Copy this byte into the pixel array
        // TODO: Copy directly into the buffer
        // Buffer the color
        buffer[bufferIndex++] = c;

        // If this makes a complete pixel color, update the display and reset for the next color
        if (bufferIndex > 2) {
            bufferIndex = 0;

            // Prevent overflow by ignoring any pixel data beyond LED_COUNT
            if (pixelIndex < LED_ROWS * LED_COLS) {
                data[pixelIndex * LED_BYTES_PER_PIXEL + 0] = buffer[0];
                data[pixelIndex * LED_BYTES_PER_PIXEL + 1] = buffer[1];
                data[pixelIndex * LED_BYTES_PER_PIXEL + 2] = buffer[2];

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
        if (escapeRunCount == 1) {
            canDraw = true;
        }

        if (escapeRunCount > 8) {
            serialMode = SERIAL_MODE_COMMAND;
            commandBufferIndex = 0;
        }
    }

    return canDraw;
}


uint8_t commandFormatFilesystem(uint8_t &length, uint8_t *buffer) {
    length = 0;

    if((buffer[0] != 'E') || (buffer[1] != 'e')) {
        return 1;
    }

    // Format will automatically un-mount, then re-mount the filesystem.
    // (esp8266/spiffs_api.h)
    if(!SPIFFS.format()) {
        return 2;
    }

    return 0;
}

uint8_t commandOpenFile(uint8_t &length, uint8_t *buffer) {
    length = 0;

    // TODO: Test that buffer[1] is null terminated

    char fileMode[5];
    if( buffer[0] == FILEMODE_READ) {
        sprintf(fileMode, "r");
    }
    else if(buffer[0] == FILEMODE_WRITE) {
        sprintf(fileMode, "w");
    }
    else {
        return 1;
    }

    file = SPIFFS.open((char *)buffer + 1, fileMode);
    if(!file) {
        return 2;
    }

    return 0;
}

uint8_t commandWrite(uint8_t &length, uint8_t *buffer) {
    const uint8_t requestedLength = length;
    const uint8_t actualLength = file.write(buffer, requestedLength);

    length = 1;
    buffer[0] = actualLength;
    if (requestedLength == actualLength) {
        return 0;
    } else {
        return 1;
    }
}

uint8_t commandRead(uint8_t &length, uint8_t *buffer) {
    // TODO: Test that the length is 1

    const uint8_t requestedLength = buffer[0];
    const uint8_t actualLength = file.read(buffer, requestedLength);

    length = actualLength;
    if (requestedLength == actualLength) {
        return 0;
    } else {
        return 1;
    }
}

uint8_t commandCloseFile(uint8_t &length, uint8_t *buffer) {
    length = 0;

    // TODO: Anything to check here?
    file.close();

    return 0;
}

uint8_t commandLockFileAccess(uint8_t &length, uint8_t *buffer) {
    length = 0;

    // Set a magic flag that tells the device not to access the filesystem
    fileAccessLocked = true;

    return 0;
}

uint8_t commandUnlockFileAccess(uint8_t &length, uint8_t *buffer) {
    length = 0;

    // Unset a magic flag that tells the device not to access the filesystem
    fileAccessLocked = false;

    // And set another magic flag to tell the device to reload the pattern table
    reloadAnimations = true;

    return 0;
}

uint8_t commandGetFirmwareVersion(uint8_t &length, uint8_t *buffer) {
    length = 4;

    // Send the protocol version, in big-endian format
    buffer[0] = (FIRMWARE_VERSION >> 24) & 0xFF;
    buffer[1] = (FIRMWARE_VERSION >> 16) & 0xFF;
    buffer[2] = (FIRMWARE_VERSION >> 8) & 0xFF;
    buffer[3] = (FIRMWARE_VERSION >> 0) & 0xFF;

    return 0;
}

struct Command {
    uint8_t name;   // Command identifier
    uint8_t (*function)(uint8_t &, uint8_t *);
};

Command commands[] = {
    {COMMAND_FORMAT_FILESYTEM, commandFormatFilesystem},
    {COMMAND_OPEN_FILE, commandOpenFile},
    {COMMAND_WRITE, commandWrite},
    {COMMAND_READ, commandRead},
    {COMMAND_CLOSE_FILE, commandCloseFile},
    {COMMAND_LOCK_FILE_ACCESS, commandLockFileAccess},
    {COMMAND_UNLOCK_FILE_ACCESS, commandUnlockFileAccess},
    {COMMAND_GET_FIRMWARE_VERSION, commandGetFirmwareVersion},
    {0xFF,   NULL}
};



void SerialReceiver::readCommand() {
    uint8_t c = Serial.read();

    // Filter out any extraneous 0xFF's before a command
    if((commandBufferIndex == 0) && (c == 0xFF))
        return;

    commandBuffer[commandBufferIndex++] = c;

    // All commands have the format:
    // (command field) (1 byte)
    // (data length) (1 byte)
    // (data) (0-256 bytes)

    // And all responses have the format:
    // (result) (1 byte) (0 for no error, positive for error)
    // (return data length) (1 byte)
    // (return data) (0-256 bytes)

    // Return if we haven't read the length field yet
    if(commandBufferIndex < 2) {
        return;
    }

    // Return if we haven't read the data length in yet
    if(commandBufferIndex < 2 + commandBuffer[1]) {
        return;
    }

    // Check if we have a valid command
    Command *command = commands;
    while (command->name != commandBuffer[0]) {

        // If we reached the end and didn't find anything, bail
        if(command->name == 0xFF) {
            Serial.write(char(0x01));  // Error
            Serial.write(char(0x00));  // 0 bytes data
            reset();
            return;
        }

        command++;
    }

    // Run the command, then write out the results
    Serial.write(command->function(commandBuffer[1], &commandBuffer[2]));
    Serial.write(commandBuffer[1]);
    Serial.write(&commandBuffer[2], commandBuffer[1]);
    reset();
}
