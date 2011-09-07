/*
  DMX Test for LeafLabs Maple 
  (tested on a RET6)
 
  by Brian Tovar
  created  26 Aug 2011
  modified  6 Sep 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

#include "dmx.h"

DmxClass test_dmx;

void setup() {
  //pinMode(BOARD_LED_PIN, OUTPUT);  // use the on board led
  test_dmx.begin(3);
}

void loop() {
  test_dmx.send();
  delay(500);
  for(int i=0; i<test_dmx.number_of_channels; i++) {
    test_dmx.write(i, random(256) );
  }
}

