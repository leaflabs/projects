 /******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2011, LeafLabs, LLC.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/*
  DMX Class for LeafLabs Maple 
  (tested on a RET6)
 
  by Brian Tovar
  created  26 Aug 2011
  modified  6 Sep 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

#include "dmx.h"

#define STOP_BIT HIGH
#define START_BIT LOW
#define END_OF_PACKET_BIT LOW
#define DEBUG_LED 1

 Dmx::Dmx(int n) {
  // application specific constants (user-configurable)
  DMX_RTS_PIN = 36;
  DMX_TX1_PIN = 34;
  DMX_RX1_PIN = 32;
  // NB: timer device must have type  TIMER_ADVANCED or TIMER_GENERAL
  DMX_TIMER = TIMER2;
  DMX_TIMER_CH = TIMER_CH2;
  
  // initializes class members and timer settings
  NUM_OF_CHANNELS = n;             // number of channels = 3x number of lights
  bitBuffer = 0;                   // initialize bit buffer
  pinMode(DMX_RX1_PIN, INPUT);     // configure RX pin as input
  pinMode(DMX_TX1_PIN, OUTPUT);    // configure TX pin as output
  pinMode(DMX_RTS_PIN, OUTPUT);    // configure RTS pin as output
  pinMode(30, OUTPUT);             // temporary vcc for dmx breakout board
  digitalWrite(30, HIGH);          // vcc pin is 3.3v
  digitalWrite(DMX_RTS_PIN, HIGH); // RTS high for drive mode

  //initTimer(DMX_TIMER);
  timer_pause(DMX_TIMER);
  timer_set_prescaler(DMX_TIMER, 1);
  timer_set_reload(DMX_TIMER, 288); // 4 us = 288 clock pulses @ 72MHz
  timer_generate_update(DMX_TIMER); // update new reload value
  timer_set_mode(DMX_TIMER, DMX_TIMER_CH, TIMER_OUTPUT_COMPARE);
  timer_set_compare(DMX_TIMER, DMX_TIMER_CH, 1); // test
  timer_attach_interrupt(DMX_TIMER, TIMER_CC1_INTERRUPT, handler);
  timer_resume(DMX_TIMER);
}

void Dmx::send(void) {
  if (DEBUG_LED) { toggleLED(); }
  this->h = 0;
  this->c = 0;
  this->b = 0;
  timer_resume(DMX_TIMER);
}

void Dmx::handler(void) {
  digitalWrite(DMX_TX1_PIN, bitBuffer);
  if (c == NUM_OF_CHANNELS) {
      timer_pause(DMX_TIMER);
      bitBuffer = END_OF_PACKET_BIT;
      if (DEBUG_LED) { toggleLED(); }
  }
  else if (h < SIZE_OF_HEADER) {
    bitBuffer = header();
    h++;
  }
  else {
    switch ( b ) {
        case 0:
          bitBuffer = START_BIT;
          b++;
          break;
        case 10:
          bitBuffer = STOP_BIT;
          b=0;
          c++;
          break;
        case 9: 
          bitBuffer = STOP_BIT;
          b++;
          break;
        default: // channel data: cases 1 through 8
          bitBuffer = bitRead(channel[c],b-1);
          b++;
          break;
    } //end-of-switch
  } // end-of-else
}

void Dmx::write(int chan, uint8 value) {
  channel[chan] = value;
}

uint8 Dmx::header(void) {
  switch ( h ) {
    case 0:
      return 0; // BREAK (should be at least 22-bits long)                                   
    case 1:
    case 2:
    case 3:
    case 4:
      return 1; // MARK AFTER BREAK                                                                      
    case 5:
      return START_BIT; // START-BIT                                                                            
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13: 
      return 0; // START CODE
    case 14:
    case 15:    
      return STOP_BIT; // STOP-BIT
    };
}

// Declare the instance that the users of the library can use
Dmx dmx;
