#include "WProgram.h"
#include "matrix.h"
#include "patterns.h"

void count_up_loop() {
    static int pixel = 0;
    #define slowdown 5000
   
    for (uint16_t row = 0; row < LED_ROWS; row+=1) {
        for (uint16_t col = 0; col < LED_COLS; col+=1) {
            if(col == pixel/slowdown) {
                setPixel(col, row, 255);
            }
            else {
                setPixel(col, row, 0);
            }
        }
    }
    
    pixel = (pixel+1)%(LED_COLS*slowdown);
}

