#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266mDNS.h>

#include "Matrix.h"
#include "secrets.h"

const int listen_port = 6454;


byte buffer[512]; //buffer to hold incoming and outgoing packets

WiFiUDP Udp;

Matrix matrix;


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

void loop() {
  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    Serial.print(millis() / 1000);
    Serial.print(":Packet of ");
    Serial.print(noBytes);
    Serial.print(" received from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.println(Udp.remotePort());
    // We've received a packet, read the data from it
    Udp.read(buffer,noBytes); // read the packet into the buffer

    // display the packet contents in HEX
    for (int i=1;i<=noBytes;i++){
      Serial.print(buffer[i-1],HEX);
      if (i % 32 == 0){
        Serial.println();
      }
      else Serial.print(' ');
    } // end for

  // Todo: check for a valid art-net frame here
  uint8_t* matrixData = matrix.getPixels();
  uint8_t* udpData = &buffer[18];
  memcpy(matrixData, udpData, LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL);

  for(int i = 0; i < LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL; i++) {
    if(matrixData[i] == 255) {
      matrixData[i] = 254;
    }
  }
  
  matrix.show();
    
    Serial.println();
  } // end if - See more at: http://www.esp8266.com/viewtopic.php?f=29&t=2222&start=8#sthash.5kCVT3fO.dpuf
}

