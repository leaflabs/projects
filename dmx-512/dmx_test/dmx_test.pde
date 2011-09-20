/*
 * DMX Test Program
 *
 * by Brian Tovar
 * created  26 Aug 2011
 * modified 14 Sep 2011
 *
 * NB: Be sure to use a 120 Ohm terminating resistor to prevent reflections
 */

#include "dmx.h"
//#include "~/Documents/MapleIDE/libraries/fftw/api/fftw3.h"

DmxClass DMX;

void setup() {
    SerialUSB.begin();
    DMX.begin(3);
}

void loop() {
    for (int i=1; i<=DMX.channel_count; i++) {
        DMX.write(i, random(256));
    }
    DMX.send();
    delay(1000);
}
