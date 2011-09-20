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
  pinMode(BOARD_LED_PIN, OUTPUT);          // On board LED as ouptu
  pinMode(BOARD_BUTTON_PIN, INPUT);        // On board button as input
  pinMode(DMX_BRK_PIN, OUTPUT);            // configure Break as an output
  pinMode(DMX_RTS_PIN, OUTPUT);            // configure RTS pin as output
  digitalWrite(DMX_RTS_PIN, HIGH);         // RTS high for drive mode
}

void DmxClass::begin(uint16 n) {
    //SerialUSB.end();
    SerialUSB.begin();
    if (n > MAX_CHANNELS) {
        return; }
    else {
        this->channel_count = n; }
    // NB: red, green, and blue are independent channels
    Serial2.begin(250000);
//    gpio_set_mode(GPIOA, 2, GPIO_AF_OUTPUT_PP);
//    gpio_set_mode(GPIOA, 3, GPIO_INPUT_FLOATING);
//    timer_set_mode(txi->timer_device, txi->timer_channel, TIMER_DISABLED);
//    usart_init(DMX_USART_DEV);
//    usart_set_baud_rate(DMX_USART_DEV, PCLK2, 250000);
//    usart_enable(DMX_USART_DEV);
    DMX_USART_DEV->regs->CR2 |= USART_CR2_STOP_BITS_2;
    this->channel[0]=0; //start-code hack
    this->chan = &this->channel[0];
}

void DmxClass::end(void) {
    SerialUSB.begin();
    //usart_disable(DMX_USART_DEV);
}

void DmxClass::send(void) { 
    if (DEBUG_LED) { digitalWrite(BOARD_LED_PIN, HIGH); }
    gpio_write_bit(GPIOC, 15, HIGH);
    delayMicroseconds(16);//96);
    gpio_set_mode(GPIOC, 15, GPIO_INPUT_FLOATING);
    Serial2.print(this->channel);
//    usart_reg_map *regs = DMX_USART_DEV->regs;
//    uint16 txed = 0;
//    while ((regs->SR & USART_SR_TXE) && (txed <= this->channel_count+1)) {
//        regs->DR = this->channel[txed++];
//        //delayMicroseconds(4);
//    }
    //pinMode(DMX_BRK_PIN, OUTPUT);
    gpio_set_mode(GPIOC, 15, GPIO_OUTPUT_PP);
    gpio_write_bit(GPIOC, 15, LOW);
    delayMicroseconds(156);
    //usart_reset_rx(DMX_USART_DEV);
    if (DEBUG_LED) { toggleLED(); }  
}

void DmxClass::write(uint16 c, uint8 v) {
    if (c >= MAX_CHANNELS) { 
        return; }
    else if (c == 0) {
        return; }
    else { 
        this->channel[c] = v; }
}
