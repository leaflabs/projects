/*
 * Generate the THX Deep Note using the wm8731 codec chip
 */

#include "wirish.h"
#include "usb.h"

#define BOARD_LED_PIN 13
#define BOARD_BUT_PIN 38


void setup() {
    pinMode(BOARD_BUT_PIN,INPUT);
    pinMode(BOARD_LED_PIN,OUTPUT);
    while (!digitalRead(BOARD_BUT_PIN)); // wait for press

}

void loop() {
    digitalWrite(BOARD_LED_PIN,HIGH);
    delay(1000);
    digitalWrite(BOARD_LED_PIN,LOW);
    delay(1000);
}

// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated object that need libmaple may fail.
 __attribute__(( constructor )) void premain() {
    init();
}

int main(void) {
    setup();

    while (1) {
        loop();
    }

    return 0;
}
