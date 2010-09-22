/*
 * VGA leaf logo fade-in, September 2010.
 *
 * Adapted from VGA Pong by Marti Bolivar
 *
 * This code is released into the public domain.
 */

#include "wirish.h"

/* Pinouts -- you also must change the GPIO macros below if you change
 * these.  Increasing number means more significant bit. */

#define R0_PIN 34               // STM32: B15
#define R1_PIN 33               // STM32: B14
#define R2_PIN 32               // STM32: B13

#define G0_PIN 31               // STM32: B12
#define G1_PIN 30               // STM32: B11
#define G2_PIN 29               // STM32: B10

#define B0_PIN 28               // STM32: B1
#define B1_PIN 27               // STM32: B0
#define B2_PIN 24               // STM32: B9

#define VS_PIN 21               // STM32: C13
#define HS_PIN 22               // STM32: C14

/* color GPIO (general purpose input/output) macros
 *
 * (All of the low-level macros that follow are necessary for speed;
 * digitalWrite just isn't fast enough for VGA's 25 MHz pixel clock.
 * But hey, at least we didn't have to write any assembly!) */

// A pointer to the STM32F103 GPIO port B bit set/reset register.  See
// ST's reference manual, especially sections 2.3 and 8.2.5:
// http://www.st.com/stonline/products/literature/rm/13902.pdf
//
// Writing to this lets us deterministically update all of the color
// values we send to the monitor in a single cycle, which is crucial
// for getting decent quality VGA output. (Note that it still takes
// multiple cycles when you include loading the value from memory,
// etc.)
#define BBSRR ((volatile uint32*)0x40010C10)

// What bits to write in BBSRR to update the corresponding pins.
#define R0_BIT 15
#define R1_BIT 14
#define R2_BIT 13

#define G0_BIT 12
#define G1_BIT 11
#define G2_BIT 10

#define B0_BIT 1
#define B1_BIT 0
#define B2_BIT 9

// convenience macros for color values.  BIT(n) produces an integer
// with the nth bit set.
//
// black
#define C_BLACK (0)
// primary colors at various strengths
#define C_R0 (BIT(R0_BIT))
#define C_R1 (BIT(R1_BIT))
#define C_R2 (BIT(R2_BIT))
#define C_G0 (BIT(G0_BIT))
#define C_G1 (BIT(G1_BIT))
#define C_G2 (BIT(G2_BIT))
#define C_B0 (BIT(B0_BIT))
#define C_B1 (BIT(B1_BIT))
#define C_B2 (BIT(B2_BIT))
// primary colors
#define C_RED   (C_R0 | C_R1 | C_R2)
#define C_GREEN (C_G0 | C_G1 | C_G2)
#define C_BLUE  (C_B0 | C_B1 | C_B2)
// secondary colors
#define C_MAGENTA (C_R2 | C_B2)
#define C_CYAN    (C_G2 | C_B2)
#define C_YELLOW  (C_G2 | C_R2)

// white
#define C_WHITE (C_RED | C_GREEN | C_BLUE)

// given an color (which is an OR of BIT applied to the [RGB][012]_BIT
// macros, above), convert it into a 32-bit integer which, if written
// to the BSRR, will set the corresponding [RGB][012]_PIN pins.
#define COLOR_TO_BSRR(c) \
    ((c) |                                                              \
     BIT(R0_BIT+16) | BIT(R1_BIT+16) | BIT(R2_BIT+16) |                 \
     BIT(G0_BIT+16) | BIT(G1_BIT+16) | BIT(G2_BIT+16) |                 \
     BIT(B0_BIT+16) | BIT(B1_BIT+16) | BIT(B2_BIT+16))

// Assumes the argument is an appropriate value for sending to the
// BSRR.  see COLOR_TO_BSRR for a convenient way to get such a value.
#define VGA_SET_BSRR(bsrr) (*BBSRR = (bsrr))

/* Hsync/vsync GPIO macros */

// STM32 port C BSRR and bit reset register BRR.  See the STM32
// manual, referenced above.
#define CBSRR ((volatile uint32*)0x40011010)
#define CBRR  ((volatile uint32*)0x40011014)

// Bits to write in CBSRR/CBRR to set/reset the vsync/hsync pins.
#define VS_BIT 13
#define HS_BIT 14

// Macros for sending hsync/vsync to the monitor.
#define VGA_VS_HIGH (*CBSRR = BIT(VS_BIT))
#define VGA_VS_LOW  (*CBRR  = BIT(VS_BIT))
#define VGA_HS_HIGH (*CBSRR = BIT(HS_BIT))
#define VGA_HS_LOW  (*CBRR  = BIT(HS_BIT))

/* VGA state */

// We take 2287 cycles to draw a full horizontal line (based on a 72
// MHz clock).  This is the cycle when we start drawing the visible
// part of the screen.
#define START_IMAGE 400

// # bits of color
#define N_COLOR_BITS 9

// Controls the dimensions of the buffer we use as a frame.  Note that
// we don't/can't support full resolution -- at 640 x 480, 9-bit color
// would require ~340KB RAM, and we only have 20KB total, of which
// 17KB is currently usable by a sketch.
#define FRAME_WIDTH 54
#define FRAME_HEIGHT 48        // you want this to be a divisor of 480

// this is (hackishly) used to force delays
volatile uint8 volatile_int = 0;

// vga state
uint16 x = 0;       // X coordinate
uint16 y = 0;       // Y coordinate
uint8 v_active = 1; // Are we in the image?

// The frame buffer.  Each cell is an OR of BIT applied to the color
// bits R0_BIT,R1_BIT,...,B1_BIT,B2_BIT, so we get 9-bit color with 3
// bits for each of R, G, and B.
//
// access like frame[y][x], where frame[0][0] is upper left, and
// frame[FRAME_HEIGHT-1][FRAME_WIDTH-1] is bottom right
uint32 frame[FRAME_HEIGHT][FRAME_WIDTH];
uint32 *frame_row;    // cache our current row at hsync time, for speed

// convenience
uint8 color_pins[] = {R0_PIN, R1_PIN, R2_PIN,
                      G0_PIN, G1_PIN, G2_PIN,
                      B0_PIN, B1_PIN, B2_PIN};

// 12-color color wheel
#define N_COLOR_WHEEL_COLORS 12
uint32 bsrr_color_wheel[] = \
    {COLOR_TO_BSRR(C_RED),
     COLOR_TO_BSRR(C_R2 | C_R1 | C_B0), // rose
     COLOR_TO_BSRR(C_MAGENTA),
     COLOR_TO_BSRR(C_R0 | C_B1 | C_B2), // violet
     COLOR_TO_BSRR(C_BLUE),
     COLOR_TO_BSRR(C_B2 | C_B1 | C_G0), // azure
     COLOR_TO_BSRR(C_CYAN),
     COLOR_TO_BSRR(C_B0 | C_G1 | C_G2), // spring green
     COLOR_TO_BSRR(C_GREEN),
     COLOR_TO_BSRR(C_G2 | C_G1 | C_R0), // chartreuse green
     COLOR_TO_BSRR(C_YELLOW),
     COLOR_TO_BSRR(C_R2 | C_R1 | C_G0) // orange
    };
int current_color = 0;          // start red

// These interrupt service routines control hsync, vsync, and sending
// the visible part of each scan line to the monitor.
void isr_porch(void);
void isr_start(void);
void isr_update(void);

// This function is called during scan lines 480--489; these lines
// aren't visible in the display, and so it's a good time to compute
// what the next frame should be.
void update_frame(void);

//------------------------------ setup()/loop() -------------------------------

void setup() {
    // Set up our color output pins
    for (int i = 0; i < N_COLOR_BITS; i++) {
        pinMode(color_pins[i], OUTPUT);
        // we can use digitalWrite here, because we aren't driving the
        // display yet, and this only gets run once.  We use the (much
        // faster) VGA_SET_BSRR from within the inner loop of each
        // horizontal scan line.
        digitalWrite(color_pins[i], LOW);
    }

    // Set up hsync/vsync pins.
    pinMode(HS_PIN, OUTPUT);
    pinMode(VS_PIN, OUTPUT);
    digitalWrite(HS_PIN, HIGH);
    digitalWrite(VS_PIN, HIGH);

    // USB and SysTick each fire interrupts, the handling of which
    // causes jitter and other interrupt artifacts in our display.
    // Turning them off lets us get cleaner video.
    SerialUSB.end();
    systick_disable();

    // Fill the frame with black.
    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            frame[y][x] = COLOR_TO_BSRR(C_BLACK);
        }
    }

    // Configure the interrupts which handle a scan line.
    Timer4.pause(); // while we configure
    Timer4.setPrescaleFactor(1);     // Full speed
    Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setChannel2Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setChannel3Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setOverflow(2287);   // Total line time

    Timer4.setCompare1(200);
    Timer4.attachCompare1Interrupt(isr_porch);
    Timer4.setCompare2(START_IMAGE);
    Timer4.attachCompare2Interrupt(isr_start);
    Timer4.setCompare3(1);      // Could be zero I guess
    Timer4.attachCompare3Interrupt(isr_update);

    Timer4.setCount(0);         // Ready...
    Timer4.resume();            // Go!
}

void loop() {
    // Empty.  Everything happens in the interrupts!
}

//--------------------------------- VGA ISRs ----------------------------------

// This ISR will end horizontal sync for most of the image and
// setup the vertical sync for higher line counts
void isr_porch(void) {
    VGA_HS_HIGH;
    y++;
    frame_row = frame[y / (480/FRAME_HEIGHT)];
    // Back to the top
    if(y >= 523) {
        y = 1;
        v_active = 1;
        frame_row = frame[0];
        return;
    }

    // we could also dedicate lines 493--522 to advancing the frame,
    // if we wanted to (and lines 479, 490, 492, and 523 could also be
    // used, since we don't really do anything during those here).
    //
    // don't worry about it for now.

    // Other vsync stuff below the image
    if(y >= 492) {
        VGA_VS_HIGH;
        return;
    }
    if(y >= 490) {
        VGA_VS_LOW;
        return;
    }
    if(y == 479) {
        v_active = 0;           // stop drawing
        return;
    }

    // dedicate lines 480--489 to thinking about the next frame, one
    // line at a time
    if(y >= 480) {
        update_frame();
        return;
    }
}

// This is the main horizontal sweep
void isr_start(void) {
    // Skip if we're not in the image at all
    if(!v_active) { return; }

    // main loop: draw the contents of the frame as fast as we can.
    // use VGA_BIT so per-iteration time is deterministic.
    for(x = 0; x < FRAME_WIDTH; x++) {
        VGA_SET_BSRR(frame_row[x]);
    }

    // delay a bit so the last pixel in the row doesn't look so narrow
    volatile_int++;

    // black out what's left, or vsync won't work
    VGA_SET_BSRR(C_BLACK);
}

// Start horizonal sync
void isr_update(void) {
    VGA_HS_LOW;
}

//------------------------------- frame update --------------------------------

void update_frame(void) {       // TODO
    // right now, we've got lines 480--489 to do our thinking.
    switch (y) {
    case 480:
        break;
    case 481:
        break;
    case 482:
        break;
    case 483:
        break;
    case 484:
        break;
    case 485:
        break;
    case 486:
        break;
    case 487:
        break;
    case 488:
        break;
    case 489:
        break;
    default:
        ASSERT(0);
        break;
    }
}

//---------------------------------- main() -----------------------------------

int main(void) {
    init();
    setup();

    while (1) {
        loop();
    }

    return 0;
}
