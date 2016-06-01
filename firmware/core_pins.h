/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _core_pins_h_
#define _core_pins_h_

#include "mk20dn64.h"
#include "pins_arduino.h"


#define HIGH		1
#define LOW		0
#define INPUT		0
#define OUTPUT		1
#define INPUT_PULLUP	2
#define LSBFIRST	0
#define MSBFIRST	1
#define _BV(n)		(1<<(n))
#define CHANGE		4
#define FALLING		2
#define RISING		3

// Pin				Arduino
//  0   PA0
//  1   PA1                     RXD
//  2   PA2                     TXD
//  3   PA3
//  4   PA4
//  5   PB0
//  6   PB1
//  7   PC1
//  8   PC2
//  9   PC3
// 10   PC4
// 11   PC5                     SCK
// 12   PC6                     MOSI
// 13   PC7                     MISO
// 14   PD4
// 15   PD5
// 16   PD6
// 17   PD7


// not available to user:
//	A18	CRYSTAL
//	A19	CRYSTAL

#define CORE_NUM_TOTAL_PINS     18
#define CORE_NUM_DIGITAL        18
#define CORE_NUM_INTERRUPT      18

#define CORE_PIN0_BIT		0
#define CORE_PIN1_BIT		1
#define CORE_PIN2_BIT		2
#define CORE_PIN3_BIT		3
#define CORE_PIN4_BIT		4
#define CORE_PIN5_BIT		0
#define CORE_PIN6_BIT		1
#define CORE_PIN7_BIT		1
#define CORE_PIN8_BIT		2
#define CORE_PIN9_BIT		3
#define CORE_PIN10_BIT		4
#define CORE_PIN11_BIT		5
#define CORE_PIN12_BIT		6
#define CORE_PIN13_BIT		7
#define CORE_PIN14_BIT		4
#define CORE_PIN15_BIT		5
#define CORE_PIN16_BIT		6
#define CORE_PIN17_BIT		7

#define CORE_PIN0_BITMASK	(1<<(CORE_PIN0_BIT))
#define CORE_PIN1_BITMASK	(1<<(CORE_PIN1_BIT))
#define CORE_PIN2_BITMASK	(1<<(CORE_PIN2_BIT))
#define CORE_PIN3_BITMASK	(1<<(CORE_PIN3_BIT))
#define CORE_PIN4_BITMASK	(1<<(CORE_PIN4_BIT))
#define CORE_PIN5_BITMASK	(1<<(CORE_PIN5_BIT))
#define CORE_PIN6_BITMASK	(1<<(CORE_PIN6_BIT))
#define CORE_PIN7_BITMASK	(1<<(CORE_PIN7_BIT))
#define CORE_PIN8_BITMASK	(1<<(CORE_PIN8_BIT))
#define CORE_PIN9_BITMASK	(1<<(CORE_PIN9_BIT))
#define CORE_PIN10_BITMASK	(1<<(CORE_PIN10_BIT))
#define CORE_PIN11_BITMASK	(1<<(CORE_PIN11_BIT))
#define CORE_PIN12_BITMASK	(1<<(CORE_PIN12_BIT))
#define CORE_PIN13_BITMASK	(1<<(CORE_PIN13_BIT))
#define CORE_PIN14_BITMASK	(1<<(CORE_PIN14_BIT))
#define CORE_PIN15_BITMASK	(1<<(CORE_PIN15_BIT))
#define CORE_PIN16_BITMASK	(1<<(CORE_PIN16_BIT))
#define CORE_PIN17_BITMASK	(1<<(CORE_PIN17_BIT))

#define CORE_PIN0_PORTREG	GPIOA_PDOR
#define CORE_PIN1_PORTREG	GPIOA_PDOR
#define CORE_PIN2_PORTREG	GPIOA_PDOR
#define CORE_PIN3_PORTREG	GPIOA_PDOR
#define CORE_PIN4_PORTREG	GPIOA_PDOR
#define CORE_PIN5_PORTREG	GPIOB_PDOR
#define CORE_PIN6_PORTREG	GPIOB_PDOR
#define CORE_PIN7_PORTREG	GPIOC_PDOR
#define CORE_PIN8_PORTREG	GPIOC_PDOR
#define CORE_PIN9_PORTREG	GPIOC_PDOR
#define CORE_PIN10_PORTREG	GPIOC_PDOR
#define CORE_PIN11_PORTREG	GPIOC_PDOR
#define CORE_PIN12_PORTREG	GPIOC_PDOR
#define CORE_PIN13_PORTREG	GPIOC_PDOR
#define CORE_PIN14_PORTREG	GPIOD_PDOR
#define CORE_PIN15_PORTREG	GPIOD_PDOR
#define CORE_PIN16_PORTREG	GPIOD_PDOR
#define CORE_PIN17_PORTREG	GPIOD_PDOR

#define CORE_PIN0_PORTSET	GPIOA_PSOR
#define CORE_PIN1_PORTSET	GPIOA_PSOR
#define CORE_PIN2_PORTSET	GPIOA_PSOR
#define CORE_PIN3_PORTSET	GPIOA_PSOR
#define CORE_PIN4_PORTSET	GPIOA_PSOR
#define CORE_PIN5_PORTSET	GPIOB_PSOR
#define CORE_PIN6_PORTSET	GPIOB_PSOR
#define CORE_PIN7_PORTSET	GPIOC_PSOR
#define CORE_PIN8_PORTSET	GPIOC_PSOR
#define CORE_PIN9_PORTSET	GPIOC_PSOR
#define CORE_PIN10_PORTSET	GPIOC_PSOR
#define CORE_PIN11_PORTSET	GPIOC_PSOR
#define CORE_PIN12_PORTSET	GPIOC_PSOR
#define CORE_PIN13_PORTSET	GPIOC_PSOR
#define CORE_PIN14_PORTSET	GPIOD_PSOR
#define CORE_PIN15_PORTSET	GPIOD_PSOR
#define CORE_PIN16_PORTSET	GPIOD_PSOR
#define CORE_PIN17_PORTSET	GPIOD_PSOR

#define CORE_PIN0_PORTCLEAR	GPIOA_PCOR
#define CORE_PIN1_PORTCLEAR	GPIOA_PCOR
#define CORE_PIN2_PORTCLEAR	GPIOA_PCOR
#define CORE_PIN3_PORTCLEAR	GPIOA_PCOR
#define CORE_PIN4_PORTCLEAR	GPIOA_PCOR
#define CORE_PIN5_PORTCLEAR	GPIOB_PCOR
#define CORE_PIN6_PORTCLEAR	GPIOB_PCOR
#define CORE_PIN7_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN8_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN9_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN10_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN11_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN12_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN13_PORTCLEAR	GPIOC_PCOR
#define CORE_PIN14_PORTCLEAR	GPIOD_PCOR
#define CORE_PIN15_PORTCLEAR	GPIOD_PCOR
#define CORE_PIN16_PORTCLEAR	GPIOD_PCOR
#define CORE_PIN17_PORTCLEAR	GPIOD_PCOR

#define CORE_PIN0_DDRREG	GPIOA_PDDR
#define CORE_PIN1_DDRREG	GPIOA_PDDR
#define CORE_PIN2_DDRREG	GPIOA_PDDR
#define CORE_PIN3_DDRREG	GPIOA_PDDR
#define CORE_PIN4_DDRREG	GPIOA_PDDR
#define CORE_PIN5_DDRREG	GPIOB_PDDR
#define CORE_PIN6_DDRREG	GPIOB_PDDR
#define CORE_PIN7_DDRREG	GPIOC_PDDR
#define CORE_PIN8_DDRREG	GPIOC_PDDR
#define CORE_PIN9_DDRREG	GPIOC_PDDR
#define CORE_PIN10_DDRREG	GPIOC_PDDR
#define CORE_PIN11_DDRREG	GPIOC_PDDR
#define CORE_PIN12_DDRREG	GPIOC_PDDR
#define CORE_PIN13_DDRREG	GPIOC_PDDR
#define CORE_PIN14_DDRREG	GPIOD_PDDR
#define CORE_PIN15_DDRREG	GPIOD_PDDR
#define CORE_PIN16_DDRREG	GPIOD_PDDR
#define CORE_PIN17_DDRREG	GPIOD_PDDR

#define CORE_PIN0_PINREG	GPIOA_PDIR
#define CORE_PIN1_PINREG	GPIOA_PDIR
#define CORE_PIN2_PINREG	GPIOA_PDIR
#define CORE_PIN3_PINREG	GPIOA_PDIR
#define CORE_PIN4_PINREG	GPIOA_PDIR
#define CORE_PIN5_PINREG	GPIOB_PDIR
#define CORE_PIN6_PINREG	GPIOB_PDIR
#define CORE_PIN7_PINREG	GPIOC_PDIR
#define CORE_PIN8_PINREG	GPIOC_PDIR
#define CORE_PIN9_PINREG	GPIOC_PDIR
#define CORE_PIN10_PINREG	GPIOC_PDIR
#define CORE_PIN11_PINREG	GPIOC_PDIR
#define CORE_PIN12_PINREG	GPIOC_PDIR
#define CORE_PIN13_PINREG	GPIOC_PDIR
#define CORE_PIN14_PINREG	GPIOD_PDIR
#define CORE_PIN15_PINREG	GPIOD_PDIR
#define CORE_PIN16_PINREG	GPIOD_PDIR
#define CORE_PIN17_PINREG	GPIOD_PDIR

#define CORE_PIN0_CONFIG	PORTA_PCR0
#define CORE_PIN1_CONFIG	PORTA_PCR1
#define CORE_PIN2_CONFIG	PORTA_PCR2
#define CORE_PIN3_CONFIG	PORTA_PCR3
#define CORE_PIN4_CONFIG	PORTA_PCR4
#define CORE_PIN5_CONFIG	PORTB_PCR0
#define CORE_PIN6_CONFIG	PORTB_PCR1
#define CORE_PIN7_CONFIG	PORTC_PCR1
#define CORE_PIN8_CONFIG	PORTC_PCR2
#define CORE_PIN9_CONFIG	PORTC_PCR3
#define CORE_PIN10_CONFIG	PORTC_PCR4
#define CORE_PIN11_CONFIG	PORTC_PCR5
#define CORE_PIN12_CONFIG	PORTC_PCR6
#define CORE_PIN13_CONFIG	PORTC_PCR7
#define CORE_PIN14_CONFIG	PORTD_PCR4
#define CORE_PIN15_CONFIG	PORTD_PCR5
#define CORE_PIN16_CONFIG	PORTD_PCR6
#define CORE_PIN17_CONFIG	PORTD_PCR7

// TODO: These need to be corrected for this hardware.
#define CORE_ADC0_PIN		14
#define CORE_ADC1_PIN		15
#define CORE_ADC2_PIN		16
#define CORE_ADC3_PIN		17
#define CORE_ADC4_PIN		18
#define CORE_ADC5_PIN		19
#define CORE_ADC6_PIN		20
#define CORE_ADC7_PIN		21
#define CORE_ADC8_PIN		22
#define CORE_ADC9_PIN		23
#define CORE_ADC10_PIN		34
#define CORE_ADC11_PIN		35
#define CORE_ADC12_PIN		36
#define CORE_ADC13_PIN		37

#define CORE_RXD0_PIN		1
#define CORE_TXD0_PIN		2

#define CORE_INT0_PIN		0
#define CORE_INT1_PIN		1
#define CORE_INT2_PIN		2
#define CORE_INT3_PIN		3
#define CORE_INT4_PIN		4
#define CORE_INT5_PIN		5
#define CORE_INT6_PIN		6
#define CORE_INT7_PIN		7
#define CORE_INT8_PIN		8
#define CORE_INT9_PIN		9
#define CORE_INT10_PIN		10
#define CORE_INT11_PIN		11
#define CORE_INT12_PIN		12
#define CORE_INT13_PIN		13
#define CORE_INT14_PIN		14
#define CORE_INT15_PIN		15
#define CORE_INT16_PIN		16
#define CORE_INT17_PIN		17
#define CORE_INT_EVERY_PIN	1




#ifdef __cplusplus
extern "C" {
#endif

void digitalWrite(uint8_t pin, uint8_t val);
static inline void digitalWriteFast(uint8_t pin, uint8_t val) __attribute__((always_inline, unused));
static inline void digitalWriteFast(uint8_t pin, uint8_t val)
{
	if (__builtin_constant_p(pin)) {
		if (val) {
			if (pin == 0) {
				CORE_PIN0_PORTSET = CORE_PIN0_BITMASK;
			} else if (pin == 1) {
				CORE_PIN1_PORTSET = CORE_PIN1_BITMASK;
			} else if (pin == 2) {
				CORE_PIN2_PORTSET = CORE_PIN2_BITMASK;
			} else if (pin == 3) {
				CORE_PIN3_PORTSET = CORE_PIN3_BITMASK;
			} else if (pin == 4) {
				CORE_PIN4_PORTSET = CORE_PIN4_BITMASK;
			} else if (pin == 5) {
				CORE_PIN5_PORTSET = CORE_PIN5_BITMASK;
			} else if (pin == 6) {
				CORE_PIN6_PORTSET = CORE_PIN6_BITMASK;
			} else if (pin == 7) {
				CORE_PIN7_PORTSET = CORE_PIN7_BITMASK;
			} else if (pin == 8) {
				CORE_PIN8_PORTSET = CORE_PIN8_BITMASK;
			} else if (pin == 9) {
				CORE_PIN9_PORTSET = CORE_PIN9_BITMASK;
			} else if (pin == 10) {
				CORE_PIN10_PORTSET = CORE_PIN10_BITMASK;
			} else if (pin == 11) {
				CORE_PIN11_PORTSET = CORE_PIN11_BITMASK;
			} else if (pin == 12) {
				CORE_PIN12_PORTSET = CORE_PIN12_BITMASK;
			} else if (pin == 13) {
				CORE_PIN13_PORTSET = CORE_PIN13_BITMASK;
			} else if (pin == 14) {
				CORE_PIN14_PORTSET = CORE_PIN14_BITMASK;
			} else if (pin == 15) {
				CORE_PIN15_PORTSET = CORE_PIN15_BITMASK;
			} else if (pin == 16) {
				CORE_PIN16_PORTSET = CORE_PIN16_BITMASK;
			} else if (pin == 17) {
				CORE_PIN17_PORTSET = CORE_PIN17_BITMASK;
			}
		} else {
			if (pin == 0) {
				CORE_PIN0_PORTCLEAR = CORE_PIN0_BITMASK;
			} else if (pin == 1) {
				CORE_PIN1_PORTCLEAR = CORE_PIN1_BITMASK;
			} else if (pin == 2) {
				CORE_PIN2_PORTCLEAR = CORE_PIN2_BITMASK;
			} else if (pin == 3) {
				CORE_PIN3_PORTCLEAR = CORE_PIN3_BITMASK;
			} else if (pin == 4) {
				CORE_PIN4_PORTCLEAR = CORE_PIN4_BITMASK;
			} else if (pin == 5) {
				CORE_PIN5_PORTCLEAR = CORE_PIN5_BITMASK;
			} else if (pin == 6) {
				CORE_PIN6_PORTCLEAR = CORE_PIN6_BITMASK;
			} else if (pin == 7) {
				CORE_PIN7_PORTCLEAR = CORE_PIN7_BITMASK;
			} else if (pin == 8) {
				CORE_PIN8_PORTCLEAR = CORE_PIN8_BITMASK;
			} else if (pin == 9) {
				CORE_PIN9_PORTCLEAR = CORE_PIN9_BITMASK;
			} else if (pin == 10) {
				CORE_PIN10_PORTCLEAR = CORE_PIN10_BITMASK;
			} else if (pin == 11) {
				CORE_PIN11_PORTCLEAR = CORE_PIN11_BITMASK;
			} else if (pin == 12) {
				CORE_PIN12_PORTCLEAR = CORE_PIN12_BITMASK;
			} else if (pin == 13) {
				CORE_PIN13_PORTCLEAR = CORE_PIN13_BITMASK;
			} else if (pin == 14) {
				CORE_PIN14_PORTCLEAR = CORE_PIN14_BITMASK;
			} else if (pin == 15) {
				CORE_PIN15_PORTCLEAR = CORE_PIN15_BITMASK;
			} else if (pin == 16) {
				CORE_PIN16_PORTCLEAR = CORE_PIN16_BITMASK;
			} else if (pin == 17) {
				CORE_PIN17_PORTCLEAR = CORE_PIN17_BITMASK;
			}
		}
	} else {
		if (val) {
			*portSetRegister(pin) = 1;
		} else {
			*portClearRegister(pin) = 1;
		}
	}
}

uint8_t digitalRead(uint8_t pin);
static inline uint8_t digitalReadFast(uint8_t pin) __attribute__((always_inline, unused));
static inline uint8_t digitalReadFast(uint8_t pin)
{
	if (__builtin_constant_p(pin)) {
		if (pin == 0) {
			return (CORE_PIN0_PINREG & CORE_PIN0_BITMASK) ? 1 : 0;
		} else if (pin == 1) {
			return (CORE_PIN1_PINREG & CORE_PIN1_BITMASK) ? 1 : 0;
		} else if (pin == 2) {
			return (CORE_PIN2_PINREG & CORE_PIN2_BITMASK) ? 1 : 0;
		} else if (pin == 3) {
			return (CORE_PIN3_PINREG & CORE_PIN3_BITMASK) ? 1 : 0;
		} else if (pin == 4) {
			return (CORE_PIN4_PINREG & CORE_PIN4_BITMASK) ? 1 : 0;
		} else if (pin == 5) {
			return (CORE_PIN5_PINREG & CORE_PIN5_BITMASK) ? 1 : 0;
		} else if (pin == 6) {
			return (CORE_PIN6_PINREG & CORE_PIN6_BITMASK) ? 1 : 0;
		} else if (pin == 7) {
			return (CORE_PIN7_PINREG & CORE_PIN7_BITMASK) ? 1 : 0;
		} else if (pin == 8) {
			return (CORE_PIN8_PINREG & CORE_PIN8_BITMASK) ? 1 : 0;
		} else if (pin == 9) {
			return (CORE_PIN9_PINREG & CORE_PIN9_BITMASK) ? 1 : 0;
		} else if (pin == 10) {
			return (CORE_PIN10_PINREG & CORE_PIN10_BITMASK) ? 1 : 0;
		} else if (pin == 11) {
			return (CORE_PIN11_PINREG & CORE_PIN11_BITMASK) ? 1 : 0;
		} else if (pin == 12) {
			return (CORE_PIN12_PINREG & CORE_PIN12_BITMASK) ? 1 : 0;
		} else if (pin == 13) {
			return (CORE_PIN13_PINREG & CORE_PIN13_BITMASK) ? 1 : 0;
		} else if (pin == 14) {
			return (CORE_PIN14_PINREG & CORE_PIN14_BITMASK) ? 1 : 0;
		} else if (pin == 15) {
			return (CORE_PIN15_PINREG & CORE_PIN15_BITMASK) ? 1 : 0;
		} else if (pin == 16) {
			return (CORE_PIN16_PINREG & CORE_PIN16_BITMASK) ? 1 : 0;
		} else if (pin == 17) {
			return (CORE_PIN17_PINREG & CORE_PIN17_BITMASK) ? 1 : 0;
		} else {
			return 0;
		}
	} else {
		return *portInputRegister(pin);
	}
}


void pinMode(uint8_t pin, uint8_t mode);
void init_pins(void);
void analogWrite(uint8_t pin, int val);
void analogWriteRes(uint32_t bits);
static inline void analogWriteResolution(uint32_t bits) { analogWriteRes(bits); }
void analogWriteFrequency(uint8_t pin, uint32_t frequency);
void analogWriteDAC0(int val);
void attachInterrupt(uint8_t pin, void (*function)(void), int mode);
void detachInterrupt(uint8_t pin);
void _init_Teensyduino_internal_(void);

int analogRead(uint8_t pin);
void analogReference(uint8_t type);
void analogReadRes(unsigned int bits);
static inline void analogReadResolution(unsigned int bits) { analogReadRes(bits); }
void analogReadAveraging(unsigned int num);
void analog_init(void);

#define DEFAULT         0
#define INTERNAL        2
#define INTERNAL1V2     2
#define INTERNAL1V1     2
#define EXTERNAL        0

int touchRead(uint8_t pin);


static inline void shiftOut(uint8_t, uint8_t, uint8_t, uint8_t) __attribute__((always_inline, unused));
extern void _shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value) __attribute__((noinline));
extern void shiftOut_lsbFirst(uint8_t dataPin, uint8_t clockPin, uint8_t value) __attribute__((noinline));
extern void shiftOut_msbFirst(uint8_t dataPin, uint8_t clockPin, uint8_t value) __attribute__((noinline));

static inline void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t value)
{
        if (__builtin_constant_p(bitOrder)) {
                if (bitOrder == LSBFIRST) {
                        shiftOut_lsbFirst(dataPin, clockPin, value);
                } else {
                        shiftOut_msbFirst(dataPin, clockPin, value);
                }
        } else {
                _shiftOut(dataPin, clockPin, bitOrder, value);
        }
}

static inline uint8_t shiftIn(uint8_t, uint8_t, uint8_t) __attribute__((always_inline, unused));
extern uint8_t _shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) __attribute__((noinline));
extern uint8_t shiftIn_lsbFirst(uint8_t dataPin, uint8_t clockPin) __attribute__((noinline));
extern uint8_t shiftIn_msbFirst(uint8_t dataPin, uint8_t clockPin) __attribute__((noinline));

static inline uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
        if (__builtin_constant_p(bitOrder)) {
                if (bitOrder == LSBFIRST) {
                        return shiftIn_lsbFirst(dataPin, clockPin);
                } else {
                        return shiftIn_msbFirst(dataPin, clockPin);
                }
        } else {
                return _shiftIn(dataPin, clockPin, bitOrder);
        }
}

void _reboot_Teensyduino_(void) __attribute__((noreturn));
void _restart_Teensyduino_(void) __attribute__((noreturn));

void yield(void);

void delay(uint32_t msec);

extern volatile uint32_t systick_millis_count;

static inline uint32_t millis(void) __attribute__((always_inline, unused));
static inline uint32_t millis(void)
{
	volatile uint32_t ret = systick_millis_count; // single aligned 32 bit is atomic;
	return ret;
}

uint32_t micros(void);

static inline void delayMicroseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayMicroseconds(uint32_t usec)
{
#if F_CPU == 168000000
	uint32_t n = usec * 56;
#elif F_CPU == 144000000
	uint32_t n = usec * 48;
#elif F_CPU == 120000000
	uint32_t n = usec * 40;
#elif F_CPU == 96000000
	uint32_t n = usec << 5;
#elif F_CPU == 72000000
	uint32_t n = usec * 24;
#elif F_CPU == 48000000
	uint32_t n = usec << 4;
#elif F_CPU == 24000000
	uint32_t n = usec << 3;
#elif F_CPU == 16000000
	uint32_t n = usec << 2;
#elif F_CPU == 8000000
	uint32_t n = usec << 1;
#elif F_CPU == 4000000
	uint32_t n = usec;
#elif F_CPU == 2000000
	uint32_t n = usec >> 1;
#endif
    // changed because a delay of 1 micro Sec @ 2MHz will be 0
	if (n == 0) return;
	__asm__ volatile(
		"L_%=_delayMicroseconds:"		"\n\t"
#if F_CPU < 24000000
		"nop"					"\n\t"
#endif
		"subs   %0, #1"				"\n\t"
		"bne    L_%=_delayMicroseconds"		"\n"
		: "+r" (n) :
	);
}

#ifdef __cplusplus
}
#endif








#ifdef __cplusplus
extern "C" {
#endif
unsigned long rtc_get(void);
void rtc_set(unsigned long t);
void rtc_compensate(int adjust);
#ifdef __cplusplus
}
class teensy3_clock_class
{
public:
	static unsigned long get(void) __attribute__((always_inline)) { return rtc_get(); }
	static void set(unsigned long t) __attribute__((always_inline)) { rtc_set(t); }
	static void compensate(int adj) __attribute__((always_inline)) { rtc_compensate(adj); }
};
extern teensy3_clock_class Teensy3Clock;
#endif




#endif
