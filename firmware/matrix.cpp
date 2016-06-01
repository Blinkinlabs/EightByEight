/*
 * DMA Matrix Driver
 * 
 * Copyright (c) 2014 Matt Mets
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "matrix.h"
#include "brightness_table.h"

// Offsets in the port c register (data)
#define DMA_DAT_SHIFT   0       // Location of the data pin in Port B register
#define DMA_CLK_SHIFT   1       // Location of the clock pin in Port B register

// Offsets in the port D register (address)
#define DMA_S0_SHIFT   5
#define DMA_S1_SHIFT   6
#define DMA_STB_SHIFT  4        // Location of the strobe pin in Port D register


// Output positions for the signals in each group
// These are out of order to make the board routing easier
// RGB RGB RGB RGB RGB
/*
uint8_t OUTPUT_ORDER[] = {
    2,  // R0
    0,  // G0
    1,  // B0
    5,  // R1
    3,  // G1
    4,  // B1
    8,  // R2
    6,  // G2
    7,  // B2
    11,  // R3
    9, // G3
    10, // B3
    14, // R4
    12, // G4
    13, // B4
};
*/
uint8_t OUTPUT_ORDER[] = {
    0,  // R0
    1,  // G0
    2,  // B0
    3,  // R1
    4,  // G1
    5,  // B1
    6,  // R2
    7,  // G2
    8,  // B2
    9,  // R3
    10, // G3
    11, // B3
    12, // R4
    13, // G4
    14, // B4
};

// Display buffer (write into this!)
Pixel pixels[LED_ROWS * LED_COLS];

float systemBrightness = 1;

// Address output buffer
// Note: We repeat the address output multiple times, to add a delay between OE deasserting and the address lines changing
#define ADDRESS_REPEAT_COUNT 10  // was 10
uint8_t Addresses[BIT_DEPTH*LED_ROWS*ADDRESS_REPEAT_COUNT];

// Timer output buffers (these will be DMAd to the FTM0_MOD and FTM0_C1V registers)
uint32_t FTM0_MODStates[BIT_DEPTH*LED_ROWS];
uint32_t FTM0_C1VStates[BIT_DEPTH*LED_ROWS];

// Big 'ol waveform that should be sent out over DMA in chunks.
// There are LED_ROWS separate loops, where the LED matrix address lines
// to be set before they are activated.
// For each of these rows, there are then BIT_DEPTH separate inner loops
// And each inner loop has LED_COLS * 2 bytes states (the data is LED_COLS long, plus the clock signal is baked in)

#define ROW_BIT_SIZE (LED_COLS*BYTES_PER_PIXEL*2)                  // Number of bytes required to store a single row of 1-bit color data output
#define ROW_DEPTH_SIZE (ROW_BIT_SIZE*BIT_DEPTH)                    // Number of bytes required to store a single row of full-color data output
#define PANEL_DEPTH_SIZE (ROW_DEPTH_SIZE*LED_ROWS)                 // Number of bytes required to store an entire panel's worth of data output.

// 2x DMA buffer
// Note: Extra ROW_BIT_SIZE at end to account for extra DMA transfer
// TODO: Trigger int from last address and skip the extra data transfer?
uint8_t dmaBuffer[2][PANEL_DEPTH_SIZE*PAGES];
uint8_t* frontBuffer;
uint8_t* backBuffer;
volatile bool swapBuffers;
uint8_t currentPage;

void pixelsToDmaBuffer(Pixel* pixelInput, uint8_t bufferOutput[]);

void setupTCD0(uint32_t* source, int minorLoopSize, int majorLoops);
void setupTCD1(uint32_t* source, int minorLoopSize, int majorLoops);
void setupTCD2(uint8_t* source, int minorLoopSize, int majorLoops);
void setupTCD3(uint8_t* source, int minorLoopSize, int majorLoops);
void dma_ch2_isr(void);
void setupTCDs();
void setupFTM0();

void matrixStart() {
    setupTCDs();
}

void matrixSetup() {
  // Set all the pins to outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);

  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(LED_CLOCK_PIN, OUTPUT);
  pinMode(LED_STROBE_PIN, OUTPUT);
  pinMode(LED_OE_PIN, OUTPUT);

  digitalWrite(LED_DATA_PIN, HIGH);

  // Fill the address table
  // To make the DMA engine easier to program, we store a copy of the address table for each output page.
  for(int address = 0; address < LED_ROWS; address++) {
    for(int depth = 0; depth < BIT_DEPTH; depth++) {
      int last_address;
      if(depth == 0) {
        last_address = (address + LED_ROWS - 1)%(LED_ROWS);
      }
      else {
        last_address = address;
      }

//#define addressBits(addr) (~((1<<DMA_STB_SHIFT) | (1<<(DMA_S0_SHIFT + addr))))
#define addressBits(addr) (~((1<<DMA_STB_SHIFT) | ((addr)<<(DMA_S0_SHIFT))))

      for(int i = 0; i < ADDRESS_REPEAT_COUNT; i++) {
        // Note: We're actually pumping out the last address here, to avoid changing it too soon after
        // deasserting enable.
        //Addresses[(address*BIT_DEPTH + depth)*ADDRESS_REPEAT_COUNT + i] = (0x3F & ~(1 << last_address));
        Addresses[(address*BIT_DEPTH + depth)*ADDRESS_REPEAT_COUNT + i] = addressBits(last_address);
      }
      
      // TODO: Inserted to cause extra delay between OE and address change.
      Addresses[(address*BIT_DEPTH + depth)*ADDRESS_REPEAT_COUNT + ADDRESS_REPEAT_COUNT - 2] = addressBits(address) | (1 << DMA_STB_SHIFT);
      Addresses[(address*BIT_DEPTH + depth)*ADDRESS_REPEAT_COUNT + ADDRESS_REPEAT_COUNT - 1] = addressBits(address);
    }
  }


  // Fill the timer states table
  for(int address = 0; address < LED_ROWS; address++) {

    // Each row update consists of BIT_DEPTH cycles. The length of the 'on' time
    // (when OE is asserted) on each cycle is set by onTime; it begins with
    // ON_TIME_MIN and doubles every cycle after that to create a binary progression.
    // TODO: What does this translate to, in time?
    #define LOW_BIT_ENABLE_TIME     0x2             // Shortest OE on interval; the shorter, the dimmer the lowest bit.
    
    // The interval between OE cycle is set by one of the three cases:
    // 1. For low bits, where onTime is small, the interval is expanded to MIN_CYCLE_TIME
    // 2. For longer bits, where onTime is longer, the cycle time is calculated as onTime + MIN_BLANKING_TIME
    // 3. For the last cycle of the last row, the cycle time is expanded to MIN_LAST_CYCLE_TIME to allow
    //    the display interrupt to update.

    #define MIN_BLANKING_TIME       0x50        // Minimum time between OE assertions
    #define MIN_CYCLE_TIME          0x05F       // 
    #define MIN_LAST_CYCLE_TIME     0x0120      // Mininum number of cycles for the last cycle loop.


    int onTime = LOW_BIT_ENABLE_TIME;               

    for(int depth= 0; depth< BIT_DEPTH; depth++) {
      if((address == LED_ROWS -1)
         && (depth == BIT_DEPTH - 2)
         && ((onTime + MIN_BLANKING_TIME) < MIN_LAST_CYCLE_TIME)) {
        // On the second-to-last cycle, we need enough time to flush the DMA engines and handle the
        // interrupt to reset the DMA engines. If the combination of blanking time and
        // on time don't meet this, increase the timer cycle count to an acceptable length.
        FTM0_C1VStates[address*BIT_DEPTH + depth] = onTime;
        FTM0_MODStates[address*BIT_DEPTH + depth] = MIN_LAST_CYCLE_TIME;        
      }      
      else if((onTime + MIN_BLANKING_TIME) < MIN_CYCLE_TIME) {
        // The DMA engines need enough time to write out the data after every cycle.
        // WHen the on time is really low, the combination of blanking time and
        // on time might not create a long enough delay to meet this, so we need to increase
        // the timer cycle count to meet this requirement.
        FTM0_C1VStates[address*BIT_DEPTH + depth] = onTime;
        FTM0_MODStates[address*BIT_DEPTH + depth] = MIN_CYCLE_TIME;
      }
      else {
        FTM0_C1VStates[address*BIT_DEPTH + depth] = onTime;      
        FTM0_MODStates[address*BIT_DEPTH + depth] = onTime + MIN_BLANKING_TIME;
      }

      onTime = onTime*2;
    }
  }

  // DMA
  // Configure DMA
  SIM_SCGC7 |= SIM_SCGC7_DMA;  // Enable DMA clock
  DMA_CR = 0;  // Use default configuration

  // Configure the DMA request input for DMA0
  DMA_SERQ = DMA_SERQ_SERQ(0);

  // Enable interrupt on major completion for DMA channel 2 (address)
  DMA_TCD2_CSR = DMA_TCD_CSR_INTMAJOR;  // Enable interrupt on major complete
  NVIC_ENABLE_IRQ(IRQ_DMA_CH2);         // Enable interrupt request

  // DMAMUX
  // Configure the DMAMUX
  SIM_SCGC6 |= SIM_SCGC6_DMAMUX; // Enable DMAMUX clock

  // Timer DMA channel:
  // Configure DMAMUX to trigger DMA0 from FTM0_CH1
  DMAMUX0_CHCFG0 = DMAMUX_DISABLE;
  DMAMUX0_CHCFG0 = DMAMUX_SOURCE_FTM0_CH1 | DMAMUX_ENABLE;

  // Load this frame of data into the DMA engine
  setupTCDs();

  // FTM
  SIM_SCGC6 |= SIM_SCGC6_FTM0;  // Enable FTM0 clock
  setupFTM0();

  frontBuffer = dmaBuffer[0];
  backBuffer = dmaBuffer[1];
  swapBuffers = false;
  currentPage = 0;

  // Clear the display and kick off transmission
  memset(backBuffer, 0, LED_ROWS*LED_COLS*PAGES);
  show();
}

bool bufferWaiting() {
    return swapBuffers;
}

void show() {
    if(swapBuffers) {
        return;
    }
    
    pixelsToDmaBuffer(pixels, backBuffer);
    // TODO: Atomic operation?
    swapBuffers = true;
}

#if BYTES_PER_PIXEL == 3
void setPixel(int column, int row, uint8_t r, uint8_t g, uint8_t b) {
    // Don't do anything if the pixel is out of range
    if (column >= LED_COLS || row >= LED_ROWS) {
        return;
    }

    pixels[row*LED_COLS + column].R = r;
    pixels[row*LED_COLS + column].G = g;
    pixels[row*LED_COLS + column].B = b;
}

#elif BYTES_PER_PIXEL == 1
void setPixel(int column, int row, uint8_t intensity) {
    // Don't do anything if the pixel is out of range
    if (column >= LED_COLS || row >= LED_ROWS) {
        return;
    }

    pixels[row*LED_COLS + column] = intensity;
}

#endif

Pixel* getPixels() {
    return pixels;
}

void setBrightness(float brightness) {
    systemBrightness = brightness;
}

// Munge the data so it can be written out by the DMA engine
// Note: bufferOutput[][xxx] should have BIT_DEPTH as xxx
void pixelsToDmaBuffer(Pixel* pixelInput, uint8_t bufferOutput[]) {
  for(int page = 0; page < PAGES; page++) {
    for(int row = 0; row < LED_ROWS; row++) {
      for(int col = 0; col < LED_COLS; col++) {
    
#if BYTES_PER_PIXEL == 3

#error Cannot handle 3 bytes with pages yet!     
        uint16_t data_R = brightnessTable[pixelInput[row*LED_COLS + col].R];
        uint16_t data_G = brightnessTable[pixelInput[row*LED_COLS + col].G];
        uint16_t data_B = brightnessTable[pixelInput[row*LED_COLS + col].B];
 
        for(int depth = 0; depth < BIT_DEPTH; depth++) {
          uint8_t output_r =
              (((data_R >> depth) & 0x01) << DMA_DAT_SHIFT);
          uint8_t output_g =
              (((data_G >> depth) & 0x01) << DMA_DAT_SHIFT);
          uint8_t output_b =
              (((data_B >> depth) & 0x01) << DMA_DAT_SHIFT);
 
          int offset_r = OUTPUT_ORDER[col*3 + 0];
          int offset_g = OUTPUT_ORDER[col*3 + 1];
          int offset_b = OUTPUT_ORDER[col*3 + 2];
 
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_r*2 + 0] = output_r;
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_r*2 + 1] = output_r | 1 << DMA_CLK_SHIFT;
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_g*2 + 0] = output_g;
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_g*2 + 1] = output_g | 1 << DMA_CLK_SHIFT;
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_b*2 + 0] = output_b;
          bufferOutput[row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset_b*2 + 1] = output_b | 1 << DMA_CLK_SHIFT;
 
#elif BYTES_PER_PIXEL == 1
 
        int data = brightnessTable[int(systemBrightness*pixelInput[row*LED_COLS + col])];

        int page_data = data & ((1<<PAGED_BITS)-1);
        data = data >> (PAGED_BITS - 1);
 
        for(int depth = 0; depth < BIT_DEPTH; depth++) {
          uint8_t output = 0;

          if(depth > 0) {
            // Regular bit
            output = (((data >> depth) & 0x01) << DMA_DAT_SHIFT);
          }
          else {
                if(page == 0 && page_data > 0) {
                    output = (1 << DMA_DAT_SHIFT);
                }
                else if(page == 1 && page_data > 1) {
                    output = (1 << DMA_DAT_SHIFT);
                }
                else if(page == 2 && page_data > 2) {
                    output = (1 << DMA_DAT_SHIFT);
                }
                else if(page == 3 && page_data > 2) {
                    output = (1 << DMA_DAT_SHIFT);
                }
          }
 
          int offset = col;
 
          bufferOutput[page*PANEL_DEPTH_SIZE + row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset*2 + 0] = output;
          bufferOutput[page*PANEL_DEPTH_SIZE + row*ROW_DEPTH_SIZE + depth*ROW_BIT_SIZE + offset*2 + 1] = output | 1 << DMA_CLK_SHIFT;
 
#endif
        }
      }
    }
  }
}


// TCD0 updates the timer values for FTM0
void setupTCD0(uint32_t* source, int minorLoopSize, int majorLoops) {
  DMA_TCD0_SADDR = source;                                        // Address to read from
  DMA_TCD0_SOFF = 4;                                              // Bytes to increment source register between writes 
  DMA_TCD0_ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2);  // 32-bit input and output
  DMA_TCD0_NBYTES_MLNO = minorLoopSize;                           // Number of bytes to transfer in the minor loop
  DMA_TCD0_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
  //  DMA_TCD0_DADDR = TimerStatesDump;                               // Address to write to
  DMA_TCD0_DADDR = &FTM0_MOD;                                      // Address to write to
  DMA_TCD0_DOFF = 0;                                              // Bytes to increment destination register between write
  //  DMA_TCD0_CITER_ELINKNO = majorLoops;                            // Number of major loops to complete
  //  DMA_TCD0_BITER_ELINKNO = majorLoops;                            // Reset value for CITER (must be equal to CITER)
  DMA_TCD0_DLASTSGA = 0;                                          // Address of next TCD (N/A)

  // Workaround for DMA majorelink unreliability: increase the minor loop count by one
  // Note that the final transfer doesn't end up happening, because 
  DMA_TCD0_CITER_ELINKYES = majorLoops + 1;                           // Number of major loops to complete
  DMA_TCD0_BITER_ELINKYES = majorLoops + 1;                           // Reset value for CITER (must be equal to CITER)

  // Trigger DMA1 (timer) after each minor loop
  DMA_TCD0_BITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD0_BITER_ELINKYES |= (0x01 << 9);  
  DMA_TCD0_CITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD0_CITER_ELINKYES |= (0x01 << 9);
}

// TCD1 updates the timer values for FTM0
void setupTCD1(uint32_t* source, int minorLoopSize, int majorLoops) {
  DMA_TCD1_SADDR = source;                                        // Address to read from
  DMA_TCD1_SOFF = 4;                                              // Bytes to increment source register between writes 
  DMA_TCD1_ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2);  // 32-bit input and output
  DMA_TCD1_NBYTES_MLNO = minorLoopSize;                           // Number of bytes to transfer in the minor loop
  DMA_TCD1_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
  //  DMA_TCD0_DADDR = TimerStatesDump;                               // Address to write to
  DMA_TCD1_DADDR = &FTM0_C1V;                                      // Address to write to
  DMA_TCD1_DOFF = 0;                                              // Bytes to increment destination register between write
  //  DMA_TCD1_CITER_ELINKNO = majorLoops;                            // Number of major loops to complete
  //  DMA_TCD1_BITER_ELINKNO = majorLoops;                            // Reset value for CITER (must be equal to CITER)
  DMA_TCD1_DLASTSGA = 0;                                          // Address of next TCD (N/A)

  // Workaround for DMA majorelink unreliability: increase the minor loop count by one
  // Note that the final transfer doesn't end up happening, because 
  DMA_TCD1_CITER_ELINKYES = majorLoops + 1;                           // Number of major loops to complete
  DMA_TCD1_BITER_ELINKYES = majorLoops + 1;                           // Reset value for CITER (must be equal to CITER)

  // Trigger DMA2 (address) after each minor loop
  DMA_TCD1_BITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD1_BITER_ELINKYES |= (0x02 << 9);  
  DMA_TCD1_CITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD1_CITER_ELINKYES |= (0x02 << 9);
}




// TCD2 writes out the address select lines, which are on port D
void setupTCD2(uint8_t* source, int minorLoopSize, int majorLoops) {
  DMA_TCD2_SADDR = source;                                        // Address to read from
  DMA_TCD2_SOFF = 1;                                              // Bytes to increment source register between writes 
  DMA_TCD2_ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);  // 8-bit input and output
  DMA_TCD2_NBYTES_MLNO = minorLoopSize;                           // Number of bytes to transfer in the minor loop
  DMA_TCD2_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
  DMA_TCD2_DADDR = &GPIOD_PDOR;                                   // Address to write to
  DMA_TCD2_DOFF = 0;                                              // Bytes to increment destination register between write
  DMA_TCD2_CITER_ELINKYES = majorLoops;                           // Number of major loops to complete
  DMA_TCD2_BITER_ELINKYES = majorLoops;                           // Reset value for CITER (must be equal to CITER)
  DMA_TCD2_DLASTSGA = 0;                                          // Address of next TCD (N/A)
  
  // Workaround for DMA majorelink unreliability: increase the minor loop count by one
  // Note that the final transfer doesn't end up happening, because 
  DMA_TCD2_CITER_ELINKYES = majorLoops;                           // Number of major loops to complete
  DMA_TCD2_BITER_ELINKYES = majorLoops;                           // Reset value for CITER (must be equal to CITER)

  // Trigger DMA3 (address) after each minor loop
  DMA_TCD2_BITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD2_BITER_ELINKYES |= (0x03 << 9);  
  DMA_TCD2_CITER_ELINKYES |= DMA_TCD_CITER_ELINK;
  DMA_TCD2_CITER_ELINKYES |= (0x03 << 9);
}

// TCD3 clocks and strobes the pixel data, which are on port B
void setupTCD3(uint8_t* source, int minorLoopSize, int majorLoops) {
  DMA_TCD3_SADDR = source;                                        // Address to read from
  DMA_TCD3_SOFF = 1;                                              // Bytes to increment source register between writes 
  DMA_TCD3_ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);  // 8-bit input and output
  DMA_TCD3_NBYTES_MLNO = minorLoopSize;                           // Number of bytes to transfer in the minor loop
  DMA_TCD3_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
  DMA_TCD3_DADDR = &GPIOB_PDOR;                                   // Address to write to
  DMA_TCD3_DOFF = 0;                                              // Bytes to increment destination register between write
  DMA_TCD3_CITER_ELINKNO = majorLoops;                            // Number of major loops to complete
  DMA_TCD3_BITER_ELINKNO = majorLoops;                            // Reset value for CITER (must be equal to CITER)
  DMA_TCD3_DLASTSGA = 0;                                          // Address of next TCD (N/A)
}


// When the last address write has completed, that means we're at the end of the display refresh cycle
// Set up the next display frame
void dma_ch2_isr(void) {
  DMA_CINT = DMA_CINT_CINT(2);

  if(swapBuffers) {
    uint8_t* lastBuffer = frontBuffer;
    frontBuffer = backBuffer;
    backBuffer = lastBuffer;
    swapBuffers = false;
  }
  
  setupTCDs();
}

void setupTCDs() {
  setupTCD0(FTM0_MODStates, 4,                        BIT_DEPTH*LED_ROWS);
  setupTCD1(FTM0_C1VStates, 4,                        BIT_DEPTH*LED_ROWS);
  setupTCD2(Addresses,      ADDRESS_REPEAT_COUNT, BIT_DEPTH*LED_ROWS);
  setupTCD3(frontBuffer+currentPage*PANEL_DEPTH_SIZE,    ROW_BIT_SIZE,             BIT_DEPTH*LED_ROWS);

  currentPage=(currentPage+1)%PAGES;

  DMA_SSRT = DMA_SSRT_SSRT(3);
}


// FTM0 drives our whole operation! We need to periodically update the
// FTM0_MOD and FTM0_C1V registers to program the next cycle.
void setupFTM0(){
  FTM0_MODE = FTM_MODE_WPDIS;    // Disable Write Protect

  FTM0_SC = 0;                   // Turn off the clock so we can update CNTIN and MODULO?
  FTM0_MOD = 0x02FF;             // Period register
  FTM0_SC |= FTM_SC_CLKS(1) | FTM_SC_PS(1);

  FTM0_MODE |= FTM_MODE_INIT;         // Enable FTM0

  FTM0_C1SC = 0x40                    // Enable interrupt
  | 0x20                    // Mode select: Edge-aligned PWM 
  | 0x04                    // Low-true pulses (inverted)
  | 0x01;                   // Enable DMA out
  FTM0_C1V = 0x0200;        // Duty cycle of PWM signal
  FTM0_SYNC |= 0x80;        // set PWM value update


  // Configure LED_OE_PIN pinmux (LED_OE_PIN is on PORTA-4 / FTM0_CH1)
  PORTA_PCR4 = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE; 
}
