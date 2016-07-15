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

Matrix matrix;

// Offsets in the port C register (data)
#define DMA_LED_HS_EN_SHIFT 1 // Location of the LED_HS_EN pin in the Port C register
#define DMA_CLK_SHIFT 5 // Location of the clock pin in Port C register
#define DMA_DAT_SHIFT 6 // Location of the data pin in Port C register

// Offsets in the port D register (address)
#define DMA_STB_SHIFT 4 // Location of the strobe pin in Port D register
#define DMA_S0_SHIFT 5 // Note that S1 and S2 have to follow S0 sequentially.
#define DMA_S1_SHIFT 6
#define DMA_S2_SHIFT 7

// Bit output order mapping for the LEDs
// This is a table to map the input color data (which is stored as RGB triplets)
// to the output bitstream (which is hardware dependent). This is required to
// maintain flexibility in the hardware design.
uint8_t OUTPUT_ORDER[] = {
    16, // R0
    0, // G0
    8, // B0
    17, // R1
    1, // G1
    9, // B1
    18, // R2
    2, // G2
    10, // B2
    19, // R3
    3, // G3
    11, // B3
    20, // R4
    4, // G4
    12, // B4
    21, // R5
    5, // G5
    13, // B5
    22, // R6
    6, // G6
    14, // B6
    23, // R7
    7, // G7
    15, // B7
};

void dma_ch2_isr(void);

Matrix::Matrix() {
    brightness = 1;
}

void Matrix::begin() {
    // Set all the pins to outputs
    digitalWrite(LED_OE_PIN, HIGH);
    pinMode(LED_OE_PIN, OUTPUT);

    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);

    pinMode(LED_DATA_PIN, OUTPUT);
    pinMode(LED_CLOCK_PIN, OUTPUT);
    pinMode(LED_STROBE_PIN, OUTPUT);

    pinMode(LED_HS_EN_PIN, OUTPUT);
    digitalWrite(LED_HS_EN_PIN, HIGH);

    buildAddressTable();
    buildTimerTables();
 
    // DMA
    // Configure DMA
    SIM_SCGC7 |= SIM_SCGC7_DMA;  // Enable DMA clock
    DMA_CR = 0;  // Use default configuration
 
    // Configure the DMA request input for DMA0
    DMA_SERQ = DMA_SERQ_SERQ(0);
 
    // Enable interrupt on major completion for DMA channel 3 (address)
    DMA_TCD2_CSR = DMA_TCD_CSR_INTMAJOR;  // Enable interrupt on major complete
    NVIC_ENABLE_IRQ(IRQ_DMA_CH2);         // Enable interrupt request
 
    // DMAMUX
    // Configure the DMAMUX
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX; // Enable DMAMUX clock
 
    // Configure DMAMUX to trigger DMA0 from FTM0_CH0
    DMAMUX0_CHCFG0 = DMAMUX_DISABLE;
    DMAMUX0_CHCFG0 = DMAMUX_SOURCE_FTM0_CH0 | DMAMUX_ENABLE;

 
    // SPI
    setupSPI0();
    
    frontBuffer = dmaBufferSpi[0];
    backBuffer = dmaBufferSpi[1];

    swapBuffers = false;
    currentPage = 0;

    // Clear the display and kick off transmission
    memset(pixels, 0, LED_ROWS*LED_COLS*BYTES_PER_PIXEL);
    show();

    setupTCD0();
    setupTCD1();
    setupTCD2();
    setupTCD3();

    // Load this frame of data into the DMA engine
    refresh();

    // FTM
    setupFTM0();
}

bool Matrix::bufferWaiting() const {
    return swapBuffers;
}

void Matrix::show() {
    if(swapBuffers) {
        return;
    }
    
    pixelsToDmaBuffer(pixels, backBuffer);

    // TODO: Atomic operation?
    swapBuffers = true;
}

void Matrix::setPixelColor(uint8_t column, uint8_t row, uint8_t r, uint8_t g, uint8_t b) {
    // Don't do anything if the pixel is out of range
    if (column >= LED_COLS || row >= LED_ROWS) {
        return;
    }

    Pixel* pixel = &pixels[row*LED_COLS + column];

    pixel->R = r;
    pixel->G = g;
    pixel->B = b;
}

void Matrix::setPixelColor(uint8_t column, uint8_t row, const Pixel& pixel) {
    pixels[row*LED_COLS + column] = pixel;
}

Pixel* Matrix::getPixels() {
    return pixels;
}

void Matrix::setBrightness(float brightness) {
    this->brightness = brightness;
}

float Matrix::getBrightness() const {
    return brightness;
}

// Munge the data so it can be written out by the DMA engine
// Note: buffer[][xxx] should have BIT_DEPTH as xxx
void Matrix::pixelsToDmaBuffer(Pixel* pixelInput, dmaBuffer_t buffer[]) {
    // First, clear the outputs
    memset(buffer, 0, PANEL_DEPTH_SPI_SIZE*PAGES*sizeof(dmaBuffer_t));

    for(uint8_t page = 0; page < PAGES; page++) {
        for(uint8_t row = 0; row < LED_ROWS; row++) {
            for(uint8_t col = 0; col < LED_COLS; col++) {
   
		Pixel* pixel = &pixelInput[row*LED_COLS + col];
                uint16_t dataR = brightnessTable[(uint16_t)(brightness*(pixel->R))];
                uint16_t dataG = brightnessTable[(uint16_t)(brightness*(pixel->G))];
                uint16_t dataB = brightnessTable[(uint16_t)(brightness*(pixel->B))];
  
                uint8_t offsetR = OUTPUT_ORDER[col*BYTES_PER_PIXEL + 0];
                uint8_t offsetG = OUTPUT_ORDER[col*BYTES_PER_PIXEL + 1];
                uint8_t offsetB = OUTPUT_ORDER[col*BYTES_PER_PIXEL + 2];

                for(uint8_t depth = 0; depth < BIT_DEPTH; depth++) {
                    uint8_t bitR = ((dataR >> (depth)) & 0x01);
                    uint8_t bitG = ((dataG >> (depth)) & 0x01);
                    uint8_t bitB = ((dataB >> (depth)) & 0x01);
                   
                    uint32_t offsetBase = row*ROW_DEPTH_SPI_SIZE + depth*WRITES_PER_COLUMN_SPI;
                   
                    buffer[offsetBase + (offsetR/BITS_PER_WRITE_SPI)] |= (bitR << (offsetR % BITS_PER_WRITE_SPI));
                    buffer[offsetBase + (offsetG/BITS_PER_WRITE_SPI)] |= (bitG << (offsetG % BITS_PER_WRITE_SPI));
                    buffer[offsetBase + (offsetB/BITS_PER_WRITE_SPI)] |= (bitB << (offsetB % BITS_PER_WRITE_SPI));
                }
            }
        }
    }
}

void Matrix::setupTCD0() {
//    DMA_TCD0_SADDR = source;                                      // Address to read from
    DMA_TCD0_SOFF = 2;                                              // Bytes to increment source register between writes 
    DMA_TCD0_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);  // 16-bit input and output
    DMA_TCD0_NBYTES_MLNO = 2;                                       // Number of bytes to transfer in the minor loop
    DMA_TCD0_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
    DMA_TCD0_DADDR = &FTM0_MOD;                                     // Address to write to
    DMA_TCD0_DOFF = 0;                                              // Bytes to increment destination register between write
    DMA_TCD0_DLASTSGA = 0;                                          // Address of next TCD (N/A)

}

void Matrix::setupTCD1() {
//    DMA_TCD1_SADDR = source;                                        // Address to read from
    DMA_TCD1_SOFF = 2;                                              // Bytes to increment source register between writes 
    DMA_TCD1_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);  // 16-bit input and output
    DMA_TCD1_NBYTES_MLNO = 2;                                       // Number of bytes to transfer in the minor loop
    DMA_TCD1_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
    DMA_TCD1_DADDR = &FTM0_C1V;                                     // Address to write to
    DMA_TCD1_DOFF = 0;                                              // Bytes to increment destination register between write
    DMA_TCD1_DLASTSGA = 0;                                          // Address of next TCD (N/A)
}

void Matrix::setupTCD2() {
    DMA_TCD2_SOFF = 1;                                              // Bytes to increment source register between writes 
    DMA_TCD2_ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);  // 8-bit input and output
    DMA_TCD2_NBYTES_MLNO = ADDRESS_REPEAT_COUNT;                           // Number of bytes to transfer in the minor loop
    DMA_TCD2_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
    DMA_TCD2_DADDR = &GPIOD_PDOR;                                   // Address to write to
    DMA_TCD2_DOFF = 0;                                              // Bytes to increment destination register between write
    DMA_TCD2_DLASTSGA = 0;                                          // Address of next TCD (N/A)
}

void Matrix::setupTCD3() {
    DMA_TCD3_SOFF = 2;                                              // Bytes to increment source register between writes 
    DMA_TCD3_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);  // 16-bit input and output
    DMA_TCD3_NBYTES_MLNO = WRITES_PER_COLUMN_SPI*2;                 // Number of bytes to transfer in the minor loop
    DMA_TCD3_SLAST = 0;                                             // Bytes to add after a major iteration count (N/A)
    DMA_TCD3_DADDR = &SPI0_PUSHR;                                   // Address to write to
    DMA_TCD3_DOFF = 0;                                              // Bytes to increment destination register between write
    DMA_TCD3_DLASTSGA = 0;                                          // Address of next TCD (N/A)
}


// TCD0 updates the timer values for FTM0
void Matrix::armTCD0(void* source, int majorLoops) {
    DMA_TCD0_SADDR = source;                                        // Address to read from
 
    // Workaround for DMA majorelink unreliability: increase the minor loop count by one
    // Note that the final transfer doesn't end up happening, since this descriptor will be
    // overwritten by the interrupt routine.
    DMA_TCD0_CITER_ELINKYES = majorLoops;                           // Number of major loops to complete
    DMA_TCD0_BITER_ELINKYES = majorLoops;                           // Reset value for CITER (must be equal to CITER)

    // Trigger DMA1 (timer) after each minor loop
    DMA_TCD0_BITER_ELINKYES |= DMA_TCD_CITER_ELINK;
    DMA_TCD0_BITER_ELINKYES |= (0x01 << 9);  
    DMA_TCD0_CITER_ELINKYES |= DMA_TCD_CITER_ELINK;
    DMA_TCD0_CITER_ELINKYES |= (0x01 << 9);
}

// TCD1 updates the timer values for FTM0
void Matrix::armTCD1(void* source, int majorLoops) {
    DMA_TCD1_SADDR = source;                                        // Address to read from
 
    // Workaround for DMA majorelink unreliability: increase the minor loop count by one
    // Note that the final transfer doesn't end up happening, since this descriptor will be
    // overwritten by the interrupt routine.
    DMA_TCD1_CITER_ELINKYES = majorLoops;                           // Number of major loops to complete
    DMA_TCD1_BITER_ELINKYES = majorLoops;                           // Reset value for CITER (must be equal to CITER)
 
    // Trigger DMA2 (address) after each minor loop
    DMA_TCD1_BITER_ELINKYES |= DMA_TCD_CITER_ELINK | (0x02 << 9);  
    DMA_TCD1_CITER_ELINKYES |= DMA_TCD_CITER_ELINK | (0x02 << 9);
}


// TCD2 writes out the address select lines, which are on port D
void Matrix::armTCD2(void* source, int majorLoops) {
    DMA_TCD2_SADDR = source;                                        // Address to read from
    DMA_TCD2_CITER_ELINKYES = majorLoops;                           // Number of major loops to complete
    DMA_TCD2_BITER_ELINKYES = majorLoops;                           // Reset value for CITER (must be equal to CITER)

    // Trigger DMA3 (data) after each minor loop
    DMA_TCD2_BITER_ELINKYES |= DMA_TCD_CITER_ELINK | (0x03 << 9);  
    DMA_TCD2_CITER_ELINKYES |= DMA_TCD_CITER_ELINK | (0x03 << 9);
}


// TCD3 writes bytes out to the SPI TX FIFO
void Matrix::armTCD3(void* source, int majorLoops) {
    DMA_TCD3_SADDR = source;                                        // Address to read from
    DMA_TCD3_CITER_ELINKNO = majorLoops;                            // Number of major loops to complete
    DMA_TCD3_BITER_ELINKNO = majorLoops;                            // Reset value for CITER (must be equal to CITER)
}


void Matrix::buildAddressTable() {
    // Fill the address table
    // To make the DMA engine easier to program, we store a copy of the
    // address table for each output page.
    for(int address = 0; address < LED_ROWS; address++) {
        for(int depth = 0; depth < BIT_DEPTH; depth++) {
            int last_address;
            if(depth == 0) {
                last_address = (address + LED_ROWS - 1)%(LED_ROWS);
            }
            else {
                last_address = address;
            }

            // direct address select lines
            //#define addressBits(addr) (~((1<<DMA_STB_SHIFT) | (1<<(DMA_S0_SHIFT + addr))))

            // Mux-based address select lines
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
}

void Matrix::buildTimerTables() {
    // Fill the timer states table
    for(int address = 0; address < LED_ROWS; address++) {

        // TODO: These comments need to be updated.
        // Each row update consists of BIT_DEPTH cycles. The length of the 'on' time
        // (when OE is asserted) on each cycle is set by onTime; it begins with
        // ON_TIME_MIN and doubles every cycle after that to create a binary progression.

        // The interval between OE cycle is set by one of the three cases:
        // 1. For low bits, where onTime is small, the interval is expanded to MIN_CYCLE_TIME. This is to give time for the next levels data to be shifted out
        // 2. For longer bits, where onTime is longer, the cycle time is calculated as onTime + MIN_BLANKING_TIME
        // 3. For the last cycle of the last row, the cycle time is expanded to MIN_LAST_CYCLE_TIME to allow
        //    the display interrupt to update.
 
        #define MIN_CYCLE_TIME          0x0059      // Minimum time between OE assertions
        #define MIN_BLANKING_TIME       0x0030      // Minimum time OE must be unasserted
        #define MIN_LAST_CYCLE_TIME     0x0059      // Mininum number of cycles for the last cycle loop.
 
 
        int onTime = 0;
 
        for(int depth= 0; depth< BIT_DEPTH; depth++) {

            if((address == LED_ROWS -1)
                 && (depth == BIT_DEPTH - 1)
                 && ((onTime + MIN_BLANKING_TIME) < MIN_LAST_CYCLE_TIME)) {
                // On the last cycle, we need enough time to flush the DMA engines and handle the
                // interrupt to reset the DMA engines. If the combination of blanking time and
                // on time don't meet this, increase the timer cycle count to an acceptable length.
                FTM0_C1VStates[address*BIT_DEPTH + depth] = MIN_LAST_CYCLE_TIME - onTime;
                FTM0_MODStates[address*BIT_DEPTH + depth] = MIN_LAST_CYCLE_TIME;
            }      
            else if((onTime + MIN_BLANKING_TIME) < MIN_CYCLE_TIME) {
                // The DMA engines need enough time to write out the data after every cycle.
                // WHen the on time is really low, the combination of blanking time and
                // on time might not create a long enough delay to meet this, so we need to increase
                // the timer cycle count to meet this requirement.
                FTM0_C1VStates[address*BIT_DEPTH + depth] = MIN_CYCLE_TIME - onTime;
                FTM0_MODStates[address*BIT_DEPTH + depth] = MIN_CYCLE_TIME;
            }
            else {
                FTM0_C1VStates[address*BIT_DEPTH + depth] = MIN_BLANKING_TIME;
                FTM0_MODStates[address*BIT_DEPTH + depth] = MIN_BLANKING_TIME + onTime;
            }
            
            if(onTime == 0) {
                onTime = 1;
            }
            else {   
                onTime *= 2;
            }
        }
    }
}

void Matrix::refresh() {
    if(swapBuffers) {
        dmaBuffer_t* lastBuffer = frontBuffer;
        frontBuffer = backBuffer;
        backBuffer = lastBuffer;
        swapBuffers = false;
    }
    
    armTCD1(FTM0_C1VStates+1, BIT_DEPTH*LED_ROWS);
    armTCD0(FTM0_MODStates+1, BIT_DEPTH*LED_ROWS);
    armTCD2(Addresses, BIT_DEPTH*LED_ROWS);
    armTCD3(frontBuffer+currentPage*PANEL_DEPTH_SPI_SIZE, BIT_DEPTH*LED_ROWS);

    //currentPage=(currentPage+1)%PAGES;

    // Write out the first bit state
//    DMA_SSRT = DMA_SSRT_SSRT(0);
}


// When the last address write has completed, that means we're at the end of the display refresh cycle
// Set up the next display frame
void dma_ch2_isr(void) {
    DMA_CINT = DMA_CINT_CINT(2);

    // Stop the timer, just to see what happens
    FTM0_SC = 0;

    matrix.refresh();

    // TODO
    matrix.setupFTM0();
}

// FTM0 drives our whole operation! We need to periodically update the
// FTM0_MOD and FTM0_C1V registers to program the next cycle.
void Matrix::setupFTM0(){
    SIM_SCGC6 |= SIM_SCGC6_FTM0;  // Enable FTM0 clock

    FTM0_MODE = FTM_MODE_WPDIS;    // Disable Write Protect

    FTM0_SC = 0;        // Make sure the clock is stopped

    // pre-set the clock
    FTM0_CNTIN = 0;         // Start at 0
    FTM0_CNT = 0;
    FTM0_MOD = 200;         // Set the mod to something high so that it can be rewritten
    FTM0_C1V = FTM0_MOD;    // and C1V to something larger than MOD so it will not be reached


    // FTM0_CH0 triggers timer update/address/strobe writes
    FTM0_C0SC = 0x40        // Enable interrupt
        | 0x20              // Mode select: Edge-aligned PWM 
        | 0x08              // High-true pulses
        | 0x01;             // Enable DMA out
    FTM0_SYNC |= 0x80;      // set PWM value update
 
    FTM0_C0V = 0x001;      // Duty cycle of PWM signal

 
    // FTM0_CH1 drives the LED_OE pin and triggers data write
    FTM0_C1SC = 0x40        // Enable interrupt
        | 0x20              // Mode select: Edge-aligned PWM 
        | 0x08              // High-true pulses
        | 0x01;             // Enable DMA out
    FTM0_SYNC |= 0x80;      // set PWM value update


    // And start the clock
    FTM0_SC |=
        FTM_SC_CLKS(1)    // Use system clock source
        | FTM_SC_PS(0);     // Divide by 1 prescaler

    // Configure LED_OE_PIN pinmux (LED_OE_PIN is on PORTA-4 / FTM0_CH1)
    PORTA_PCR4 = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE; 
 
}

// SPI is used to program the column drivers
void Matrix::setupSPI0() {
    SIM_SCGC6 |= SIM_SCGC6_SPI0;    // Enable SPI0 clock

    // Configure SCK and SOUT pins for SPI use (SIN is not used)
    PORTC_PCR5 = PORT_PCR_DSE | PORT_PCR_MUX(2);
    PORTC_PCR6 = PORT_PCR_DSE | PORT_PCR_MUX(2);

    SPI0_MCR = SPI_MCR_MSTR;    // Master mode
    SPI0_MCR |= SPI_MCR_CLR_RXF | SPI_MCR_CLR_TXF;  // Clear the FIFOs

    SPI0_CTAR0 = SPI_CTAR_DBR   // Double baud rate
        | SPI_CTAR_FMSZ(BITS_PER_WRITE_SPI - 1)     // n-bit mode
        | SPI_CTAR_LSBFE;                          // least significant bit first
}
