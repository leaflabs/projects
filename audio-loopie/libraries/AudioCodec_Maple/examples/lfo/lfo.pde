/*
lfo.pde (MAPLE)
guest openmusiclabs 7.14.11
this program creates a low distortion sinewave of variable
frequency and amplitude, presented at both left and right
outputs. it uses interpolation to get 256 different
frequencies between .17Hz and 43Hz.
*/

// setup codec parameters
// must be done before #includes
// see readme file in libraries folder for explanations
#define SAMPLE_RATE 44 // 44.1kHz sample rate
#define ADCS 2 // use both ADCs

// include necessary libraries
// note the Maple library
#include <AudioCodec_Maple.h>

// create data variables for audio transfer
// even though there is no input needed, the codec requires stereo data
// note the int16, ints are 32bit in Maple
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

// create variables for ADC results
// it only has positive values -> unsigned
// note the uint16, there is no "unsigned int16"
uint16 mod0_value = 0;
uint16 mod1_value = 0;

// create sinewave lookup table
int16 sinewave[] __FLASH__ = {
  // this file is stored in AudioCodec_Maple.h and is a 1024 value
  // sinewave lookup table of signed 16bit integers
  // you can replace it with your own waveform if you like
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
  // since we will be moving through the lookup table at
  // a variable frequency, we wont always land directly
  // on a single sample.  so we will average between the
  // two samples closest to us.  this is called interpolation.
  // step through the table at rate determined by mod1
  // use upper byte of mod1 value to set the rate
  // and have an offset of 1 so there is always an increment.
  location += 1 + (mod1_value >> 8);
  // if weve gone over the table boundary -> loop back
  location &= 0x0003ffff; // this is a faster way doing the table
                          // wrap around, which is possible
                          // because our table is a multiple of 2^n.
                          // otherwise you would do something like:
                          // if (location >= 1024*256) {
                          //   location -= 1024*256;
                          // }
  temp0 = (location >> 8);
  // get first sample and store it in temp1
  temp1 = sinewave[temp0];
  ++temp0; // go to next sample
  temp0 &= 0x03ff; // check if weve gone over the boundary.
                   // we can do this because its a multiple of 2^n,
                   // otherwise it would be:
                   // if (temp0 >= 1024) {
                   //   temp0 = 0; // reset to 0
                   // }
  // get second sample and put it in temp2
  temp2 = sinewave[temp0];
  
  // interpolate between samples
  // multiply each sample by the fractional distance
  // to the actual location value
  frac = (location & 0x000000ff); // fetch the lower 8b
  temp2 = (temp2 * frac) >> 8;
  // scaled sample 2 is now in temp2
  temp1 = (temp1 * (0xff - frac)) >> 8;
  // temp1 now has the scaled sample 1
  temp2 += temp1; // add samples together to get an average
  // our sinewave is now in temp2
  
  // set amplitude with mod0
  // multiply our sinewave by the mod0 value
  temp2 = (temp2 * mod0_value) >> 16;
  left_out = temp2; // put sinusoid out on left channel
  right_out = -temp2; // put inverted version out on right chanel

  // get ADC values
  // & is required before adc variables
  AudioCodec_ADC(&mod0_value, &mod1_value);

  // you dont need reti() with Maple
}

