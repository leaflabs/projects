/*
sine_generator.pde (MAPLE)
guest openmusiclabs 7.13.11
this program creates a sinewave of variable frequency and
amplitude, presented at both left and right outputs. there
isnt any interpolation, so you only get 256 discrete frequencies
across the span of 44Hz to 10kHz.
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
// note the uint16 typedef, there is no "unsigned int16" type
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

// create variables for ADC results
// it only has positive values -> unsigned
uint16 mod0_value = 0;
uint16 mod1_value = 0;

// create sinewave lookup table
// note the int16, ints are 32b in Maple land
int16 sinewave[] __FLASH__ = {
  // this file is stored in AudioCodec_Maple.h and is a 1024 value
  // sinewave lookup table of signed 16bit integers
  // you can replace it with your own waveform if you like
  #include <sinetable.inc>
};
uint16 location; // lookup table value location


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
  int16 temp1;
  
  // create a variable frequency and amplitude sinewave
  // fetch a sample from the lookup table
  temp1 = sinewave[location];
  // step through table at rate determined by mod1
  // use upper byte of mod1 value to set the rate
  // and have an offset of 1 so there is always an increment.
  location += 1 + (mod1_value >> 8);
  // if weve gone over the table boundary -> loop back
  // around to the other side.
  location &= 0x03ff; // fast way of doing rollover for 2^n numbers
                      // otherwise it would look like this:
                      // if (location >= 1024) {
                      // location -= 1024;
                      // }
  
  // set amplitude with mod0
  // multiply our sinewave by the mod0 value
  // since it returns a 32bit value it needs to be scaled with ">> 16"
  temp1 = (temp1 * mod0_value) >> 16;
  left_out = temp1; // put sinusoid out on left channel
  right_out = -temp1; // put inverted version out on right chanel

  // get ADC values
  // & is required before adc variables
  AudioCodec_ADC(&mod0_value, &mod1_value);

  // you dont need to reti() with Maple
}

