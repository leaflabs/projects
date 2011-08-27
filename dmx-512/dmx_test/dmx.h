/*
  DMX Test Program
 
  by Brian Tovar, Rurik Primiani, Blake Rego
  for LeafLabs Maple (tested on a RET6)
  created  26 Jul 2011
  modified 26 Aug 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

#ifndef _DMX_512_
#define _DMX_512_

#include <timer.h>
#include "dmx.c"

#define DMX_TIMER_CHAN TIMER_CH1
#define DMX_RTS_PIN 36
#define DMX_TX1_PIN 34
#define DMX_RX1_PIN 32
//#define DMX_TX2_PIN 33
//#define DMX_RX2_PIN 31
static const int SIZE_OF_HEADER 16
extern const int NUM_OF_CHANNELS 3
static const int LENGTH_OF_PACKET 11*NUM_OF_CHANNELS+SIZE_OF_HEADER+1

extern HardwareTimer timer_dmx_packet(DMX_TIMER_CHAN);

static uint16 dmxIndex;

static struct Light {
  int address;
  uint8* const red;
  uint8* const grn;
  uint8* const blu;
};
extern typedef struct Light light;

extern uint8 channel[NUM_OF_CHANNELS] = {
  128, 128, 128,
};

static uint8 dmxBuffer[LENGTH_OF_PACKET] = {
  0, // BREAK (should be longer, going to assume space btwn packets)
  1, 1, 1, 1, // MARK AFTER BREAK
  0, // START BIT
  0, 0, 0, 0, 0, 0, 0, 0, // START CODE
  1, 1, // STOP BITs
};

#endif