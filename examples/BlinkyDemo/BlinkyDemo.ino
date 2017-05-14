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

#include "Badge.h"
#include "Pixels.h"
#include "SerialReceiver.h"

#include "Life.h"
#include "Bubble.h"
#include "Rain.h"
#include "ColorSwirl.h"

Badge badge;

Pixels pixels; // udp packet receiver
SerialReceiver serialReceiver;  // BlinkyTape communications protocol

Bubble bubble;  // Enabled demos
Life life;
Rain rain;
ColorSwirl colorSwirl;

Demo * demos[] = {
	&rain,
	&life,
	&bubble,
  &colorSwirl,
};

const unsigned demo_count = sizeof(demos) / sizeof(*demos);
unsigned demo_num = 0;
Demo * demo;

uint32_t last_draw_millis;
uint32_t streaming_start_time;
bool streaming;

void setup()
{
	badge.begin();

	WiFi.persistent(false);

	// do not join any wifi networks if the button is held down
	// during startup
	if (!badge.button())
	{
#if 0
		WiFi.mode(WIFI_STA);
		WiFi.begin("twosigma-blinky", "blinkblinkblink");
#else
		WiFi.mode(WIFI_AP);
		WiFi.begin("ssid", "password");
		WiFi.config(IPAddress(192,168,1,4), IPAddress(0,0,0,0), IPAddress(255,255,255,0));
#endif
	}

	pixels.begin();
  serialReceiver.begin();

	// Initialize all of the demos and start at 0
	for(int i = 0 ; i < demo_count ; i++)
		demos[i]->begin();

	demo_num = 0;
	demo = demos[demo_num];
}


void loop()
{
  if (badge.poll())
  {
    demo->tapped();
  }
  
	if (badge.button_edge())
	{
		// should cycle to the next demo
		demo_num = (demo_num + 1) % demo_count;
		demo = demos[demo_num];
	}

	const uint32_t now = millis();

	bool do_draw = demo->step(badge.ax, badge.ay, badge.az);

  if (serialReceiver.step(0,0,0))
  {    
    streaming = true;
    streaming_start_time = now;

    serialReceiver.draw(badge.matrix);
    badge.matrix.show();
  }

	if (pixels.step(0,0,0))
	{
    streaming = true;
    streaming_start_time = now;

    pixels.draw(badge.matrix);
    badge.matrix.show();
	}


	if (streaming && (now - streaming_start_time > 10000ul))
	{
		// no video for ten seconds, go back to normal demo
		streaming = false;
	}

  if (!streaming)
  {
	  // Draw the LEDs at 60Hz
	  if (!do_draw && now - last_draw_millis < (1000/60))
		  return;
	  last_draw_millis = now;
	
		demo->draw(badge.matrix);

  	badge.matrix.show();
  }
}


