/** \file
 * Color Swirl
 */

#include "Demo.h"

class ColorSwirl : public Demo
{
private:
  float j;
  float f;
  float k;
  
public:
  virtual void begin();

  virtual void tapped();

  virtual bool step(float ax, float ay, float az);

  virtual void draw(RGBMatrix &matrix);

  void reset();
};


