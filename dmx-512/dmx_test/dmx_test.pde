/*
  DMX Test Program
 
  by Brian Tovar, Rurik Primiani, Blake Rego
  for LeafLabs Maple (tested on Maple RET6)
  created  26 Jul 2011
  modified 26 Aug 2011
  
  NB: Be sure to use a 120 Ohm terminating resistor to prevent reflections
*/

#include "dmx.h"

void setup() {
    SerialUSB.begin();
    DMX.begin(3);
}

void loop() {
  for(int i=0; i<DMX.channel_count; i++) {
      DMX.write(i, random(256));
  }
  DMX.send();
  if (!DEBUG_LED) { delay(200); }
  delay(800);
}
