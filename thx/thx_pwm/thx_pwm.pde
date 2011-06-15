/*
 * Generate the THX Deep Note, in stereo, using two PWM outputs and 16 sawtooth
 * waves.
 *
 * To try this out yourself you will need:
 *  * ~1K Resistors (for volume control), need 2
 *  * A way to listen to audio from wires (a breadboardable headphone jack, 
 *    or actual speakers with wires
 *  
 * Wire up your speakers/headphone jack to pins 25 and 26 on Maple Mini,
 * put the 1K resistors in series with this connection to keep from blowing
 * your ears off! A big speaker might not need the resistor. Dont forget to
 * wire up the ground of your headphones/speakers to groun on Maple Mini!
 *
 * To use with Maple (not Maple Mini) change OUT0 and OUT1 to "6" and "7"
 * and wire your speakers to those pins instead. This is because TIMER1,
 * on which we have configured the PWM, connects to different pins on Maple
 * and Maple Mini!
 *
 * See how many channels you can add before running out of processor power!
 * The original Deep Note had 30! The current record for a Maple version is
 * 25 (then the audio mixer cant keep up with the audio rate!)
 */


/*
 * The Arduino preprocessor does not handle structs and typedefs gracefully,
 * a workaround is to make a new tab, define your structs and typedefs there
 * and then manually add '#include "name_of_tab" ' to the top of your sketch
 */
#include "Channel.h"


#define OUT0          25     /* Left Channel PWM */
#define OUT1          26     /* Right Channel PWM */
#define AIN_SEED      3      /* Analog Pin to sample for entropy */
#define AUDIO_RATE    15     /* in microseconds */
#define CONTROL_RATE  30000  /* in microseconds */
#define NUM_CHANNELS  16
#define HALF_CHANNELS (NUM_CHANNELS/2)

#define LEN_START 300 /* multiply by CONTROL_RATE to get length of intro */
#define LEN_TRANS 300 /* length of transition to criscendo */

/* The size of the values used to represent any audio/dsp math
DONT use MAX_INT32 to give ourselves headroom and avoid clipping */
#define AUDIO_BITS 31
#define MAX_VAL (1 << AUDIO_BITS)

/* Use 8-bit pwm, since its very fast and we arnt filtering the output */
#define PWM_BITS 8

/* To convert the AUDIO_BITS audio samples into PWM_BITS output samples,
   divide by 2^(AUDIO_BITS-PWM_BITS) */
#define OUTPUT_SCALE (1 << (AUDIO_BITS-PWM_BITS))

/* The hand tuned values, like the starting and stopping frequencies of each
   synth channel and the number of channels set to each octave of the final
   chord.
*/
#define DEST_LOW_BOUND  3990000
#define DEST_UP_BOUND   4010000
#define START_LOW_BOUND 4000000
#define START_INCR       500000
#define NUM_OCTAVES           5
int number_each_octave[NUM_OCTAVES] = {2,3,1,1,1};

/* Create the "synthesizer" */
Channel synth[NUM_CHANNELS];

/* Control counter, increments every CONTROL_RATE */
int thx_mode = 0;

/* Control Timer Callback, runs every CONTROL_RATE */
void handler_c(void) {
    /* Either the intro or after the criscendo */
    if (thx_mode < LEN_START || thx_mode > (LEN_START+LEN_TRANS)) {
       int divider = 2000;
       if (thx_mode > (LEN_START+LEN_TRANS)) {
           /* move more randomly AFTER the criscendo */
           divider *= 10;
       }

       for (int i=0; i<NUM_CHANNELS; i++) {
           int range = synth[i].value/divider;
           synth[i].step += random(-1*range,range);
       }
    } else {
        for (int i=0; i<NUM_CHANNELS; i++) {
            /* were in the criscendo */

            /* we could save cycles making stride global, but
               we are not low on cycles */
            int stride = (synth[i].dest-synth[i].start)/(LEN_TRANS);
            synth[i].step += stride;
        }
    }

    if (thx_mode == (LEN_START)) {
        /* reset all the start values so that we can compute
           the slope of the criscendo, which is a linear slide
           with slope:
           (destination_frequency - starting_frequency)/length_criscendo
        */
        for (int i=0;i<NUM_CHANNELS;i++) {
            synth[i].start = synth[i].step;
        }
    }

    thx_mode++;
}

/* Audio handler callback, runs every AUDIO_RATE */
void handler_t(void) {
    int output_l = 0;
    int output_r = 0;

    for (int i=0;i<HALF_CHANNELS;i++) {
        /* The left output is the first half of the synth channels and the
           right output is the second half. Each channel is a sawtooth wave
           with amplitude MAX/HALF_CHANNELS */
        synth[i].value = (synth[i].index % MAX_VAL)/HALF_CHANNELS;
        synth[i].index += synth[i].step;

        int j = i+HALF_CHANNELS;
        synth[j].value = (synth[j].index % MAX_VAL)/HALF_CHANNELS;
        synth[j].index += synth[j].step;

        output_r += synth[i].value;
        output_l += synth[j].value;
    }

    output_r = output_r/OUTPUT_SCALE;
    output_l = output_l/OUTPUT_SCALE;

    pwmWrite(OUT0,output_l);
    pwmWrite(OUT1,output_r);
}

/* this function hand tunes how many of each destination octave should be
 * present across all of the synthesizer channels. num_each_octave is setup by hand
 */
int get_octave(int index) {
    int i;
    int sum = 0;
    for (i=0; i<NUM_OCTAVES; i++) {
        sum += number_each_octave[i];
        if (index < sum) {
            return i;
        }
    }

    /* we should not ever get here */
    SerialUSB.println("Error, channel index exceeds total number of octaves");
    SerialUSB.println("Randomly assigning an octave to this channel");
    return random(0,NUM_OCTAVES);
}

void setup_synth(Channel *synth) {
    int i;

    for (i=0; i<NUM_CHANNELS; i++) {
        /* i%(NUM_CHANNELS/2) repeats our settings for the left and right
           sets of synths channels */
        int ind = i % (NUM_CHANNELS/2);
        int start_low_bound = START_LOW_BOUND + (START_INCR)*ind;

        synth[i].start = random(start_low_bound,start_low_bound+START_INCR);
        synth[i].dest = random(DEST_LOW_BOUND,DEST_UP_BOUND);

        /* multiply the dest frequency by 2^octave, where which octave is
           determined by hand tuning */
        synth[i].dest <<= get_octave(ind);
        synth[i].step = synth[i].start; /* step is an analog to frequency */

    }
}

/* 0.0.11 breaks timer setPeriod, so lets provide a working version here */
uint16 timer_set_period(HardwareTimer timer, uint32 microseconds) {
  if (!microseconds) {
    timer.setPrescaleFactor(1);
    timer.setOverflow(1);
    return timer.getOverflow();
  }

  uint32 cycles = microseconds*(72000000/1000000); // 72 cycles per microsecond

  uint16 ps = (uint16)((cycles >> 16) + 1);
  timer.setPrescaleFactor(ps);
  timer.setOverflow((cycles/ps) -1 );
  return timer.getOverflow();
}


void setup(void) {
    pinMode(BOARD_BUTTON_PIN,INPUT);
    pinMode(BOARD_LED_PIN,OUTPUT);

    /* Setup output pwm pin (PWM_BITS pwm) */
    pinMode(OUT0,PWM);
    pinMode(OUT1,PWM);
    Timer1.setOverflow(2<<PWM_BITS);
    Timer1.setPrescaleFactor(1); // go as fast as possible
    pwmWrite(OUT0,0);
    pwmWrite(OUT1,0);

    /* Setup Timer */
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);

    /* 0.0.11 IDE bug, setPeriod is no good */
    //Timer2.setPeriod(AUDIO_RATE); // in microseconds
    timer_set_period(Timer2,AUDIO_RATE);
    Timer2.setCompare1(1);      // overflow might be small
    Timer2.attachCompare1Interrupt(handler_t);

    Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    //Timer4.setPeriod(CONTROL_RATE); // in microseconds
    timer_set_period(Timer4,CONTROL_RATE);
    Timer4.setCompare1(1);      // overflow might be small
    Timer4.attachCompare1Interrupt(handler_c);

    /* some random pin to grab a seed from */
    pinMode(AIN_SEED,INPUT_ANALOG);
    randomSeed(analogRead(AIN_SEED));

    Timer2.pause();
    Timer4.pause();
    
    waitForButtonPress();
    
    /* setup the synths */
    setup_synth(synth);
    
    Timer2.resume();
    Timer4.resume();
}

void loop(void) {
    /* blink occasionally to prove not dead from audio overrun */
    digitalWrite(BOARD_LED_PIN,HIGH);
    delay(100);
    digitalWrite(BOARD_LED_PIN,LOW);
    delay(800);
}

