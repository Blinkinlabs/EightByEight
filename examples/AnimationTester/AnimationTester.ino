  /** \file
 * Combine multiple demos for the Two Sigma / Blinklabs EightByEight badge.
 *
 * The button is used to cycle between the different demos.
 * 
 * Pixels and SerialReceiver are not really seperate demos; we switch to them
 * automatically if they tell us that they have received a packet.
 * If we go more than ten seconds without a packet, the current demo
 * is restarted.
 */

#include "FS.h"

#include "Badge.h"
#include "patternfile.h"

Badge badge;

#define SERIAL_DELAY 5

void listMemoryInfo()
{
    char buff[64];

    FSInfo fs_info;
    if(!SPIFFS.info(fs_info)) {
        Serial.println("Failed to get fs_info");
        return;
    }

    snprintf(buff, sizeof(buff),
                  "total bytes: %u"
                  "\nused bytes: %u"
                  "\nblockSize: %u",
        fs_info.totalBytes,
        fs_info.usedBytes,
        fs_info.blockSize
        );
    Serial.println(buff);
    delay(SERIAL_DELAY); // workaround for buggy USB-serial

    snprintf(buff, sizeof(buff),
                  "pageSize: %u"
                  "\nmaxOpenFiles: %u"
                  "\nmaxPathLength: %u",
        fs_info.pageSize,
        fs_info.maxOpenFiles,
        fs_info.maxPathLength
        );
    Serial.println(buff);
    delay(SERIAL_DELAY); // workaround for buggy USB-serial
}

bool beginTest() {
    if(!SPIFFS.begin())    // Start the filesystem
        return false;

    if(!SPIFFS.format())   // (Slow) Format the filesystem
        return false;
}

bool noPatternsTest()
{
    // Test that there are no patterns
    Dir dir = SPIFFS.openDir("/");
    return !dir.next();
}

bool badFileNameTest() {
    // Try to load a non-existant file
    PatternFile patternFile;
    return (patternFile.open("asfuasdauecaesbc") == ERROR_BAD_FILENAME);
}

bool badMagicTest() {
    char header[] = "ABC";

    File f = SPIFFS.open("/p/badheader","w");
    f.write((uint8_t*)header, 3);
    f.close();

    PatternFile patternFile;
    return (patternFile.open("badheader") == ERROR_BAD_MAGIC);
}

bool badVersionTest() {
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION + 1;

    File f = SPIFFS.open("/p/badversion","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.close();

    PatternFile patternFile;
    return (patternFile.open("badversion") == ERROR_UNSUPPORTED_VERSION);
}

bool badEncodingTest() {
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = 99;

    File f = SPIFFS.open("/p/badencoding","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.close();

    PatternFile patternFile;
    return (patternFile.open("badencoding") == ERROR_UNSUPPORTED_ENCODING);
}

bool noFramesTest() {
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 0x10203040;
    const uint32_t frameCount = 0;

    File f = SPIFFS.open("/p/noframes","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.close();

    PatternFile patternFile;
    return (patternFile.open("noframes") == ERROR_NO_FRAMES);
}

bool badFrameDataSizeTest() {
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 0x10203040;
    const uint32_t frameCount = 0x50607080;
    const uint32_t frameDelay = 0x90A0B0C0;
    const uint32_t frameDataSize = 1;

    File f = SPIFFS.open("/p/badframedatasize","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.write((uint8_t*)(&frameDelay), 4);
    f.write((uint8_t*)(&frameDataSize), 4);
    f.close();

    PatternFile patternFile;

    return (patternFile.open("badframedatasize") == ERROR_BAD_FRAME_DATA_SIZE);

}

bool validPatternHeaderTest() {
    String name = "patternheadertest";
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 0x10203040;
    const uint32_t frameCount = 0x50607080;
    const uint32_t frameDelay = 0x90A0B0C0;
    const uint32_t frameDataSize = 12;

    File f = SPIFFS.open("/p/patternheadertest","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.write((uint8_t*)(&frameDelay), 4);
    f.write((uint8_t*)(&frameDataSize), 4);

    for(uint32_t i = 0; i < frameDataSize; i++) {
        if(f.write(0) != 1) {
            return false;
        }
    }

    f.close();

    PatternFile patternFile;

    return (patternFile.open(name) == ERROR_NO_ERROR)
            && (encoding == patternFile.getEncoding())
            && (ledCount == patternFile.getLedCount())
            && (frameCount == patternFile.getFrameCount())
            && (frameDelay == patternFile.getFrameDelay())
            && (name.equals(patternFile.getName()))
            && (0 == patternFile.getFrameIndex());
}

bool rgb24TooShortFrameTest() {
    String name = "rgb24shortframe";
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 100;
    const uint32_t frameCount = 0x50607080;
    const uint32_t frameDelay = 0x90A0B0C0;
    const uint32_t frameDataSize = (ledCount*3 - 1);

    File f = SPIFFS.open("/p/rgb24shortframe","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.write((uint8_t*)(&frameDelay), 4);
    f.write((uint8_t*)(&frameDataSize), 4);

    for(uint32_t i = 0; i < frameDataSize; i++) {
        if(f.write(0) != 1) {
            return false;
        }
    }

    f.close();

    PatternFile patternFile;

    if(patternFile.open(name) != ERROR_NO_ERROR) {
        return false;
    }

    uint8_t buffer[ledCount*3];
    return (patternFile.draw(buffer) == ERROR_READ_ERROR);
}

bool rgb24FramesDataTest() {
    String name = "rgb24framesdata";
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 64;
    const uint32_t frameCount = 500;
    const uint32_t frameDelay = 0x90A0B0C0;
    const uint32_t frameDataSize = frameCount*ledCount*3;

    File f = SPIFFS.open("/p/rgb24framesdata","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.write((uint8_t*)(&frameDelay), 4);
    f.write((uint8_t*)(&frameDataSize), 4);

    for(uint32_t frame = 0; frame < frameCount; frame++) {
        for(uint32_t i = 0; i < ledCount*3; i++) {
            if(f.write((frame + i)%256) != 1) {
                return false;
            }
        }
    }

    f.close();

    PatternFile patternFile;

    if(patternFile.open(name) != ERROR_NO_ERROR) {
        return false;
    }

    uint8_t buffer[ledCount*3];
    for(uint32_t frame = 0; frame < frameCount; frame++) {
        if(patternFile.getFrameIndex() != frame) {
            return false;
        }

        if(patternFile.draw(buffer) != ERROR_NO_ERROR) {
            return false;
        }

        for(uint32_t i = 0; i < ledCount*3; i++) {
            if(buffer[i] != (frame + i)%256) {
                return false;
            }
        }
    }

    return true;
}

bool rgb24FrameIndexWrapsTest() {
    String name = "rgb24indexwrapsdata";
    const char header[] = "PAT";
    const uint32_t version = SUPPORTED_VERSION;
    const uint32_t encoding = ENCODING_RGB24;
    const uint32_t ledCount = 64;
    const uint32_t frameCount = 111;
    const uint32_t frameDelay = 0x90A0B0C0;
    const uint32_t frameDataSize = frameCount*ledCount*3;

    File f = SPIFFS.open("/p/rgb24indexwrapsdata","w");
    f.write((uint8_t*)header, 3);
    f.write((uint8_t*)(&version), 4);
    f.write((uint8_t*)(&encoding), 4);
    f.write((uint8_t*)(&ledCount), 4);
    f.write((uint8_t*)(&frameCount), 4);
    f.write((uint8_t*)(&frameDelay), 4);
    f.write((uint8_t*)(&frameDataSize), 4);

    for(uint32_t frame = 0; frame < frameCount; frame++) {
        for(uint32_t i = 0; i < ledCount*3; i++) {
            if(f.write((frame + i)%256) != 1) {
                return false;
            }
        }
    }

    f.close();

    PatternFile patternFile;

    if(patternFile.open(name) != ERROR_NO_ERROR) {
        return false;
    }

    uint8_t buffer[ledCount*3];

    // Loop through the data multiple times to show that the pattern index resets.
    for(uint32_t loop = 0; loop < 4; loop++) {
        for(uint32_t frame = 0; frame < frameCount; frame++) {
            if(patternFile.getFrameIndex() != frame) {
                return false;
            }

            if(patternFile.draw(buffer) != ERROR_NO_ERROR) {
                return false;
            }

            for(uint32_t i = 0; i < ledCount*3; i++) {
                if(buffer[i] != (frame + i)%256) {
                    return false;
                }
            }
        }
    }

    return true;
}

struct UnitTest {
    char* name;
    bool (*function)();
};

UnitTest unitTests[] {
    {"begin", beginTest},
    {"no patterns", noPatternsTest},
    {"bad filename", badFileNameTest},
    {"bad magic", badMagicTest},
    {"bad version", badVersionTest},
    {"bad encoding", badEncodingTest},
    {"valid pattern header", validPatternHeaderTest},
    {"RGB24 frame short", rgb24TooShortFrameTest},
    {"RGB24 frame data test", rgb24FramesDataTest},
    {"RGB24 frame index wraps test", rgb24FrameIndexWrapsTest},
    {"", NULL}
};

void fsTest()
{
    int passed = 0;
    int failed = 0;

    for(UnitTest *unitTest = unitTests;; unitTest++) {
        if(unitTest->name == "") {
            break;
        }

        Serial.print("Running test: ");
        Serial.println(unitTest->name);
        delay(SERIAL_DELAY);

        if(unitTest->function()) {
            Serial.println("...passed");
            passed++;
        } else {
            Serial.println("...failed");
            failed++;
        }
    }

    Serial.print("Tests finished, passed=");
    Serial.print(passed);
    Serial.print(", failed=");
    Serial.println(failed);
    delay(SERIAL_DELAY);
}

void setup()
{
	badge.begin();

	WiFi.persistent(false);
}


void loop()
{
  if (badge.poll())
  {
  }
  
	if (badge.button_edge())
    {
        fsTest();
    }
}


