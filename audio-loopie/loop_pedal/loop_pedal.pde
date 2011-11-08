/*
variable_delay.pde (MAPLE)
guest openmusiclabs 7.13.11
this program takes input from the left channel, and delays it
by a variable amount, set by the MOD0 knob.  it then presents
this delayed data on the left channel, with the non-delayed
data coming out of the right channel.
*/
#include <stdlib.h>
// setup codec parameters
// must be done before #includes
// see readme file in libraries folder for explanations
#define SAMPLE_RATE 44 // 44.1kHz sample rate
#define ADCS 1 // only 1 ADC used here -> MOD0
#define MICBOOST 1

// include necessary libraries
// note the Maple suffix on the library
#include <AudioCodec_Maple.h>

// create data variables for audio transfer
// even though the function is mono, the codec requires stereo data
// note the int16 typedef, int is 32bit on Maple
int16 left_in = 0; // in from codec (LINE_IN)
int16 right_in = 0;
int16 left_out = 0; // out to codec (HP_OUT)
int16 right_out = 0;

// create variable for ADC result
// it only has positive values -> unsigned
// note uint16 typedef, unsigned doesnt work with Maple
uint16 mod0_value = 0;

// create a delay buffer in memory
#define SIZE 512000 // buffer size is limited by microcontroller SRAM size
//int16 delaymem[SIZE]; // 7000 positions x 2 bytes = 14k bytes of SRAM
uint16* delaymem;
uint32 location = 0; // buffer location to read/write from
uint64 boundary = 0; // end of buffer position

void setup() {
  pinMode(BOARD_BUTTON_PIN,INPUT);
  pinMode(BOARD_LED_PIN,OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  delaymem = (uint16*)malloc(2*SIZE);
  int i;
  for (i=0;i>SIZE;i++) {
    delaymem[i] = 0;
  }
  //delaymem = (int16*)0x6000000;
  SerialUSB.end(); // usb conflicts with the codec in this mode
                   // a different operating mode which allows both
                   // is in the works
  // setup codec and microcontroller registers
  AudioCodec_init(); // call this last if you are setting up other things
}

int record = false;
void loop() {
  waitForButtonPress();
  toggleLED();
  boundary = 0;
  location = 0;
  record = true;
  delay(5);
  
  waitForButtonPress();
  toggleLED();
  boundary = location;
  location = 0;
  record = false;
}

// timer4_ch1 interrupt routine - all data processed here
void AudioCodec_interrupt() {

  // &'s are necessary on data_in variables
  AudioCodec_data(&left_in, &right_in, left_out, right_out);
  
  // pass left input to right output
  right_out = left_in;
  
  
  //fetch data from buffer and put it into output register
  if (!record) {
     left_out = delaymem[location];
  }
  // put new data in same location for maximal delay time
  if (record) {
      delaymem[location++] = left_in; // post increment location to go to next memory location
  } else {
    location++;
  }
  
  // check if location has gotten bigger than buffer size
  uint32 wrap;
  if (record) {
    wrap = SIZE;
  } else {
    wrap = boundary;
  }
  
  if (location >= wrap) {
      location = 0; // reset location
  }

  // & is required before adc variable
  //AudioCodec_ADC(&mod0_value);

  // scale ADC value to match buffer size
  // since it returns a 32bit result, it needs to be scaled with ">> 16"
  //boundary = (mod0_value * SIZE) >> 16;

  // you dont need to use reti() with Maple
}

