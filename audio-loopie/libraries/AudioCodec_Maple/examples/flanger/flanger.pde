/*
flanger.pde (MAPLE)
guest openmusiclabs 7.14.11
this program implements a mono flanger.  since we have more
processing time on the Maple, we can do a regular lfo for
the mod rate.  it takes input from the left channel, and delays
it by a varying amount.  the rate of this variation is set by
the MOD1 knob, and the amplitude is set by the MOD0 knob.  it
then presents this delayed data on the right channel, and a mix
of the wet and dry on the left channel.
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
// even though the function is mono, the codec requires stereo data
// note the int16, Maple is 32b int
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

// create variables for ADC results
// it only has positive values -> unsigned
// note the uint16, there is no "unsigned int16" in maple
uint16 mod0_value = 0;
uint16 mod1_value = 0;

// create sinewave lookup table
int16 sinewave[] __FLASH__ = {
  // this file is stored in the AudioCodec_Maple library and is a
  // 1024 value sinewave lookup table of signed 16bit integers.
  // you can replace it with your own waveform if you like.
  #include <sinetable.inc>
};
unsigned int location; // lookup table value location

// create a delay buffer in memory
#define SIZE 800 // buffer size is limited by microcontroller SRAM size
int16 delaymem[SIZE]; // 800 positions x 2 bytes = 1600 bytes of SRAM
uint16 position = 0; // buffer position to read/write from

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

  // create a flanger effect by moving through delayed data
  // store incoming data
  delaymem[position++] = left_in; // post increment position to go to next memory location
  // check if position has gotten bigger than buffer size
  if (position >= SIZE) {
    position = 0; // reset position
  }
  // fetch delayed data with sinusoidal offset (temp2)
  uint16 x; // create a temporary buffer index
  x = position + (SIZE/2) + (temp2 >> 8);
  if (x >= SIZE) { // check for buffer overflow
    x -= SIZE;
  }
  // fetch delayed data
  temp1 = delaymem[x];
  // fetch next delayed data for interpolation
  if (++x == SIZE) { // check for buffer overflow
    x = 0;
  }
  // we need some more temp variables
  int16 temp3;
  temp3 = delaymem[x];
  // interpolate between values
  temp3 = (temp3 * (temp2 & 0xff)) >> 8;
  temp1 = (temp1 * (0xff - (temp2 & 0xff))) >> 8;
  // put delayed data at right output
  right_out = temp3 + temp1;
  // put modulated data at right output
  // mix modulated and current data at left output
  // divide each by 2 for proper scaling
  left_out = (right_out >> 1) + (left_in >> 1);


  // & is required before adc variables
  AudioCodec_ADC(&mod0_value, &mod1_value);
  
  // you dont need reti() with Maple
}

