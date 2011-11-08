AudioCodec_Maple_readme.txt (MAPLE)
guest openmusiclabs 7.7.11


this file describes the various functions used in the AudioCodec library for the Maple platform.  please note that "SerialUSB.end()" must be run in the init function, so you will have to manually reset the Maple to load new code.  this is a bit of a pain, but it does this to get rid of the noise the USB interrupts place on the line.  there is a non-blocking version of this library in the works, for use in applications where you want to do more than just process audio.

also note, there is a patch for the timer functions which must be installed for this library to work.  the patch file is in the "\patches" folder in the "\AudioCodec_Maple" folder.  there a read_me file in that folder that describes what it does and where to put it.

to use this library, you must place it in one of two locations.  the usual place is in:

C:\Documents and Settings\user\My Documents\MapleIDE\libraries\

where "user" is your user name.  this is where Maple stores your sketches.  you will have to make the "libraries" folder if you have not done so already for another project.  the advantage of putting the library in this location, is that further updates to Maple_IDE will not effect your personal libraries.  the disadvantage is that you have to place it in that folder for each user on the machine.  if you are the only user, its not much of a problem.  otherwise, you can place it in:

~\maple-ide-0.0.11-windowsxp32\libraries\

for now, the library is called AudioCodec_Maple, but a universal library will probably come out at some point.  be sure to include the library at the beginning of your sketch as follows:

#include <AudioCodec_Maple>


FUNCTIONS
---------

AudioCodec_init() - this initializes the codec and microcontroller.  it turns off all interrupts, so functions such as delay() and SerialUSB() will not work.  it does this to reduce clock jitter and increase the amount of processing time available.  It uses TIMER4_CH1 to count codec clock cycles to synchronize data transfers, and uses  the SPI1 hardware peripheral to communicate with the codec.  ADC0 and ADC1 are also used for the MOD pot readings.  The I2C is bit-banged on pins 19 and 20.

AudioCodec_data(&left_in, &right_in, left_out, right_out) - this transfers data to and from the codec.  it must take 4 arguments, even if you are only dealing with mono data, and the first two arguments (the inputs) must have & before them.  the data is transfered as signed 16bit integers.  the total transfer time is about 4us, although a lot of this time is spent idling.  if you really need to save some time, you can copy this function directly into your sketch, and put some other functions in the while() loops.

AudioCodec_ADC(&mod0_value, &mod1_value) - this fetches the ADC values from the MOD pots.  it can take either one or two arguments, but they must have the & before them.  the ADC is a 12bit ADC, but the values are oversampled 16 times to give a 16bit unsigned integer.  the values are further windowed by a hyterisis value HYST to eliminate random noise.  this is particularly useful when the ADC value is setting a delay time, although for some some functions, like setting of VCO frequencies, it can create a nice vibrato effect.  you can turn off the hysterisis effect by setting HYST 0 (see definitions below).


CODEC SETTINGS
--------------

the codec can be set to various bit rates and volumes, and these are all settable with the #define statement at the beginning of your sketch.  these must be placed before the #include statements, and not have an equals sign or semicolon afterwards.  for example:

#define SAMPLE_RATE 44

all of these settings take only whole number values, and have a default setting if no value is given to them.  the following is a listing of currently implemented settings you can vary.  the range of possible values is given in brackets [], with the default value in parens ().  if the setting can take a range of values, the range will be seperated with an arrow ->.

SAMPLE_RATE [2, 8, 22, (44), 88]
- this sets the codec sample rate in approximate kHz.
88 = 88.2kHz (Fck/128)
44 = 44.1kHz (Fck/256)
22 = 22.05kHz (Fck/256*2)
8 = 8.018kHz (Fck/256*5.5)
2 = 2.45kHz (Fck/256*18)

ADCS [0, 1, (2)]
- this sets the number of ADCs to be sampled by the microcontroller.  you should always set this to the minimum number of ADCs you intend to use, as sampling the ADCs takes up precious processing time.

ADCHPD [(0), 1]
- this either turns on or off the digital highpass filter on the codec input.  this filter ensures that the signal you are sampling is centered in the range.  if you want to have a DC control voltage as in input to the codec, you will need to turn this off by setting ADCHPD 1.  for all other signals, its best to leave it enabled, as its well below the audible range and will not effect the sound quality.

HYST [0 -> 255, (32)]
- this sets the deadband hysteresis on the adc samples.  this eliminates jitter in yor adc sample value due to random noise.  it takes up a bit more processing time, so if you do not need it, you can set HYST 0 to turn it off.

LINVOL [0 -> 31, (23)]
- this sets the left line-in volume, in 1.5dB steps from -34.5dB to +12dB. the default setting is 0dB.

RINVOL [0 -> 31, (23)]
- this sets the right line-in volume, same as above.

LHPVOL [0, 48 -> 127, (121)]
- this sets the left headphone volume output in 1dB steps from -73dB to +6dB.  the default setting is 0dB. LHPVOL 0 mutes the output, as does any value from 0 -> 47, but for simplicity's sake, mute is given as 0 with the other values not defined.

RHPVOL [0, 48 -> 127, (121)]
- this sets the right headphone volume output, same as above.

MICBOOST [(0), 1]
- this either enables or disables a fixed volume boost on the microphone input. MICBOOST 0 disables the boost, and MICBOOST 1 gives a +10dB volume increase.

MUTEMIC [0, (1)]
- this eithe enables or disables the microphone input to the ADC.  MUTEMIC 1 mutes the signal to the ADC.

INSEL [(0), 1]
- this selects wether the microphone or line-in inputs go the ADC. INSEL 0 selects the line-in inputs, and INSEL 1 selects the microphone input.

BYPASS [(0), 1]
- this either enables or disables a pass-through from line-in to line-out, without any digitization.  BYPASS 0 disables the pass-through.  this only applies to the line-in inputs.

DACSEL [0, (1)]
- this either enables or disables the DAC from presenting its signal at the output.  DACSEL 1 enables the DAC output.

SIDETONE [(0), 1]
- this either enables or disables the sidetone signal at the output. the sidetone is a reduced version of the microphone input signal that can be presented at the output. it is often used in telephony applications to allow the speaker to here what she is saying. SIDETONE 0 disables the sidetone.  this only applies to the microphone input.

SIDEATT [(0) -> 3]
- this sets the level of the sidetone signal that appears at the output.  the level varies from -6dB at SIDEATT 0 to -15dB at SIDEATT 3, in -3dB steps.

