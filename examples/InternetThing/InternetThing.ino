/** \file
 * Demo using the EightByEight blinky badge with MQTT.
 *
 * Registers for a message that will update its display and
 * sends acceleration data at 2 Hz.
 */
#include "Badge.h"
#include "TextScroll.h"
#include <PubSubClient.h>

#define wifi_ssid "WIFI_SSID_GOES_HERE"
#define wifi_pwd "WIFI_PASSWORD_GOES_HERE"

#define mqtt_server "MQTT_SERVER_NAME"
#define mqtt_port 13220

#define mqtt_app "InternetThing"
#define mqtt_user "thing1"
#define mqtt_password "thing1pw"


// How often to update the accleration data to the server
#define update_rate_ms 500


Badge badge;
TextScroll text;

WiFiClient espClient;
PubSubClient client(espClient);


// Display a scrolling message for a while, doing nothing else
static void scroll_delay(long length_ms)
{
	const long end = millis() + length_ms;
	while(millis() < end)
	{
		text.step();
		text.draw(badge.matrix);
		badge.matrix.show();
		delay(30);
	}
}


// Process an incoming message that we have subscribed to.
static void
mqtt_callback(
	const char * const topic_raw,
	const byte * const payload,
	unsigned int len
)
{
 	String topic = topic_raw;

	Serial.print("MQTT topic '");
	Serial.print(topic);
	Serial.print("' length ");
	Serial.println(len);

	if (topic == "thing/message")
	{
		if (len > 16)
			len = 16;
		char msg[17] = {};
		memcpy(msg, payload, len);
		text.load_string(msg);
	}
}


// Blocking reconnect to the server, trying every 5 seconds.
// This should give up after a while
static void mqtt_reconnect()
{
	text.load_string("MQTT ");

	while(!client.connected())
	{
		Serial.print("connecting to ");
		Serial.print(mqtt_server);
		Serial.print(":");
		Serial.print(mqtt_port);
		Serial.print(" as ");
		Serial.print(mqtt_app);
		Serial.print("/");
		Serial.print(mqtt_user);
		Serial.println();

		if(client.connect(mqtt_app, mqtt_user, mqtt_password))
			return;

		Serial.println("connection failed");

		// draw the reconnect message while we spin
		scroll_delay(5000);
	}
}


void setup()
{
	badge.begin();
	badge.matrix.clear();
	badge.matrix.show();

	text.begin();

	WiFi.persistent(false);
	WiFi.begin(wifi_ssid, wifi_pwd);

	text.load_string(wifi_ssid);

	// scroll the wifi network name while we wait for connection
	Serial.print("joining network: ");
	Serial.println(wifi_ssid);

 	while (WiFi.status() != WL_CONNECTED)
	{
		text.step();
		text.draw(badge.matrix);
		badge.matrix.show();
		delay(30);
	}
	
	// let the user know that we have an IP address
	String s = WiFi.localIP().toString();
	text.load_string(s.c_str());
	Serial.print("local IP: ");
	Serial.println(s);

	// connect to the mqtt server
	client.setCallback(mqtt_callback);
	client.setServer(mqtt_server, mqtt_port);

	mqtt_reconnect();

	// let them know that we are alive
	client.publish("thing/connected", "1", true);
	text.load_string("GOOD ");
	scroll_delay(2000);

	// and listen for messages to be sent to us
	client.subscribe("thing/message");

	Serial.println("ready!");
}


void loop()
{
	if (!client.connected())
		mqtt_reconnect();
	client.loop();

	badge.poll();

	// every update_rate_ms send the acceleration values to the server
	static unsigned long last_update;
	if (millis() > last_update + update_rate_ms)
	{
		last_update = millis();
		String accel;
		accel += badge.ax;
		accel += ",";
		accel += badge.ay;
		accel += ",";
		accel += badge.az;
		client.publish("thing/accel", accel.c_str(), true);
		Serial.println(accel);
	}

	text.step();
	text.draw(badge.matrix);
	badge.matrix.show();
}
