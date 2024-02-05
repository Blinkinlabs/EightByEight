/** \file
   Combine multiple demos for the Two Sigma / Blinklabs EightByEight badge.

   The button is used to cycle between the different demos.

   Pixels and SerialReceiver are not really seperate demos; we switch to them
   automatically if they tell us that they have received a packet.
   If we go more than ten seconds without a packet, the current demo
   is restarted.
*/

#include "FS.h"

#include "Badge.h"
#include "SerialReceiver.h"
#include "patterns.h"
#include "patterndemo.h"

#include "Rain.h"

Badge badge;

volatile bool fileAccessLocked;
volatile bool reloadAnimations;

SerialReceiver serialReceiver;  // BlinkyTape communications protocol

Patterns patterns;

PatternDemo patternDemo;

Rain rain;

Demo * builtin_demos[] = {
  &rain,
};

const uint8_t brightness_steps[] = {127,64,32,64,127,255};
#define BRIGHTNESS_STEP_COUNT sizeof(brightness_steps)
unsigned brightness_step = 0;

const unsigned builtin_demo_count = sizeof(builtin_demos) / sizeof(*builtin_demos);
unsigned demo_num = 0;
Demo * demo;

uint32_t last_draw_millis;
uint32_t streaming_start_time;
bool streaming;

void load_demo(unsigned new_demo_num)
{
  // If there are no loaded demos, show a rain pattern
  if (patterns.getCount() == 0) {
    if (builtin_demo_count > 0) {
      demo = builtin_demos[0];
      return;
    }
  }

  demo_num = new_demo_num % patterns.getCount();

  patternDemo = patterns.open(demo_num);
  demo = &patternDemo;
}


void setup()
{
  badge.begin();

  WiFi.persistent(false);

  SPIFFS.begin();

  //    // do not join any wifi networks if the button is held down
  //    // during startup
  //    if (!badge.button())
  //    {
  //#if 0
  //        WiFi.mode(WIFI_STA);
  //        WiFi.begin("twosigma-blinky", "blinkblinkblink");
  //#else
  //        WiFi.mode(WIFI_AP);
  //        WiFi.begin("ssid", "password");
  //        WiFi.config(IPAddress(192,168,1,4), IPAddress(0,0,0,0), IPAddress(255,255,255,0));
  //#endif
  //    }


  badge.matrix.setBrightness(brightness_steps[brightness_step]);

  serialReceiver.begin();

  // Use the accelerometer position as a random seend
  badge.poll();
  randomSeed(badge.nx + badge.ny + badge.nz);

  fileAccessLocked = false;
  reloadAnimations = true;

  // Initialize all of the demos and start at 0
  for (int i = 0 ; i < builtin_demo_count ; i++)
    builtin_demos[i]->begin();

  load_demo(0);
}

void loop()
{
  if (reloadAnimations) {
    patterns.begin();
    reloadAnimations = false;

    load_demo(0);
  }

  if (badge.poll())
  {
    demo->tapped();
  }

  if (badge.button_edge())
  {
    brightness_step = (brightness_step+1) % BRIGHTNESS_STEP_COUNT;
    badge.matrix.setBrightness(brightness_steps[brightness_step]);
  }

  const uint32_t now = millis();

  bool do_draw = demo->step(badge.ax, badge.ay, badge.az);

  if(demo->finished()) {
    if(patterns.getCount() > 1) {
      unsigned next_demo_num;
      do {
        // Skip the first pattern
        next_demo_num = random(patterns.getCount()-1)+1;
      }
      while (next_demo_num == demo_num);
      
      load_demo(next_demo_num);      
    }
  }

  if (serialReceiver.step(0, 0, 0))
  {
    streaming = true;
    streaming_start_time = now;

    serialReceiver.draw(badge.matrix);
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
    if (!do_draw && now - last_draw_millis < (1000 / 60))
      return;

    last_draw_millis = now;

    demo->draw(badge.matrix);
    badge.matrix.show();
  }
}
