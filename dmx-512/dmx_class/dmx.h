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

#ifndef _DMX_H_
#define _DMX_H_

#ifdef MAPLE_IDE
#include "wirish.h"             /* hack for IDE compile */
#endif 

#include "timer.h"
#include "libmaple_types.h"
#include "wirish_types.h"

#define MAX_CHANNELS 512 // to save program memory, reduce this number
#define SIZE_OF_HEADER 16

// class definition for dmx lights
class Dmx {

  public:
    Dmx();
    void begin(uint16);
    void end(void); 
    void send(void);
    void write(int, uint8);
    uint16 NUM_OF_CHANNELS;
    // hack: global timer interrut handler.  See dmx.cpp if you're
    // interested, but you should probably leave this alone.
    friend void dmx_handler_wrapper(void);
  private:
    // make enum: DMX_TIMER_CHAN;
    uint8 DMX_RTS_PIN;
    uint8 DMX_TX1_PIN;
    uint8 DMX_RX1_PIN;
    uint8 DMX_TX2_PIN;
    uint8 DMX_RX2_PIN;
    timer_dev* DMX_TIMER;
    timer_channel DMX_TIMER_CH;

    int h, c, b;
    uint8 volatile bitBuffer;
    uint8 channel[MAX_CHANNELS];

    void handler(void);
    uint8 header(void);
};

#endif
