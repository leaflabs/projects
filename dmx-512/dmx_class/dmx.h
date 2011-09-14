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
 * DMX Class for LeafLabs Maple 
 *
 * by Brian Tovar
 * created  26 Aug 2011
 * modified  6 Sep 2011
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

//static const uint8 HEADER[SIZE_OF_HEADER] = {
//    0, // BREAK (should be longer, going to assume space btwn packets)
//    1, 1, 1, 1, // MARK AFTER BREAK
//    0, // START BIT
//    0, 0, 0, 0, 0, 0, 0, 0, // START CODE
//    1, 1, // STOP BITs
//};

void dmx_handler_hack(void);

// class definition for dmx lights
class DmxClass {
  public:
    DmxClass();
    void begin(uint16);
    void end(void); 
    void send(void);
    void write(int, uint8);
    uint16 number_of_channels;
    void handler(void);
    
  private:
    uint8 dmx_rts_pin;
    uint8 dmx_tx1_pin;
    uint8 dmx_rx1_pin;
    uint8 dmx_tx2_pin;
    uint8 dmx_rx2_pin;
    timer_dev* dmx_timer;
    timer_channel dmx_timer_ch;

    int headerIndex, channelIndex, bitIndex;
    uint8 bitBuffer;
    uint8 channel[MAX_CHANNELS];
    uint8 header(void);
};

// Declare the instance that the users of the library can use
extern DmxClass DMX;

#endif
