/*
  Initial libmaple DMA (direct memory access) demo.

  This program demonstrates the Maple's DMA by doing ADC conversion on
  pin 15 via DMA requests instead of using analogRead().

  By default, it just prints out the average of every 1024 conversions
  to Serial1.  See the "Demo configuration variables" section, below,
  for other things it can do.
*/

#include "wirish.h"
#include "adc.h"
#include "dma.h"
#include "util.h"

/** Demo configuration variables *********************************************/

// If you set this to true, the ADC will be configured to take only
// 1.5 ADC input clock cycles, instead of libmaple's default 55.5.
// However, this means that the input impedance of what you're
// measuring must be below 400 Ohms. (The default of 55.5 cycles
// allows an input impedance of up to 50 KOhms).
const bool demo_fast_adc = false;

// If you set this to true, then after reset, the Maple will measure
// approximately how long it takes to fill the entire buffer, print
// some information about it, and then sit quietly and blink the LED.
const bool demo_time_conversions = true;

// Which pin to sample
#define ADC_PIN 18

/** Declarations, buffers, etc. **********************************************/

// The size of the buffer that the DMA controller will write into.
// For simplicity, this must be a power of 2.
#define BUF_SIZE 2048

// This is a pointer to ADC1's data register, where the results of ADC
// conversion are stored for software to pick up.
volatile uint32* p_adc1_dr = (volatile uint32*)(ADC1_BASE + 0x4C);

// Each DMA request will place a converted value into this array.  The
// "__attribute__ ((aligned (16)))" bit has to do with some low-level
// DMA details regarding memory alignment that I won't go into here.
uint16 adc_buf[BUF_SIZE] __attribute__((aligned (16)));

// This will be the handler, which we'll configure to be called
// whenever adc_buf is half full or entirely full of new values.
void adc_dma_handler(void);

// If we're timing how long it takes to do conversions, then this
// variable is set to true by adc_dma_handler() when it's all done, so
// loop() knows when to print statistics.  It's declared "volatile"
// because its value is changed by an interrupt handler; see
// http://leaflabs.com/docs/lang/api/attachinterrupt.html#discussion
volatile bool stop_timing = false;

// If we're timing conversions, keep track of when we started.
uint32 start_time;

/** setup()/loop() ***********************************************************/

void setup() {
    /** Peripheral and pin setup *********************************************/

    Serial1.begin(115200);
    pinMode(BOARD_LED_PIN, OUTPUT);
    pinMode(ADC_PIN, INPUT_ANALOG);

    /** DMA configuration ****************************************************/

    // dma_init() sets up a dma transfer to occur, but needs some
    // information on what kind of transfer it will be.  This is just
    // a bitwise OR of whichever dma_mode_flags you want (see
    // libmaple/dma.h).
    //
    // The combination of modes we use will cause adc_buf to be filled
    // over and over with converted values, and will cause an
    // interrupt to be raised every time the buffer is half full or
    // entirely full.
    int dma_mode =
        (DMA_MINC_MODE /* Increment the memory address written to
                          after each DMA request.  That is, first
                          write to adc_buf, then to (adc_buf+1), and
                          so forth. */ |
         DMA_CIRC_MODE /* Circular mode: once the adc_buf is full,
                          wrap around to the beginning. */ |
         DMA_HALF_TRNS /* Raise an interrupt when half of the transfer
                          is complete */ |
         DMA_TRNS_CMPLT /* Raise an interrupt when the transfer is
                           complete */);

    // Configure DMA to receive ADC requests
    dma_init(1 /* ADC1 is on DMA channel 1 */,
             p_adc1_dr /* Read from ADC1's data register */,
             DMA_SIZE_16BITS /* Peripheral size: ADC1_DR holds 16 bits */,
             DMA_SIZE_16BITS /* Memory size: adc_buf is an array of uint16s */,
             dma_mode /* Discussed above */);

    // Attach our interrupt, adc_dma_handler(), to be called whenever
    // DMA channel 1 wants to interrupt us.
    dma_attach_interrupt(1, adc_dma_handler);

    // Tell the DMA controller to begin handling requests.  We haven't
    // configured the ADC to actually start converting yet, so while
    // it's ready to handle requests, it will receive none (yet).
    dma_start(1, adc_buf, BUF_SIZE);

    /** ADC configuration ****************************************************/

    adc_disable();

    // Keep libmaple's default sample rate unless requested to do
    // otherwise.
    if (demo_fast_adc) {
        adc_init(ADC_SMPR_1_5);
    } else {
        adc_init(ADC_SMPR_55_5);
    }

    // If we're timing conversions, start the clock.
    if (demo_time_conversions) {
        start_time = micros();
    }

    // Ask the pin map which ADC line pin 15 corresponds to, then
    // configure the ADC to do conversions on that pin.
    ADC_SQR3 = PIN_MAP[ADC_PIN].adc;

    // Configure the ADC to start generating DMA requests through
    // ADC_CR2 (ADC control register 2).
    ADC_CR2 |= ((1 << 8) /* Generate a DMA request after each
                            conversion. */ |
                (1 << 1)); /* Continuous conversion: keep converting
                              forever. */

    CR2_ADON_BIT = 1;           // wtf why
}

void loop() {
    if (demo_time_conversions) {
        if (stop_timing) {
            // The DMA interrupt handler detected a full buffer, so
            // stop the clock.  (We lose a little accuracy in between
            // the time the interrupt handler set stop_timing to true
            // and now, but it's at least a conservative estimate).
            uint32 end_time = micros();
            uint32 elapsed_time = end_time - start_time;
            Serial1.println("**** Finished converting ****");
            Serial1.print("start_time = ");
            Serial1.print(start_time);
            Serial1.print(", end_time = ");
            Serial1.print(end_time);
            Serial1.print(", elapsed time = ");
            Serial1.print(elapsed_time);
            Serial1.print(" us. sample frequency = ");
            Serial1.print(BUF_SIZE / double(elapsed_time) * 1000000);
            Serial1.println(" samples/sec.  Press reset to go again.");
            for (int i = 0; i < BUF_SIZE; i++) {
                Serial1.print(adc_buf[i]);
                Serial1.print(" ");
            }
            Serial1.println();
            while (true) continue;
        }
        delayMicroseconds(1000); // FIXME take out
    } else {
        toggleLED();
        delay(250);
    }
}

/** DMA interrupt handler ****************************************************/

// This interrupt handler gets called whenever the buffer adc_buf is
// half-full or full.
void adc_dma_handler(void) {
    /** Figure out what caused the interrupt *********************************/

    // DMA1_ISR is the DMA1 interrupt status register.  Its bits
    // contain the reason why this interrupt handler is being
    // executed.  Bit 2 is set if we just finished a half-transfer;
    // bit 1 is set if we just finished a full transfer.
    //
    // Thus, `halfway_through' will be true if we are halfway through
    // the transfer (i.e., if we just finished converting
    // adc_buf[0]...adc_buf[BUF_SIZE/2-1]), and false if we are
    // finished with the transfer (i.e., if we just finished
    // converting adc_buf[BUF_SIZE/2]...adc_buf[BUF_SIZE-1]).
    int status_bits = __get_bits(DMA1_ISR, BIT(2) | BIT(1));
    bool halfway_through = status_bits & BIT(2);
    bool finished = status_bits & BIT(1);

    // This clears the relevant bits in DMA1_ISR, so that the next
    // time we get called, it's with fresh information.
    __set_bits(DMA1_IFCR, 0x6);

    /** See if we're doing a speed test **************************************/
    if (demo_time_conversions) {
        if (finished) {
            stop_timing = true;
        }
    }
    /** No speed test, do averaging instead **********************************/
    else {
        // Decide where to start looking in adc_buf for new data.
        int start;
        if (halfway_through) {
            // If we're halfway through, then start at 0, since the first
            // half has the new ADC readings.
            start = 0;
        } else {
            // Otherwise, we just finished filling the buffer.  Since
            // we've already looked at the first half, start looking
            // midway through adc_buf for new readings.
            start = BUF_SIZE / 2;
        }

        // Compute the total of all the new readings.
        uint32 total = 0;
        for (int i = start; i < start + BUF_SIZE / 2; i++) {
            total += adc_buf[i];
        }

        // Print the average over Serial1.
        Serial1.println(total / (BUF_SIZE / 2));
    }
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
