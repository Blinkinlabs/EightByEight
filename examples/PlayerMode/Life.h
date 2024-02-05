/** \file
 * Game of Life (on the 8x8 torus)
 */

#include "Demo.h"

class Life : public Demo
{
private:
	uint8_t board[8];
	int get(int i, int j);
	int steps;
	int change_steps;
	int blend;

public:
	virtual void begin();

  virtual void tapped();

	virtual bool step(float ax, float ay, float az);

	virtual void draw(RGBMatrix &matrix);

	void reset();
};
