/*
  DMX Test for LeafLabs Maple 
  (tested on a RET6)
 
  by Brian Tovar
  created  26 Aug 2011
  modified  6 Sep 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

#include "dmx.h"

#define gnd_pin 38
#define vcc_pin 30

void setup() {
    //pinMode(BOARD_LED_PIN, OUTPUT);  // use the on board led
    //pinMode(gnd_pin, OUTPUT);        // temporary gnd for dmx breakout board
    //digitalWrite(gnd_pin, HIGH);     // ground pin
    pinMode(vcc_pin, OUTPUT);        // temporary vcc for dmx breakout board
    digitalWrite(vcc_pin, HIGH);     // vcc pin is 3.3v
    SerialUSB.begin();
    DMX.begin(3);
}

void loop() {
    waitForButtonPress();
    DMX.send();
    delay(1000);
    for(int i=0; i<DMX.number_of_channels; i++) {
        DMX.write(i, random(256));
    }
}

