/*
  DMX Test Program
 
  by Brian Tovar, Rurik Primiani, Blake Rego
  for LeafLabs Maple (tested on a RET6)
  created  26 Jul 2011
  modified 26 Aug 2011
  
  NB: Be sure to use a 120-Ohm terminating resistor to prevent reflections
*/

extern void write_dmx_packet(void) {
  toggleLED();
  load_dmx_buffer();
  dmxIndex = 0;
  timer_dmx_packet.refresh();
  timer_dmx_packet.resume();
}

static void handler_dmx_packet(void) {
  digitalWrite(DMX_TX1_PIN, dmxBuffer[dmxIndex]);
  if (dmxIndex >= LENGTH_OF_PACKET) {
      timer_dmx_packet.pause();
      toggleLED();
  }
  dmxIndex++;
}

static void load_dmx_buffer(void) {
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

static void timer_dmx_setup(void) {    
  timer_dmx_packet.pause();
  timer_dmx_packet.setPrescaleFactor(1);
  timer_dmx_packet.setOverflow(288); // 4 us = 288 clk pulses @ 72MHz
  timer_dmx_packet.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  timer_dmx_packet.setCompare(DMX_TIMER_CHAN, 1);
  timer_dmx_packet.attachCompare1Interrupt(handler_dmx_packet);
}