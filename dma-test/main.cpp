#include "wirish.h"
#include "adc.h"
#include "dma.h"
#include "util.h"

#define ADC_PIN 15

uint16 adc_buf[512] __attribute__((aligned (16)));

void adc_dma_handler(void);

void setup() {
    Serial1.begin(115200);

    // Configure DMA to receive ADC requests
    dma_init(1, (volatile uint32*)(ADC1_BASE + 0x4C),
             DMA_SIZE_16BITS, DMA_SIZE_16BITS,
             (DMA_MINC_MODE | DMA_CIRC_MODE | DMA_TRNS_CMPLT));
    dma_attach_interrupt(1, adc_dma_handler);
    dma_start(1, adc_buf, 512);

    // Configure ADC to start generating interrupts
    ADC_CR2 |= ((1 << 8)                /* generate DMA requests */ |
                (1 << 1));              /* continuous conversion */
    ADC_SQR3 = PIN_MAP[ADC_PIN].adc;
    CR2_ADON_BIT = 1; /* go! */
}

void adc_dma_handler(void) {
    int start, end;
    bool half = __get_bits(DMA1_ISR, 2);
    uint32 total = 0;
    __set_bits(DMA1_IFCR, 0x7);

    if (half) {
        start = 0;
        end = 256;
    } else {
        start = 256;
        end = 512;
    }

    for (int i = start; i < end; i++) {
        total += adc_buf[i];
    }

    Serial1.println(total / 256);
}

void loop() {
    delay(10000);
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
