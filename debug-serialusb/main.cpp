/*
 * Stress test code for the SerialUSB module of wirish
 *
 * Intended to work with serialUSB_test.py
 *
 *
 * tests should include:
 *    - usbSendBytes    - not dropping packets at any speed/size
 *    - usbReceiveBytes - not dropping packets at any speed/size
 * 
 *    - SerialUSB.print  - blocking write, always works
 *    - SerialUSB.read() - blocking read, returns one byte in sequence and drops none 
 *
 * This code is released into the public domain.
 */

#include "wirish.h"
#include "usb.h"


//------------------------------ program macros -------------------------------
#define LED_PIN (13)

//------------------------------ setup()/loop() -------------------------------


void setup() {
    pinMode(LED_PIN,OUTPUT);
}

void loop() {
    static int  toggle = 0;

    toggle ^= 1;
    digitalWrite(LED_PIN,toggle);
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
