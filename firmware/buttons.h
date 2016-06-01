#ifndef BUTTONS_H
#define BUTTONS_H

// Button names
#define BUTTON_A      0 // Start/stop logging
#define BUTTON_B      1 // Set logging interval


struct button {
    uint8_t pin;        // Arduino pin number that the button is connected to
    uint8_t inverted;   // If true, button state is inverted
};


class Buttons {
  private:
    int pressedButton;    // Stores the button that was pressed
    int lastPressed;
    int debounceCount;    // Number of times we've seen the same input
    #define DEBOUNCE_INTERVAL 500    // Number of times we need s
        
  public:
    // Initialize the buttons class
    void setup();
    
    // Scan for button presses
    void buttonTask();
    
    // Check if a button has been pressed
    // @return True if a button press is waiting, false otherwise
    bool isPressed();
    
    // Get the last pressed button
    // @return Identifier of the last button that was pressed
    int getPressed();
};


#endif // BUTTONARRAY_HH
