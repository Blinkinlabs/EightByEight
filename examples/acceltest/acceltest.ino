#include "mma8653.h"
#include <Wire.h>

MMA8653 accel;
const int i2c_scl = 14;
const int i2c_sda = 12;


void setup()
{
	Wire.begin(i2c_sda, i2c_scl);
	accel.setup();
	Serial.begin(460800);
}


void loop()
{
	float x, y, z;
	accel.getXYZ(x,y,z);

	Serial.print(x); Serial.print(' ');
	Serial.print(y); Serial.print(' ');
	Serial.print(z); Serial.print(' ');
	Serial.println(sqrt(x*x+y*y+z*z));

	delay(100);
}
