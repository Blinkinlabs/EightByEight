#include "Demo.h"

class Bubble : public Demo
{
public:
	virtual void begin();
 
  virtual void tapped();
 
	virtual bool step(float ax, float ay, float az);

	virtual void draw(RGBMatrix &matrix);

private:
	float ax, ay, az;

  int palette_index = 0;
};

