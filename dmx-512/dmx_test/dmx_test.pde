/*
  DMX Test Program
 
  by Brian Tovar, Rurik Primiani, Blake Rego
  for LeafLabs Maple (tested on Maple RET6)
  created  26 Jul 2011
  modified 26 Aug 2011
  
  NB: Be sure to use a 120 Ohm terminating resistor to prevent reflections
*/

//#include "dmx.h"
//#include <math.h>
#include <timer.h>

#define DMX_TIMER_CHAN TIMER_CH1
#define DMX_RTS_PIN 36
#define DMX_TX1_PIN 34
#define DMX_RX1_PIN 32
//#define DMX_TX2_PIN 33
//#define DMX_RX2_PIN 31
#define SIZE_OF_HEADER 16
#define NUM_OF_CHANNELS 3 // 1 color-kinetic light (R, G, B)
#define LENGTH_OF_PACKET 11*NUM_OF_CHANNELS+SIZE_OF_HEADER+1

HardwareTimer timer_dmx_packet(DMX_TIMER_CHAN);
//HardwareTimer timer_fft(TIMER_CH2);
//HardwareTimer timer_audio(TIMER_CH3);

uint16 dmxIndex;

uint8 channel[NUM_OF_CHANNELS] = {
  128, 128, 128,
};

uint8 dmxBuffer[LENGTH_OF_PACKET] = {
  0, // BREAK (should be longer, going to assume space btwn packets)
  1, 1, 1, 1, // MARK AFTER BREAK
  0, // START BIT
  0, 0, 0, 0, 0, 0, 0, 0, // START CODE
  1, 1, // STOP BITs
};

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

void write_dmx_packet(void) {
  // FFT estimate: 1.28 ms or 46080/(36MHz)
  toggleLED();
  load_dmx_buffer();
  dmxIndex = 0;
  timer_dmx_packet.refresh();
  timer_dmx_packet.resume();
}

void handler_dmx_packet(void) {
  digitalWrite(DMX_TX1_PIN, dmxBuffer[dmxIndex]);
  if (dmxIndex >= LENGTH_OF_PACKET) {
      timer_dmx_packet.pause();
      toggleLED();
  }
  dmxIndex++;
}

void load_dmx_buffer(void) {
  int i=0;
  while (i<NUM_OF_CHANNELS) {
    for (int j=0; j<=10; j++) {
      int k = (SIZE_OF_HEADER)+(j)+(11*i);
      switch ( j ) {
       	case 0:
          dmxBuffer[k] = LOW; // start-bit
          break;
        case 9: 
          dmxBuffer[k] = HIGH; // first stop-bit
          break;
        case 10:
          dmxBuffer[k] = HIGH; // second stop-bit
          i++;
          break;
        default: // channel data: cases 1 through 8
          dmxBuffer[k] = uint8(bitRead(channel[i],j-1));
          break;
      	} // end-of-switch (each bit)
    } // end-of-for (each channel)
  } // end-of-while (each packet)
  dmxBuffer[LENGTH_OF_PACKET] = LOW;
}

void timer_dmx_setup(void) {    
  timer_dmx_packet.pause();
  timer_dmx_packet.setPrescaleFactor(1);
  timer_dmx_packet.setOverflow(288); // 4 us = 288 clk pulses @ 72MHz
  timer_dmx_packet.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  timer_dmx_packet.setCompare(DMX_TIMER_CHAN, 1);
  timer_dmx_packet.attachCompare1Interrupt(handler_dmx_packet);
}
