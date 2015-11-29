/*------------------------------------------------------------------------
  Bitbang driver for ATtiny85+TLC5940 using only three pins
------------------------------------------------------------------------*/
#ifndef _TEENY5940_H
#define _TEENY5940_H

#if defined (__AVR_ATtiny85__)
/*------------------------------------------------------------------------
  Configuration
------------------------------------------------------------------------*/
// How many TLC chips are attached. Each chip needs 32 bytes of RAM.
// The ATtiny85 only has 512 bytes of RAM and you need some left over
// for the rest of your program. Do the math...
#ifndef NUM_TLC5940s
#define NUM_TLC5940s 1
#endif

/*------------------------------------------------------------------------
  Pin assignments
------------------------------------------------------------------------*/
// You could move the BLANK_XLAT_PIN if you wanted to
// but the USI pins are fixed.
#define BLANK_XLAT_PIN PB0
#define USI_OUT_PIN PB1
#define USI_CLK_PIN PB2

/*------------------------------------------------------------------------
  The TLC5940 controller
------------------------------------------------------------------------*/
#define NUM_LEDS (16*NUM_TLC5940s)   /* 16 LEDs per chip */

class Teeny5940 {
  typedef uint8_t byte;
  typedef uint16_t pwm_val;
  pwm_val pwmVals[NUM_LEDS];
public:
  // ----------------------------------------
  // Initialize - call this in your "setup()"
  // ----------------------------------------
  void init() {
    DDRB |= _BV(BLANK_XLAT_PIN);    // Set pin as 'output'
    PORTB |= _BV(BLANK_XLAT_PIN);   // BLANK pin high (stop the TLC5940, disable all output)
    // Set all PWM values to zero
    clear();
  }
  
  // --------------------------
  // Set all PWM values to zero
  // --------------------------
  void clear() {
    byte *p = (byte*)pwmVals;
    byte *e = p + (NUM_LEDS*sizeof(pwm_val));
    while (p!=e) {
      *p++ = 0;
    }
  }
  
  // ------------------------------------------------------------------------
  // Set the PWM value of one of the PWM channels, 'value' in range [0..4095]
  //
  // eg. "set(2,2048)" sets LED 2 to 50% on, 50% off
  // ------------------------------------------------------------------------
  void set(byte channel, pwm_val value) {
    if ((channel&1)!=0) {
      pwmVals[channel] = value<<4;    // update() expects data for the odd channels to be shifted...
    }
    else {
      pwmVals[channel] = value&0x0fff;
    }
  }

  // ------------------------------------------------------------------------
  // Call this in your main loop to perform one complete PWM cycle
  //
  // With the Tiny85's internal 8mHz clock this takes almost exactly
  // one millisecond to execute :-)
  // ------------------------------------------------------------------------
  void update() const {
    // Set BLANK and XLAT low for the PWM cycle
    PORTB &= ~_BV(BLANK_XLAT_PIN);

    // Set up USI (USI drives the TLC5940 clocks and serial data input)
    byte sOut = _BV(USI_CLK_PIN)|_BV(USI_OUT_PIN);
    DDRB |= sOut;         // Pins are outputs
    PORTB &= ~sOut;       // Outputs are low
    USICR = _BV(USIWM0);  // Three wire mode

// Macros to drive the USI clock
#define tick USICR=_BV(USIWM0)|_BV(USITC)
#define tock USICR=_BV(USIWM0)|_BV(USITC)|_BV(USICLK)
#define clock8()   tick; tock;  tick; tock;  tick; tock;  tick; tock;  tick; tock;  tick; tock;  tick; tock;  tick; tock;

    // We need to send 4096 clock pulses per PWM cycle, ie. 512 bytes of data.
    // There's only 24 bytes of real data per TLC5940 so we send some dummy data first.
    byte dummySize = (512-(24*NUM_TLC5940s))/2;
    while (dummySize != 0) {
      clock8();      // Send clock pulses for dummy data...
      clock8();
      --dummySize;
    }

    // Now send the real PWM data
    // nb. This sequence of instructions is the result of a lot of
    // compiles/disassembles to get the best possible code output
    // from the compiler. Don't change it...
    byte numPairs = 8*NUM_TLC5940s;
    const pwm_val *pwm = pwmVals+NUM_LEDS-2;
    while (numPairs != 0) {
      const pwm_val hi = pwm[1];
      USIDR = hi>>8;
      clock8();
      const pwm_val lo = pwm[0];
      USIDR = hi|(lo>>8);
      clock8();
      USIDR = lo;
      clock8();
      pwm -= 2;
      --numPairs;
    }

    // Set BLANK and XLAT high while you do your updates
    PINB = _BV(BLANK_XLAT_PIN);
  }
};


// A driver for you to use
Teeny5940 tlc5940;


#else
  unsupported_chip;
#endif

#endif /*_TEENY5940_H*/

