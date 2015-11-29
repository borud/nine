/**
 * Wiring the TLC5940 to the Attiny85 use the following wiring.
 *
 *     Chip pin | Wiring pin | TLC5940
 *     ---------+------------+----------------------
 *         5    |     0      | BLANK(23) + XLAT(24)
 *         6    |     1      | SIN(25)
 *         7    |     2      | SCLK(25) + GSCLK(18)
 *     ---------------------------------------------
 *
 *
 *
 * @author borud
 */

#include <avr/sleep.h>
#include <avr/power.h>
#include "Tiny85_TLC5940.h"

#define CHANCE_FACTOR      250
#define MAX_INTENSITY     4095
#define DECAY_LOWER_BOUND   10
#define DECAY_UPPER_BOUND   80


#define NUM_LEDS       9
#define FIRST_LED      1

#define LED(x)         (FIRST_LED + x)

struct led_state_t {
    int intensity;
    int decay_rate;
};
    
static led_state_t led_states[NUM_LEDS];


void update_led_state() {
    for (int i = 0; i < NUM_LEDS; i++) {
        // If a LED is lit we decay it and skip the rest of the loop.
        if (led_states[i].intensity > 0) {
            led_states[i].intensity -= led_states[i].decay_rate;
            if (led_states[i].intensity < 0) {
                led_states[i].intensity = 0;
            }
            continue;
        }

        if (random(CHANCE_FACTOR) != 1) {
            continue;
        }
        
        // Set the LED parameters.
        led_states[i].intensity = MAX_INTENSITY;
        led_states[i].decay_rate = random(DECAY_LOWER_BOUND, DECAY_UPPER_BOUND);

    }

    for (int i = 0; i < NUM_LEDS; i++) {
        tlc5940.set(LED(i), led_states[i].intensity);
    }
    tlc5940.update();
}

void setup() {
    // Set up the output pins
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);
    pinMode(2, OUTPUT);

    tlc5940.init();
    tlc5940.clear();
}

void loop() {
    update_led_state();
    delay(1);
}
