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
  pinMode(DMX_RX1_PIN, INPUT);
  pinMode(DMX_TX1_PIN, OUTPUT);
  pinMode(DMX_RTS_PIN, OUTPUT);
  pinMode(30, OUTPUT);             // temporary vcc for dmx breakout
  digitalWrite(30, HIGH);          // vcc
  digitalWrite(DMX_RTS_PIN, HIGH); // RTS high for drive mode
  pinMode(BOARD_LED_PIN, OUTPUT);  // use the on board led
  SerialUSB.end();                 // just in case, prob not needed
  timer_dmx_setup();
}

void loop() {
  write_dmx_packet();
  delay(1000);
  for(int i=0; i<NUM_OF_CHANNELS; i++) {
    channel[i] = random(256); 
  }
}
