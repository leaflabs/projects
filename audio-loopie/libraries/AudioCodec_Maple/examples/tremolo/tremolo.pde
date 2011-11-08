/*
tremolo.pde (MAPLE)
guest openmusiclabs 7.14.11
this program creates a mono tremolo effect by taking input
from the left channel and multiplying it by a low frequency
sinewave.  the frequency is set with MOD1, and the depth is
set with MOD0.  the amplitude modulated signal is presented
at the right output, and a mix of the wet and dry signals
is presented at the left output.
*/

// setup codec parameters
// must be done before #includes
// see readme file in libraries folder for explanations
#define SAMPLE_RATE 44 // 44.1kHz sample rate
#define ADCS 2 // use both ADCs

// include necessary libraries
// note the maple library
#include <AudioCodec_Maple.h>

// create data variables for audio transfer
// even though the function is mono, the codec requires stereo data
// note the int16, ints are 32b in maple
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

// create variables for ADC results
// it only has positive values -> unsigned
// note the uint16, there are no "unsigned int16" in maple
uint16 mod0_value = 0;
uint16 mod1_value = 0;

// create sinewave lookup table
int16 sinewave[] __FLASH__ = {
  // this file is stored in the AudioCodec_Maple library and is a
  // 1024 value sinewave lookup table of signed 16bit integers.
  // you can replace it with your own waveform if you like.
  #include <sinetable.inc>
};
 // lookup table value location
unsigned int location; // this is a 32bit number
                       // the lower 8bits are the subsample fraction
                       // and the upper 24 bits contain the sample number  


void setup() {
  SerialUSB.end(); // usb conflicts with the codec in this mode
                   // a different operating mode which allows both
                   // is in the works
  // setup codec and microcontroller registers
  AudioCodec_init(); // call this last if you are setting up other things
}

void loop() {
  while (1); // reduces clock jitter
}

// timer1 interrupt routine - all data processed here
void AudioCodec_interrupt() {

  // &'s are necessary on data_in variables
  AudioCodec_data(&left_in, &right_in, left_out, right_out);

  // create some temporary variables
  uint16 temp0;
  unsigned char frac;
  int16 temp1;
  int16 temp2;
  
  // create a variable frequency and amplitude sinewave.
  // look at lfo.pde for a detailed explanation of
  // how this works
  location += 1 + (mod1_value >> 8);
  location &= 0x0003ffff; 
  temp0 = (location >> 8);
  temp1 = sinewave[temp0];
  ++temp0;
  temp0 &= 0x03ff; 
  temp2 = sinewave[temp0];
  frac = (location & 0x000000ff);
  temp2 = (temp2 * frac) >> 8;
  temp1 = (temp1 * (0xff - frac)) >> 8;
  temp2 += temp1;
  temp2 = (temp2 * mod0_value) >> 16;
  // our sinewave is now temp2

  // create a tremolo effect by multiplying input signal by sinewave
  // turn signed sinewave value into unsigned value
  temp0 = temp2 + 0x8000;
  right_out = (left_in * temp0) >> 16;
  // put amplitude modulated data at right output
  // mix modulated and current data at left output
  // divide each by 2 for proper scaling
  left_out = (right_out >> 1) + (left_in >> 1);
  
  // & is required before adc variables
  AudioCodec_ADC(&mod0_value, &mod1_value);
  
  // we dont use reti() with Maple
}

