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

#define PI 3.14159265 

DmxClass DMX;
float t = 1.0;
float s;
uint8 y;

void setup() {
    DMX.begin(512);
}

void loop() {
    s = sin(PI*t/100.0);
    y = uint8(255*pow(s, 2));
    for (int i=1; i<=DMX.count; i++) {
        DMX.write(i, y );
    }
    DMX.send();
    
    if (t >= 100.0) { t = 0; }
    else { t+=1.0; }
}
