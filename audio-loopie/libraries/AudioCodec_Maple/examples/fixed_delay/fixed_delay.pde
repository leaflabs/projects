/*
fixed_delay.pde (MAPLE)
guest openmusiclabs 7.13.11
this function delays both the left and right channels by a
fixed amount.  the limit to the length of the delay is set
by the internal SRAM of the chip you are using.  for the
Maple r5 (STM32F103RBT6), this is 20kbytes.  each audio data
sample is 16bits, or 2bytes, so at most you can get a 10k
sample delay buffer.  but, since this is in stereo, that is
limited to 5k for each, left and right.  this is further
limited by the fact that the program itself needs to use
some of that space, so feel free to play around with the SIZE
constant.  the compiler will complain if its too big.  "SIZE
7000" gives a delay of 3500 samples per channel, which is a
3500/44.1kHz = 79ms delay time. 
*/


// setup codec parameters
// must be done before #includes
// see readme file in libraries folder for explanations
#define SAMPLE_RATE 44 // 44.1Khz
#define ADCS 0 // no ADCs are being used

// include necessary libraries
#include <AudioCodec_Maple.h>

// create data variables for audio transfer
// note the use of "int16" rather than int -> on the maple,
// an int is 32bits
int16 left_in = 0x0000;
int16 left_out = 0x0000;
int16 right_in = 0x0000;
int16 right_out = 0x0000;

// create a delay buffer in memory
#define SIZE 7000 // buffer size is limited by microcontroller SRAM size
int16 delaymem[SIZE]; // 800 positions x 2 bytes = 1600 bytes of SRAM
uint16 location = 0; // buffer location to read/write from
                     // note use of "uint16", "unsigned int16" doesnt work
                    
                     
void setup() {
  SerialUSB.end(); // usb conflicts with the codec in this mode
                   // a different operating mode which allows both
                   // is in the works
  AudioCodec_init(); // setup codec registers
  // call this last if setting up other parts
}

void loop() {
  while (1); // reduces clock jitter
}

// timer4->ch1 interrupt routine - all data processed here
// you must call this function "AudioCodec_interrupt"
void AudioCodec_interrupt() {

  // &'s are necessary on data_in variables
  AudioCodec_data(&left_in, &right_in, left_out, right_out);
  
  //fetch data from buffer and put it into output register
  left_out = delaymem[location];
  // put new data in same location for maximal delay time
  delaymem[location++] = left_in; // post increment location to go to next memory location
  // repeat for right channel
  right_out = delaymem[location];
  delaymem[location++] = right_in; // post increment in preperation for next transfer
  // check if location has gotten bigger than buffer size
  if (location >= SIZE) {
    location = 0; // reset location
  }
  
  // dont use reti() with on maple
}

