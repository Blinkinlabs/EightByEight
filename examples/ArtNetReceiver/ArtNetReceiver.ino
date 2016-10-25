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

  
  matrix.show();
    
    Serial.println();
  } // end if - See more at: http://www.esp8266.com/viewtopic.php?f=29&t=2222&start=8#sthash.5kCVT3fO.dpuf
}

