/*
vco.pde (MAPLE)
guest 7.11.11
this program creates a sinewave of variable frequency and
amplitude, presented at both left and right outputs. the frequency
is set by the left channel input, at a scale of (1/3)V/octave,
with 0v being C0.  it implements this with a logarithmic lookup
table.  the sinewave and lookup table are both interpolated to
give relatively good pitch accuracy from 16Hz to 10kHz.  there
is a frequency rollover at the top value of 3.3V input.
*/

// setup codec parameters
// must be done before #includes
// see readme file in libraries folder for explanations
#define SAMPLE_RATE 44 // 44.1kHz sample rate
#define ADCS 0 // dont use ADCs
#define ADCHPD 1 // high pass filter disabled -> DC coupled

// include necessary libraries
// note Maple library
#include <AudioCodec_Maple.h>

// create data variables for audio transfer
// even though there is no input needed, the codec requires stereo data
// note int16, maple ints are 32b
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

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

// create logarithmic frequency lookup table
uint16 logtable[] __FLASH__ = {
  // this file is stored in AudioCodec.h and is a 256 value
  // sinewave lookup table of unsigned 16bit integers
  // you can replace it with your own table if you like
  #include <logtable.inc>
};


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
  // these tend to work faster than using the main data variables
  // as they arent fetched and stored all the time
  uint16 temp1;
  uint16 temp2;
  uint16 temp3;
  
  // convert input signal to unsigned integer
  // input is inverted, so it needs to be flipped as well
  temp1 = 0x8000 - left_in;
  // fetch lookup table entry to convert CV to frequency
  temp2 = logtable[temp1 >> 8];
  // fetch next value to do interpolation
  temp3 = logtable[(temp1 >> 8) + 1];
  // interpolate result
  
  unsigned char frac = (temp1 & 0x00ff); // fetch the lower 8b
  // we are done with temp1, so we can reuse it here
  temp3 = (temp3 * frac) >> 8;
  // scaled sample 2 is now in temp3
  temp2 = (temp2 * (0xff - frac)) >> 8;
  // temp2 now has the scaled sample 1
  temp2 += temp3; // add samples together to get an average
  // our frequency value is now in temp2
  
  // we need a few more variables
  int16 temp4;
  int16 temp5;
  
  // create a variable frequency sinewave
  // step through table at rate determined by temp2
  location += temp2;
  // if weve gone over the table boundary -> loop back
  location &= 0x0003ffff; // fast way for 2^n values
  // fetch a sample from the lookup table
  temp1 = location >> 8;
  temp4 = sinewave[temp1];
  // get next value for interpolation
  temp5 = sinewave[(temp1 + 1) & 0x03ff];
  // interpolate result
  frac = (location & 0x000000ff); // fetch the lower 8b
  temp5 = (temp5 * frac) >> 8;
  // scaled sample 2 is now in temp3
  temp4 = (temp4 * (0xff - frac)) >> 8;
  // temp2 now has the scaled sample 1
  temp4 += temp5; // add samples together to get an average
  // our sinewave value is now in temp2
  
  left_out = temp4; // put sinusoid out on left channel
  right_out = -temp4; // put inverted version out on right chanel

  // you dont use reti() with Maple
}

