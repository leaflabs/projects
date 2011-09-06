/*
  DMX Class for LeafLabs Maple 
  (tested on a RET6)
 
  by Brian Tovar
  created  26 Aug 2011
  modified  6 Sep 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

#include "dmx.h"

void setup() {
  //pinMode(BOARD_LED_PIN, OUTPUT);  // use the on board led
  SerialUSB.end();                 // just in case, prob not needed
  dmx(3);
}

void loop() {
  dmx.write();
  delay(1000);
  for(int i=0; i<dmx.NUM_OF_CHANNELS; i++) {
    dmx.write(i, random(256) );
  }
}

