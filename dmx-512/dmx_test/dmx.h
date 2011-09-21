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
 * modified 14 Sep 2011
 */

#ifndef _DMX_H_
#define _DMX_H_

#ifdef MAPLE_IDE
#include "wirish.h"             /* hack for IDE compile */
#endif 

#include "gpio.h"
#include "usart.h"
#include "boards.h"
//#include "timer.h"

#define DEBUG_LED 0
#define DMX_BRK_PIN 12
#define DMX_RTS_PIN 13
#define DMX_USART_DEV USART2
#define MAX_CHANNELS 512 // to save memory, reduce this number

// class definition for dmx lights
class DmxClass {
  public:
    DmxClass();
    void begin(uint16);
    void end(void); 
    void send(void);
    void write(uint16, uint8);
    uint16 count;
    //uint8 *data;
    
  private:
    uint8 channel[MAX_CHANNELS];
};

// Declare the instance that the users of the library can use
extern DmxClass DMX;

#endif
