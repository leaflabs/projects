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

#include "dmx.h"

DmxClass::DmxClass() {
  // initializes class variables and pin configurations
  // for Sparkfun RS-485 breakout board
  pinMode(BOARD_LED_PIN, OUTPUT);        // On board LED as ouptu
  pinMode(BOARD_BUTTON_PIN, INPUT);      // On board button as input
  pinMode(DMX_BRK_PIN, OUTPUT);          // configure Break as an output
  pinMode(DMX_RTS_PIN, OUTPUT);          // configure RTS pin as output
  digitalWrite(DMX_RTS_PIN, HIGH); // RTS high for drive mode
}

void DmxClass::begin(uint16 n) {
    //SerialUSB.println("DMX begin");
    if (n >= MAX_CHANNELS+1) { 
        return; }
    else {
        this->channel_count = n; }
    // NB: red, green, and blue are independent channels
    usart_set_baud_rate(DMX_USART_DEV, 72, 250000);
    usart_enable(DMX_USART_DEV);
    DMX_USART_DEV->regs->CR1 &= !USART_CR1_M;
    DMX_USART_DEV->regs->CR2 |= USART_CR2_STOP_BITS_2;
    //usart_set_baud_rate(DMX_USART_DEV, 72, 250000);
    //usart_enable(DMX_USART_DEV);
}

void DmxClass::end(void) {
  //SerialUSB.println("Packet Complete.");
}

void DmxClass::send(void) {
    if (DEBUG_LED) { toggleLED(); }
    // Set the TE bit in USART_CR1 to send an idle frame as first transmission. <opt>
    // Wake from idle: USART_CR1 Bit4 IDLEIE:IDLEinterruptenable
    digitalWrite(DMX_BRK_PIN, HIGH);
    delayMicroseconds(16);
    pinMode(DMX_BRK_PIN, INPUT);
    //usart_putc(DMX_USART_DEV, 0);
    usart_tx(DMX_USART_DEV, this-> channel, this->channel_count);
    pinMode(DMX_BRK_PIN, OUTPUT);
    digitalWrite(DMX_BRK_PIN, LOW);
    delay(20);
    if (DEBUG_LED) { toggleLED(); }
    usart_reset_rx(DMX_USART_DEV);
}

void DmxClass::write(uint16 chan, uint8 value) {
    if (chan >= MAX_CHANNELS) { 
        return; }
    else { 
        this->channel[chan] = value; }
}

//DmxClass DMX;
