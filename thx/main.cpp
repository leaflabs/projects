/*
 * Generate the THX Deep Note using the wm8731 codec chip
 */

#include "wirish.h"
#include "usb.h"

#define BOARD_LED_PIN 13
#define BOARD_BUT_PIN 38
#define OUT0          7
#define OUT1          6
#define AUDIO_RATE    15      /* timer period in us */   
#define CONTROL_RATE  30000  /* timer period in us */   
#define NUM_CHANNELS  16
#define HALF_CHANNELS (NUM_CHANNELS/2)
#define STEP_BASELINE 4000000

#define LEN_START 300
#define LEN_TRANS 400

int16 current_sample0 = 0; /* 32 bits in case we use fixed point later */
int16 current_sample1 = 0;
int   step0 = 1 << 16;
int   step1 = 1 << 16;
int   index0 = 0;   
int   index1 = 0;   

typedef struct 
{
    int step;
    uint32 index;
    int value;
    int start;
    int dest;
} Channel;

Channel synth_r[HALF_CHANNELS];
Channel synth_l[HALF_CHANNELS];
int thx_mode = 0;

void handler_c(void) {
    if (thx_mode < LEN_START || thx_mode > (LEN_START+LEN_TRANS)) 
        {
            int divider = 2000;
            if (thx_mode > (LEN_START+LEN_TRANS)) 
                {
                    divider *= 10;
                }
            
            for (int i=0; i<HALF_CHANNELS;i++) {
                int range = synth_r[i].value/divider;
                synth_r[i].step += random(-1*range,range);

                range = synth_l[i].value/divider;
                synth_l[i].step += random(-1*range,range);
            }
        }
    else 
        {
            for (int i=0; i<HALF_CHANNELS;i++) 
                {
                    /* were in the first half of the criscendo */
                    int stride = (synth_r[i].dest-synth_r[i].start)/(LEN_TRANS);
                    synth_r[i].step += stride;

                    stride = (synth_l[i].dest-synth_l[i].start)/(LEN_TRANS);
                    synth_l[i].step += stride;
                    
                }
            
        }
    
    if (thx_mode == (LEN_START+(LEN_TRANS)/2)) 
        {
            Timer4.setPeriod(CONTROL_RATE/2); // in microseconds
        }        

    if (thx_mode == (LEN_START)) 
        {
            /* reset all the start values */
            for (int i=0;i<HALF_CHANNELS;i++) 
                {
                    synth_r[i].start = synth_r[i].step;
                    synth_l[i].start = synth_l[i].step;                    
                }
            
        }
    
    thx_mode++;
    
}

void handler_t(void) {
    /* runs every TIMER_RATE microsecond */
    int output_l = 0;
    int output_r = 0;
    
    for (int i=0;i<HALF_CHANNELS;i++) {
        synth_r[i].value = (synth_r[i].index % 2147483648)/HALF_CHANNELS;
        synth_r[i].index += synth_r[i].step;
   
        synth_l[i].value = (synth_l[i].index % 2147483648)/HALF_CHANNELS;
        synth_l[i].index += synth_l[i].step;

        output_r += synth_r[i].value;
        output_l += synth_l[i].value;
                
    }
    output_r = output_r/8388608;
    output_l = output_l/8388608;
    
    pwmWrite(OUT0,output_l);
    pwmWrite(OUT1,output_r);
    
}

void setup_synth(Channel *synth) {

    /* setup roughly where the channels start and end. todo generify against NUM_CHANNELS */
    synth[0].dest  = random(3990000,4010000);
    synth[0].start = random(4000000,4500000);

    synth[1].dest  = random(3990000,4010000);
    synth[1].start = random(4500000,5000000);

    synth[2].dest  = random(3990000,4010000) << 1;
    synth[2].start = random(5000000,5500000);
    
    synth[3].dest  = random(3990000,4010000) << 1;
    synth[3].start = random(5500000,6000000);

    synth[4].dest  = random(3990000,4010000) << 1;
    synth[4].start = random(6000000,6500000);

    synth[5].dest  = random(3990000,4010000) << 2;
    synth[5].start = random(6500000,7000000);
        
    synth[6].dest  = random(3990000,4010000) << 3;
    synth[6].start = random(7000000,7500000);

    synth[7].dest  = random(3990000,4010000) << 4;
    synth[7].start = random(7500000,8000000);    

    for (int i=0;i<8;i++) {
        synth[i].step = synth[i].start;
    }
    
}

void setup(void) {
    pinMode(BOARD_BUT_PIN,INPUT);
    pinMode(BOARD_LED_PIN,OUTPUT);

    // Setup output pwm pin (8 bit)
    pinMode(OUT0,PWM);
    pinMode(OUT1,PWM);
    Timer1.setOverflow(0xFF);
    Timer1.setPrescaleFactor(1);
    
    // Setup Timer
    Timer2.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer2.setPeriod(AUDIO_RATE); // in microseconds
    Timer2.setCompare1(1);      // overflow might be small
    Timer2.attachCompare1Interrupt(handler_t);

    Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setPeriod(CONTROL_RATE); // in microseconds
    Timer4.setCompare1(1);      // overflow might be small
    Timer4.attachCompare1Interrupt(handler_c);

    /* some random pin to grab a seed from */
    pinMode(20,INPUT_ANALOG);
    randomSeed(analogRead(20));

    /* setup the synths */
    setup_synth(synth_l);
    setup_synth(synth_r);
    

    Timer2.pause();
    while(!digitalRead(BOARD_BUT_PIN));
    Timer2.resume();
}

void loop(void) {
    static int last_but;

    int this_but = digitalRead(BOARD_BUT_PIN);
    if (this_but && !last_but) {
        step0 += 1;
        step1 += 2;
    }
    last_but = this_but;

    SerialUSB.println(step0);
    delay(1000);

    digitalWrite(BOARD_LED_PIN,HIGH);
    delay(100);
    digitalWrite(BOARD_LED_PIN,LOW);
    
}

// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated object that need libmaple may fail.
 __attribute__(( constructor )) void premain() {
    init();
}

int main(void) {
    setup();

    while (1) {
        loop();
    }

    return 0;
}
