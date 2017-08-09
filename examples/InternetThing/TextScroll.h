#include "RGBMatrix.h"


class TextScroll
{
public:
	void begin();
	bool step();

	void draw(RGBMatrix & matrix);

	void load_string(const char* string);

private:

	void scroll_step();

	bool screen[LED_ROWS][LED_COLS];
	uint16_t next_row;
	uint32_t last_time_scroll;

	bool buffer[8*256][LED_ROWS];
	bool *buffer_end;
};
