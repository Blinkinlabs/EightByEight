/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>

#include "mma8653.h"
#include "matrix.h"

const int listen_port = 6454;

const char ssid[] = "SSID";
const char password[] = "PASSWORD";

byte buffer[512]; //buffer to hold incoming and outgoing UDP packets

WiFiUDP Udp;

ESP8266WebServer server ( 80 );

const int i2c_scl = 14;
const int i2c_sda = 12;

const int button = 4;
const int pinA = 5;
const int pinB = 16;

MMA8653 mma8653;

Matrix matrix;

const int maxValueIndex = 10;
int valueIndex;
float xValues[maxValueIndex];
float yValues[maxValueIndex];
float zValues[maxValueIndex];

float xValue;
float yValue;
float zValue;


bool buttonPressed = false;

#include "mma8653.h"
#include "matrix.h"

void colorSwirl() {
  static float j = 0;
  static float f = 0;
  static float k = 0;

  float xValue = getAveraged(xValues, maxValueIndex)/2 + .5;
  if(xValue > 1) { xValue = 1;}
  if(xValue < 0) { xValue = 0;}
  float yValue = -getAveraged(yValues, maxValueIndex)/2 + .5;
  if(yValue > 1) { yValue = 1;}
  if(yValue < 0) { yValue = 0;}
  float zValue = -getAveraged(zValues, maxValueIndex)/2 + .5;
  if(zValue > 1) { zValue = 1;}
  if(zValue < 0) { zValue = 0;}
  zValue = .5;

  
  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      uint8_t r = xValue*64*(1+sin(row/2.0 + -col/3.0 + j/4.0       ));
      uint8_t g = yValue*64*(1+sin(-row/1.0 + col/4.0 + f/9.0  + 2.1));
      uint8_t b = zValue*64*(1+sin(row/3.0 + -col/2.0 + k/14.0 + 4.2));
      matrix.setPixelColor(row, col, r, g, b);
    }
  }
  matrix.show();

  j = j + .03;
  f = f + .02;
  k = k + .04;
}


void staticColor(int r, int g, int b) {
  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      matrix.setPixelColor(row, col, r, g, b);
    }
  }
  matrix.show();
}

void whitePulse() {
  static int i = 0;
  const int counts = 2;

  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      if(i >= counts/2) {
        matrix.setPixelColor(row, col, 255, 0, 0);
      }
      else {
        matrix.setPixelColor(row, col, 0, 0, 255);
      }
    }
  }
  matrix.show();
  
  i = (i + 1)%counts;
}

void gradientTest() {
  static int i = 0;
  const int counts = 200;

  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      int val=(row*LED_COLS+col)*255.0/(LED_ROWS*LED_COLS);
      matrix.setPixelColor(row, col, val,val,val);
    }
  }
  matrix.show();
    
  i = (i + 1)%counts;
}

void bleedTest() {
  static int i = 0;
  const int counts = 200;

  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      if((row == 0 && col == 0)) {
        matrix.setPixelColor(row, col, 255,255,255);
      }
      else if((row == 1)) {
        matrix.setPixelColor(row, col, 255,0,0);
      }
      else {
        matrix.setPixelColor(row, col, 0, 0, 0);
      }
    }
  }
  matrix.show();
    
  i = (i + 1)%counts;
}

void brightnessSteps() {
  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
        uint8_t val = (row + col*LED_ROWS)*4;
        matrix.setPixelColor(7-row, col, val, val, val);
    }
  }
  matrix.show();
}

void rgbTest() {
    static uint32_t count;
  const uint32_t maxCounts = 100;

  if(count < maxCounts) {
    staticColor(255,0,0);
  }
  else if(count < maxCounts*2) {
    staticColor(0,255,0);
  }
  else if(count < maxCounts*3) {
    staticColor(0,0,255);
  }
  else if(count < maxCounts*4) {
    staticColor(255,255,255);
  }
  else {
    count = 0;
  }
  
  count++;
}

void countUp() {
  static int count = 0;
  const int countSlowdown = 30;
  
  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      uint8_t val = 10;
      if(row*LED_COLS+col > count/countSlowdown) {
        val = 255;
      }
      
      matrix.setPixelColor(row, col, val, val, val);
    }
  }

  count = (count+1)%(LED_ROWS*LED_COLS*countSlowdown);
  
  matrix.show();
}


void handleRoot() {
	char temp[500];
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	snprintf ( temp, 500,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='1'/>\
    <title>Blinkinlabs EightByEight Accelerometer test</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from the BlinkyBadge!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" width=\"400\" height=\"150\" />\
    <p>Red: X</p>\
    <p>Green: Y</p>\
    <p>Blue: Z</p>\
    <p>Button state: %d</p>\
  </body>\
</html>",

		hr, min % 60, sec % 60, buttonPressed
	);
	server.send ( 200, "text/html", temp );
}

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void setup ( void ) {
  // USB communication to PC
	Serial.begin ( 460800 );
	WiFi.begin ( ssid, password );  
//	Serial.println ( "" );

//	// Wait for connection
//	while ( WiFi.status() != WL_CONNECTED ) {
//		delay ( 500 );
//		Serial.print ( "." );
//	}

//	Serial.println ( "" );
//	Serial.print ( "Connected to " );
//	Serial.println ( ssid );
//	Serial.print ( "IP address: " );
//	Serial.println ( WiFi.localIP() );

  Udp.begin(listen_port);

	if ( MDNS.begin ( "badge" ) ) {
		Serial.println ( "MDNS responder started" );
	}

	server.on ( "/", handleRoot );
	server.on ( "/test.svg", drawGraph );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );

  Wire.begin(i2c_sda, i2c_scl);
  Wire.setClock(400000);
  mma8653.setup();

  valueIndex = 0;
  for(int i = 0; i < maxValueIndex; i++) {
    xValues[i] = 0;
    yValues[i] = 0;
    zValues[i] = 0;
  }

  matrix.setup();

  pinMode(button, INPUT_PULLUP);

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);

  
  brightnessSteps();
}

float getAveraged(float* data, int count) {
  float value = 0;
  for(int i = 0; i < count; i++) {
    value += (data[i]);
  }
  return (value / count);
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
    Udp.read(buffer,noBytes); // read the packet into the buffer
  
//    // display the packet contents in HEX
//    for (int i=1;i<=noBytes;i++){
//      Serial.print(buffer[i-1],HEX);
//      if (i % 32 == 0){
//        Serial.println();
//      }
//      else Serial.print(' ');
//    } // end for
  
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
}

unsigned long lastFrame = 0;

void loop ( void ) {
  unsigned long now = millis();
  
	server.handleClient();

  // read the accelerometer data
//  valueIndex = (valueIndex + 1) % maxValueIndex;
//  mma8653.getXYZ(xValues[valueIndex], yValues[valueIndex], zValues[valueIndex]);

  buttonPressed = !digitalRead(button);

  
  if(lastFrame + 3000 < now) {
//    staticColor(255,255,255);
//    staticColor(127,127,127);
//    colorSwirl();
//  bleedTest();
//    brightnessSteps();
//    gradientTest();
//    rgbTest();
    countUp();
  }

  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    lastFrame = now;
    handleUdpPacket(noBytes);
  }
}

void drawGraph() {
	String out = "";
	char temp[100];
	out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
 	out += "<rect width=\"400\" height=\"150\" fill=\"rgb(255, 255, 255)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
   
 	out += "<g stroke=\"red\">\n";
 	for (int x = 0; x < 99; x+= 1) {
    int y = xValues[x]*5;
    int y2 = xValues[x+1]*5;
 		sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", (x)*4, y + 75, (x+1)*4, y2 + 75);
 		out += temp;
 	}
	out += "</g>\n";
 
  out += "<g stroke=\"green\">\n";
  for (int x = 0; x < 99; x+= 1) {
    int y = yValues[x]*5;
    int y2 = yValues[x+1]*5;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", (x)*4, y + 75, (x+1)*4, y2 + 75);
    out += temp;
  }
  out += "</g>\n";
  
  out += "<g stroke=\"blue\">\n";
   for (int x = 0; x < 99; x+= 1) {
    int y = zValues[x]*5;
    int y2 = zValues[x+1]*5;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", (x)*4, y + 75, (x+1)*4, y2 + 75);
    out += temp;
  }
  out += "</g>\n";
  
	out += "</svg>\n";

	server.send ( 200, "image/svg+xml", out);
}
