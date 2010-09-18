/*
 * VGA pong, September 2010.
 *
 * Adapted from the CrudeVGA demo by Marti Bolivar
 *
 * This code is released into the public domain.
 */

#include "wirish.h"

#define LED_PIN 13

// Pinouts -- you also must change the GPIO macros below if you change
// these
#define VGA_R 6     // STM32: A8
#define VGA_G 7     // STM32: A9
#define VGA_B 8     // STM32: A10
#define VGA_V 11    // STM32: A6
#define VGA_H 12    // STM32: A7

// These low level (and non-portable) macros make GPIO writes much faster
#define ABSRR ((volatile uint32*)0x40010810)
#define ABRR  ((volatile uint32*)0x40010814)

#define RBIT 8                  // (see pinouts)
#define GBIT 9
#define BBIT 10

#define VGA_R_HIGH *ABSRR = BIT(RBIT)
#define VGA_R_LOW  *ABRR  = BIT(RBIT)
#define VGA_G_HIGH *ABSRR = BIT(GBIT)
#define VGA_G_LOW  *ABRR  = BIT(GBIT)
#define VGA_B_HIGH *ABSRR = BIT(BBIT)
#define VGA_B_LOW  *ABRR  = BIT(BBIT)

#define VGA_RED    *ABSRR = BIT(RBIT) | BIT(GBIT+16) | BIT(BBIT+16)
#define VGA_GREEN  *ABSRR = BIT(GBIT) | BIT(RBIT+16) | BIT(BBIT+16)
#define VGA_BLUE   *ABSRR = BIT(BBIT) | BIT(RBIT+16) | BIT(GBIT+16)
#define VGA_WHITE  *ABSRR = BIT(RBIT) | BIT(GBIT)    | BIT(BBIT)
#define VGA_BLACK  *ABRR  = BIT(RBIT) | BIT(GBIT)    | BIT(BBIT)
#define VGA_BIT(b) *ABSRR = BIT((b))  | BIT(RBIT+16) | BIT(GBIT+16) | BIT(BBIT+16) // set has priority!

#if 1 // confusing: changing between these has noticeable effects on timing
#define VGA_V_HIGH (GPIOA_BASE)->BSRR = BIT(6)
#define VGA_V_LOW  (GPIOA_BASE)->BRR  = BIT(6)
#define VGA_H_HIGH (GPIOA_BASE)->BSRR = BIT(7)
#define VGA_H_LOW  (GPIOA_BASE)->BRR  = BIT(7)
#else
#define VGA_V_HIGH *ABSRR = BIT(6)
#define VGA_V_LOW  *ABRR  = BIT(6)
#define VGA_H_HIGH *ABSRR = BIT(7)
#define VGA_H_LOW  *ABRR  = BIT(7)
#endif

// pick one of these for when isr_start gets triggered, they'll place
// the image appropriately
#define START_IMAGE_FLUSH_LEFT 300
#define START_IMAGE_CENTERED   600
// we'll center
#define START_IMAGE START_IMAGE_CENTERED

// joystick pins
#define LEFT_UP 20
#define LEFT_DOWN 19
#define RIGHT_UP 17
#define RIGHT_DOWN 16

// paddle IDs
#define LEFT_PADDLE 0
#define RIGHT_PADDLE 1
// paddle x positions, in board coordinates
#define LEFT_PADDLE_X 0
#define RIGHT_PADDLE_X 15
// controls how fast paddles move
#define PADDLE_OVERFLOW 6
// how big, in board pixels, to draw a paddle
#define PADDLE_HEIGHT 4
// controls how fast the ball moves
#define BALL_OVERFLOW 3

// joystick commands -- we only read up and down
typedef enum {
    C_UP, C_DOWN, C_NONE
} command_t;

// vga functions
void isr_porch(void);
void isr_start(void);
void isr_stop(void);
void isr_update(void);

// board update functions
void advance_state();
void update_paddle(uint8, uint8, uint8, uint8*, command_t*, uint8*);
void clear_paddle(uint8);
void draw_paddle(uint8);
void update_ball();
void clear_ball();
void draw_ball();

// vga state
uint8 toggle;
uint16 x = 0;       // X coordinate
uint16 y = 0;       // Y coordinate
uint8 v_active = 1; // Are we in the image?

// board state
uint8 pad_left = 6;          // board y-coord of left paddle bottom
uint8 pad_right = 6;         // likewise
command_t left_cmd = C_NONE;  // last left command
command_t right_cmd = C_NONE; // last right command
uint8 pad_left_count = 0;     // number of times had last left command
uint8 pad_right_count = 0;    // likewise
int8 ball_x = 8, ball_y = 8;
int8 ball_vx = 1, ball_vy = 1;
uint8 ball_count = 0;

// 1-bit!  this _must_ be 1-bit; an optimization depends on it.
//
// access like board[y][x], where board[0][0] is upper left, and
// board[17][15] is bottom right
uint8 board[18][16] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,},
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,},
    {0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,},
    {1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,},
    {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,},
    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,},
    {0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,},
    {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}, };
uint8 *board_row;               // for speed

#define BOARD_WIDTH 16
#define BOARD_HEIGHT 18

//------------------------------ setup()/loop() -------------------------------

void setup() {
    // Setup our pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(VGA_R, OUTPUT);
    pinMode(VGA_G, OUTPUT);
    pinMode(VGA_B, OUTPUT);
    pinMode(VGA_V, OUTPUT);
    pinMode(VGA_H, OUTPUT);
    pinMode(LEFT_UP, INPUT_PULLDOWN);
    pinMode(LEFT_DOWN, INPUT_PULLDOWN);
    pinMode(RIGHT_UP, INPUT_PULLDOWN);
    pinMode(RIGHT_DOWN, INPUT_PULLDOWN);
    digitalWrite(VGA_R, LOW);
    digitalWrite(VGA_G, LOW);
    digitalWrite(VGA_B, LOW);
    digitalWrite(VGA_H, HIGH);
    digitalWrite(VGA_V, HIGH);

    // This gets rid of the majority of the interrupt artifacts
    SerialUSB.end();
    systick_disable();

    // Configure
    Timer4.pause(); // while we configure
    Timer4.setPrescaleFactor(1);     // Full speed
    Timer4.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setChannel2Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setChannel3Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setChannel4Mode(TIMER_OUTPUTCOMPARE);
    Timer4.setOverflow(2287);   // Total line time

    Timer4.setCompare1(200);
    Timer4.attachCompare1Interrupt(isr_porch);
    Timer4.setCompare2(START_IMAGE);
    Timer4.attachCompare2Interrupt(isr_start);
    Timer4.setCompare3(2170);
    Timer4.attachCompare3Interrupt(isr_stop);
    Timer4.setCompare4(1);      // Could be zero I guess
    Timer4.attachCompare4Interrupt(isr_update);
    
    Timer4.setCount(0);         // Ready...
    Timer4.resume();            // Go!
}

// void loop() {} Everything happens in the interrupts!

//--------------------------------- VGA ISRs ----------------------------------

// This ISR will end horizontal sync for most of the image and
// setup the vertical sync for higher line counts
void isr_porch(void) {
    VGA_H_HIGH;
    y++;
    board_row = board[y/28];
    // Back to the top
    if(y>=523) {
        y=1;
        v_active = 1;
        return;
    }
    // Other vsync stuff below the image 
    if(y>=492) {
        VGA_V_HIGH;
        return;
    }
    if(y>=490) {
        VGA_V_LOW;
        return;
    }
    if(y==479) { 
        v_active = 0;           // stop drawing
        return;
    }
    // give lines 480--489 to game logic
    if(y==480) {
        advance_state();
        return;
    }
}

// This is the main horizontal sweep
void isr_start(void) {  
    // Skip if we're not in the image at all
    if(!v_active) { return; }

    // Start red
    VGA_RED;

    for(x=0; x<16; x++) {
        switch(board_row[x]) {
        case 0:
            VGA_GREEN;
            break;
        case 1:
            VGA_BLUE;
            break;
        }
    }

    // black out what's left
    VGA_BLACK;
}

// End of the horizontal line
void isr_stop(void) {
    if(!v_active) { return; }
    VGA_BLACK;
}

// Setup horizonal sync
void isr_update(void) {
    VGA_H_LOW;
}

//-------------------------------- Game logic ---------------------------------

// This function gets called at the end of a frame (i.e., after h-line
// 479).  It's got until h-line 490 (i.e. 480--489 = 10 h-lines = 22870
// cycles) to do its business, or weird timing bugs will manifest
void advance_state() {
#if 0
    update_ball();
#endif
    // update_paddle(LEFT_PADDLE, LEFT_UP, LEFT_DOWN,
    //               &pad_left, &left_cmd, &pad_left_count);
    // update_paddle(RIGHT_PADDLE, RIGHT_UP, RIGHT_DOWN,
    //               &pad_right, &right_cmd, &pad_right_count);
}

inline void update_ball() {
    clear_ball();
    // ball_count++;
    // if (ball_count >= BALL_OVERFLOW) {
    //     ball_count = 0;
    //     if (ball_vx > 0) {
    //         if (ball_x + ball_vx < BOARD_WIDTH) {
    //             ball_x += ball_vx;
    //         } else {
    //             ball_x = BOARD_WIDTH-1;
    //             ball_vx *= -1;
    //         }
    //     } else {
    //         if (ball_x + ball_vx > 0) {
    //             ball_x += ball_vx;
    //         } else {
    //             ball_x = 0;
    //             ball_vx *= -1;
    //         }
    //     }
    //     if (ball_vy > 0) {
    //         if (ball_y + ball_vy < BOARD_HEIGHT) {
    //             ball_y += ball_vy;
    //         } else {
    //             ball_y = BOARD_HEIGHT-1;
    //             ball_vy *= -1;
    //         }
    //     } else {
    //         if (ball_y + ball_vy > 0) {
    //             ball_y += ball_vy;
    //         } else {
    //             ball_y = 0;
    //             ball_vy *= -1;
    //         }
    //     }
    // }
    draw_ball();
}

inline void clear_ball() {
    board[ball_y][ball_x] = 0;
}

inline void draw_ball() {
    board[ball_y][ball_x] = 1;
}

void update_paddle(uint8 paddle, uint8 up_pin, uint8 down_pin,
                   uint8 *pos, command_t *cmd, uint8 *count) {
    clear_paddle(paddle);

    if (digitalRead(up_pin)) {
        if (*cmd == C_UP) {
            (*count)++;
        } else {
            *cmd = C_UP;
            *count = 1;
        }
    } else if (digitalRead(down_pin)) {
        if (*cmd == C_DOWN) {
            (*count)++;
        } else {
            *cmd = C_DOWN;
            *count = 1;
        }
    } else {
        *cmd = C_NONE;      // so that old commands don't stack up
        *count = 0;
    }

    if (*count == PADDLE_OVERFLOW) {
        *count = 0;
        if (*cmd == C_UP && *pos > 0)
            (*pos)--;           // board y-axis grows down
        else if (*cmd == C_DOWN && *pos + PADDLE_HEIGHT < BOARD_HEIGHT)
            (*pos)++;
    }

    draw_paddle(paddle);
}

void clear_paddle(uint8 paddle) {
    uint8 px=0, py=0;           // shutup, compiler
    switch (paddle) {
    case LEFT_PADDLE:
        px = LEFT_PADDLE_X;
        py = pad_left;
        break;
    case RIGHT_PADDLE:
        px = RIGHT_PADDLE_X;
        py = pad_right;
        break;
    }
    for (int y = 0; y < PADDLE_HEIGHT; y++) {
        board[py+y][px] = 0;
    }
}

void draw_paddle(uint8 paddle) {
    uint8 px=0, py=0;           // shutup, compiler
    switch (paddle) {
    case LEFT_PADDLE:
        px = LEFT_PADDLE_X;
        py = pad_left;
        break;
    case RIGHT_PADDLE:
        px = RIGHT_PADDLE_X;
        py = pad_right;
        break;
    }
    for (int y = 0; y < PADDLE_HEIGHT; y++) {
        board[py+y][px] = 1;
    }
}

//---------------------------------- main() -----------------------------------

int main(void) {
    init();
    setup();

    while (1) {
    }

    return 0;
}
