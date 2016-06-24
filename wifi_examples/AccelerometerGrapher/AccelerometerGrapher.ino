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
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <Wire.h>

#include "mma8653.h"

const char *ssid = "BLINKINLABS";
const char *password = "***REMOVED***";

ESP8266WebServer server ( 80 );

const int i2c_scl = 14;
const int i2c_sda = 12;

const int button = 4;

MMA8653 mma8653;

const int maxValueIndex = 100;
int valueIndex;
float xValues[maxValueIndex];
float yValues[maxValueIndex];
float zValues[maxValueIndex];

bool buttonPressed = false;

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

	server.on ( "/", handleRoot );
	server.on ( "/test.svg", drawGraph );
	server.on ( "/inline", []() {
		server.send ( 200, "text/plain", "this works as well" );
	} );
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );

  Wire.begin(i2c_sda, i2c_scl);
  mma8653.setup();

  Serial1.begin( 230400 );

  valueIndex = 0;
  for(int i = 0; i < maxValueIndex; i++) {
    xValues[i] = 0;
    yValues[i] = 0;
    zValues[i] = 0;
  }

  

  pinMode(button, INPUT_PULLUP);
}

#define LED_ROWS 8
#define LED_COLS 8
#define LED_BYTES_PER_PIXEL 3

uint8_t colorData[LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL];

void matrix_setPixelColor(int row, int col, uint8_t r, uint8_t g, uint8_t b) {
  if(r == 255) {
    r = 254;
  }
  if(g == 255) {
    g = 254;
  }
  if(b == 255) {
    b = 254;
  }
  
  colorData[(row*LED_COLS + col)*3 + 0] = r;
  colorData[(row*LED_COLS + col)*3 + 1] = g;
  colorData[(row*LED_COLS + col)*3 + 2] = b;
}

void matrix_show() {
  // LED control
  for(int i = 0; i < LED_ROWS*LED_COLS*LED_BYTES_PER_PIXEL; i++) {
    Serial1.print(char(colorData[i]));
  }
  Serial1.print(char(255));
}

void colorSwirl() {
  static float j = 0;
  static float f = 0;
  static float k = 0;

  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      uint8_t r = 64*(1+sin(row/2.0 + -col/3.0 + j/4.0       ));
      uint8_t g = 64*(1+sin(-row/1.0 + col/4.0 + f/9.0  + 2.1));
      uint8_t b = 64*(1+sin(row/3.0 + -col/2.0 + k/14.0 + 4.2));
      matrix_setPixelColor(row, col, r, g, b);
    }
  }
  matrix_show();

  j = j + .3;
  f = f + .2;
  k = k + .4;
}


void whitePulse() {
  static int i = 0;
  const int counts = 2;

  for (uint8_t row = 0; row < LED_ROWS; row++) {
    for (uint8_t col = 0; col < LED_COLS; col++) {
      if(i >= counts/2) {
        matrix_setPixelColor(row, col, 255, 0, 0);
      }
      else {
        matrix_setPixelColor(row, col, 0, 0, 255);
      }
    }
  }
  matrix_show();
    
  i = (i + 1)%counts;
}


void loop ( void ) {
	server.handleClient();

  // read the accelerometer data
  mma8653.getXYZ(xValues[valueIndex], yValues[valueIndex], zValues[valueIndex]);
  valueIndex = (valueIndex + 1) % maxValueIndex;

  buttonPressed = !digitalRead(button);

  colorSwirl();
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
