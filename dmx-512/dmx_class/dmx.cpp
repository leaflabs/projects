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

DmxClass DMX;

void dmx_handler_hack(void) {
  DMX.handler();
}

DmxClass::DmxClass() {
  // application specific constants (user-configurable)
  this->dmx_rts_pin = 18; //36;
  this->dmx_tx1_pin = 19; //34;
  this->dmx_rx1_pin = 20; //32;
  this->dmx_timer = TIMER2;
  this->dmx_timer_ch = TIMER_CH2;
  // NB: timer device must have type  TIMER_ADVANCED or TIMER_GENERAL
  
  // initializes class variables and pin configurations
  // for Sparkfun RS-485 breakout board
  this->bitBuffer = 0;                   // initialize bit buffer
  pinMode(this->dmx_rx1_pin, INPUT);     // configure RX pin as input
  pinMode(this->dmx_tx1_pin, OUTPUT);    // configure TX pin as output
  pinMode(this->dmx_rts_pin, OUTPUT);    // configure RTS pin as output
  digitalWrite(this->dmx_rts_pin, HIGH); // RTS high for drive mode
}

void DmxClass::begin(uint16 n) {
  SerialUSB.println("DMX begin");
  this->number_of_channels = n; // red, green, and blue are independent channels
  //SerialUSB.end();
  
  // initializes timer configurations
  timer_pause(this->dmx_timer);
  timer_set_prescaler(this->dmx_timer, 1); 
  timer_set_reload(this->dmx_timer, 288); // 4 us = 288 clock pulses @ 72MHz
  timer_generate_update(this->dmx_timer); // update new reload value
  timer_set_mode(this->dmx_timer, dmx_timer_ch, TIMER_OUTPUT_COMPARE);
  timer_set_compare(this->dmx_timer, dmx_timer_ch, 1); // test
  timer_attach_interrupt(this->dmx_timer, TIMER_CC1_INTERRUPT, dmx_handler_hack);
  timer_resume(this->dmx_timer);
}

void DmxClass::end(void) {
  //SerialUSB.begin();
  //SerialUSB.println();
  SerialUSB.println("DMX closing");
  SerialUSB.println(this->number_of_channels, DEC);
}

void DmxClass::send(void) {
    SerialUSB.println("DMX send");
    if (DEBUG_LED) { toggleLED(); }
    this->headerIndex = 0;
    this->channelIndex = 0;
    this->bitIndex = 0;
    timer_resume(this->dmx_timer);
}

void DmxClass::handler(void) {
    digitalWrite(this->dmx_tx1_pin, this->bitBuffer);
    if (this->channelIndex == this->number_of_channels) {
        timer_pause(this->dmx_timer);
        this->bitBuffer = END_OF_PACKET_BIT;
        if (DEBUG_LED) { toggleLED(); }
        SerialUSB.println("end of packet");
    }
    else if (this->headerIndex < SIZE_OF_HEADER) {
        SerialUSB.print(headerIndex, DEC);
        this->bitBuffer = this->header();
        headerIndex++;
        if (headerIndex >= SIZE_OF_HEADER) { SerialUSB.println(); }
    }
    else {
        switch ( this->bitIndex ) {
            case 0:
              this->bitBuffer = START_BIT;
              this->bitIndex++;
              SerialUSB.print("chan: ");
              SerialUSB.println(channelIndex, DEC);
              break;
            case 10:
              this->bitBuffer = STOP_BIT;
              this->bitIndex = 0;
              this->channelIndex++;
              SerialUSB.println();
              break;
            case 9: 
              this->bitBuffer = STOP_BIT;
              this->bitIndex++;
              break;
            default: // channel data: cases 1 through 8
              this->bitBuffer = bitRead(this->channel[this->channelIndex], this->bitIndex - 1);
              SerialUSB.print(bitBuffer, DEC);
              this->bitIndex++;
              break;
        }
    }
}

void DmxClass::write(int chan, uint8 value) {
  this->channel[chan] = value;
}

uint8 DmxClass::header(void) {
    switch ( this->headerIndex ) {
        case 0:
            return END_OF_PACKET_BIT; // BREAK (should be at least 22-bits long)
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
        default:
            return END_OF_PACKET_BIT;
    }
}

