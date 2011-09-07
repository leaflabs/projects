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
 
  by Brian Tovar
  created  26 Aug 2011
  modified  6 Sep 2011
*/

#include "dmx.h"

#define STOP_BIT HIGH
#define START_BIT LOW
#define END_OF_PACKET_BIT LOW
#define DEBUG_LED 1

// Hack for C++ type system distinction between pointer-to-function
// and pointer-to-member-function
static DmxClass *activeInstance = NULL;
void dmx_handler_wrapper(void);

DmxClass::DmxClass() {
  // application specific constants (user-configurable)
  dmx_rts_pin = 36;
  dmx_tx1_pin = 34;
  dmx_rx1_pin = 32;
  dmx_timer = TIMER2;
  dmx_timer_ch = TIMER_CH2;
  // NB: timer device must have type  TIMER_ADVANCED or TIMER_GENERAL
  
  // initializes class variables and pin configurations
  bitBuffer = 0;                   // initialize bit buffer
  pinMode(dmx_rx1_pin, INPUT);     // configure RX pin as input
  pinMode(dmx_tx1_pin, OUTPUT);    // configure TX pin as output
  pinMode(dmx_rts_pin, OUTPUT);    // configure RTS pin as output
  digitalWrite(dmx_rts_pin, HIGH); // RTS high for drive mode
  pinMode(30, OUTPUT);             // temporary vcc for dmx breakout board
  digitalWrite(30, HIGH);          // vcc pin is 3.3v
}

void DmxClass::begin(uint16 n) {
  number_of_channels = n; // red, green, and blue are independent channels
  SerialUSB.end();
  
  // Turn off any other Dmx instances, just in case
  activeInstance = NULL;
  
  // initializes timer configurations
  timer_pause(this->dmx_timer);
  timer_set_prescaler(this->dmx_timer, 1);
  timer_set_reload(this->dmx_timer, 288); // 4 us = 288 clock pulses @ 72MHz
  timer_generate_update(this->dmx_timer); // update new reload value
  timer_set_mode(this->dmx_timer, dmx_timer_ch, TIMER_OUTPUT_COMPARE);
  timer_set_compare(this->dmx_timer, dmx_timer_ch, 1); // test
  timer_attach_interrupt(this->dmx_timer, TIMER_CC1_INTERRUPT, dmx_handler_wrapper);
  timer_resume(this->dmx_timer);

  // Make this the active Dmx instance
  activeInstance = this;
}

void DmxClass::end(void) {
  activeInstance = NULL;
  SerialUSB.begin();
  SerialUSB.println();
}

void DmxClass::send(void) {
    if (DEBUG_LED) { toggleLED(); }
    this->headerIndex = 0;
    this->channelIndex = 0;
    this->bitIndex = 0;
    timer_resume(this->dmx_timer);
}

void dmx_handler_wrapper(void) {
    // Do nothing if we don't have an active Dmx instance
    if (!activeInstance) {
        return;
    }
    
    activeInstance->handler();  
}

void DmxClass::handler(void) {
    digitalWrite(dmx_tx1_pin, this->bitBuffer);
    if (this->channelIndex == number_of_channels) {
        timer_pause(this->dmx_timer);
        bitBuffer = END_OF_PACKET_BIT;
        if (DEBUG_LED) { toggleLED(); }
    }
    else if (this->headerIndex < SIZE_OF_HEADER) {
        //this->bitBuffer = this->header();
        headerIndex++;
    }
    else {
        switch ( this->bitIndex ) {
            case 0:
              bitBuffer = START_BIT;
              bitIndex++;
              break;
            case 10:
              bitBuffer = STOP_BIT;
              bitIndex = 0;
              channelIndex++;
              break;
            case 9: 
              bitBuffer = STOP_BIT;
              bitIndex++;
              break;
            default: // channel data: cases 1 through 8
              bitBuffer = bitRead(this->channel[this->channelIndex], this->bitIndex - 1);
              this->bitIndex++;
              break;
        }
    }
}

void DmxClass::write(int chan, uint8 value) {
  this->channel[chan] = value;
}

//uint8 DmxClass::header(void) {
//    switch ( this->headerIndex ) {
//        case 0:
//          return END_OF_PACKET_BIT; // BREAK (should be at least 22-bits long)
//          break;
//        case 2:
//        case 3:
//        case 4:
//          return 1; // MARK AFTER BREAK
//        case 5:
//          return START_BIT; // START-BIT
//        case 6:
//        case 7:
//        case 8:
//        case 9:
//        case 10:
//        case 11:
//        case 12:
//        case 13: 
//          return 0; // START CODE
//        case 14:
//        case 15:
//          return STOP_BIT; // STOP-BIT
//    }
//}


// Declare the instance that the users of the library can use
extern DmxClass Dmx;
