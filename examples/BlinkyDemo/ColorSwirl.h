/** \file
 * Game of Life (on the 8x8 torus)
 */

#include "Demo.h"

class ColorSwirl : public Demo
{
private:
  float j;
  float f;
  float k;
  
public:
  ColorSwirl();

  virtual void begin();

  virtual bool step(float ax, float ay, float az);

  virtual void draw(RGBMatrix &matrix);

  void reset();
};
