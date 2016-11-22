#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <lwip/udp.h>

#include "matrix.h"

const int listen_port = 1337;

const char ssid[] = "SSID";
const char password[] = "PASSWORD";

char buffer[512]; //buffer to hold incoming and outgoing packets

WiFiUDP Udp;

Matrix matrix;

struct ImageMetaInfo {
  int width;
  int height;
  int range;      // Range of gray-levels. We only handle 255 correctly(=1byte)

  // FlaschenTaschen extensions
  int offset_x;   // display image at this x-offset
  int offset_y;   // .. y-offset
  int layer;      // stacked layer
};


void setup(void)
{
  // USB communication to PC
  Serial.begin ( 460800 );
  WiFi.begin ( ssid, password );  
  Serial.println ( "" );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

  if ( MDNS.begin ( "badge" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  Udp.begin(listen_port);

  matrix.setup();
}

void handleUdpPacket(int noBytes) {
//    Serial.print(millis() / 1000);
//    Serial.print(":Packet of ");
//    Serial.print(noBytes);
//    Serial.print(" received from ");
//    Serial.print(Udp.remoteIP());
//    Serial.print(":");
//    Serial.println(Udp.remotePort());

  // We've received a packet, read the data from it
  Udp.read(buffer, noBytes); // read the packet into the buffer
  
//    // display the packet contents in HEX
//    for (int i=1;i<=noBytes;i++){
//      Serial.print(buffer[i-1],HEX);
//      if (i % 32 == 0){
//        Serial.println();
//      }
//      else Serial.print(' ');
//    } // end for
  
  ImageMetaInfo img_info = {0};
  img_info.width = LED_ROWS;
  img_info.height = LED_COLS;

  uint8_t* matrixData = matrix.getPixels();
  const byte *pixel_pos = (byte *)ReadImageData(buffer, noBytes, &img_info);
  memcpy(matrixData, pixel_pos, LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL);

  for(int i = 0; i < LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL; i++) {
    if(matrixData[i] == 255) {
      matrixData[i] = 254;
    }
  }

  matrix.show();
}

void loop() {
  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    handleUdpPacket(noBytes);
  }
}

static const char *skipWhitespace(const char *buffer, const char *end) {
  for (;;) {
    while (buffer < end && isspace(*buffer))
      ++buffer;
    if (buffer >= end)
      return NULL;
    if (*buffer == '#') {
      while (buffer < end && *buffer != '\n') // read to end of line.
        ++buffer;
      continue;  // Back to whitespace eating.
    }
    return buffer;
  }
}

// Read next number. Start reading at *start; modifies the *start pointer
// to point to the character just after the decimal number or NULL if reading
// was not successful.
static int readNextNumber(const char **start, const char *end) {
  const char *start_number = skipWhitespace(*start, end);
  if (start_number == NULL) {
    *start = NULL;
    return 0;
  }
  char *end_number = NULL;
  int result = strtol(start_number, &end_number, 10);
  if (end_number == start_number) {
    *start = NULL;
    return 0;
  }
  *start = end_number;
  return result;
}

const char *ReadImageData(const char *in_buffer, size_t buf_len,
                          struct ImageMetaInfo *info) {
  if (in_buffer[0] != 'P' || in_buffer[1] != '6' ||
      (!isspace(in_buffer[2]) && in_buffer[2] != '#')) {
    return in_buffer;  // raw image. No P6 magic header.
  }
  const char *const end = in_buffer + buf_len;
  const char *parse_buffer = in_buffer + 2;
  const int width = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  const int height = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  const int range = readNextNumber(&parse_buffer, end);
  if (parse_buffer == NULL) return in_buffer;
  if (!isspace(*parse_buffer++)) return in_buffer;   // last char before data
  // Now make sure that the rest of the buffer still makes sense
  const size_t expected_image_data = width * height * 3;
  const size_t actual_data = end - parse_buffer;
  if (actual_data < expected_image_data)
    return in_buffer;   // Uh, not enough data.
  if (actual_data > expected_image_data) {
    // Our extension: at the end of the binary data, we provide an optional
    // offset. We can't store it in the header, as it is fixed in number
    // of fields. But nobody cares what is at the end of the buffer.
    const char *offset_data = parse_buffer + expected_image_data;
    info->offset_x = readNextNumber(&offset_data, end);
    if (offset_data != NULL) {
      info->offset_y = readNextNumber(&offset_data, end);
    }
    if (offset_data != NULL) {
      info->layer = readNextNumber(&offset_data, end);
    }
  }
  info->width = width;
  info->height = height;
  info->range = range;
  return parse_buffer;
}
