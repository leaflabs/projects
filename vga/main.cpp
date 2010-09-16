
#include "wirish.h"

#define LED_PIN 13

// Pinouts
#define VGA_R 5     // STM32: B6
#define VGA_G 6     // STM32: A8
#define VGA_B 7     // STM32: A9
#define VGA_V 11    // STM32: A6
#define VGA_H 12    // STM32: A7

// These low level macros make GPIO writes much faster
#define VGA_R_HIGH (GPIOB_BASE)->BSRR = BIT(6)
#define VGA_R_LOW  (GPIOB_BASE)->BRR  = BIT(6)
#define VGA_G_HIGH (GPIOA_BASE)->BSRR = BIT(8)
#define VGA_G_LOW  (GPIOA_BASE)->BRR  = BIT(8)
#define VGA_B_HIGH (GPIOA_BASE)->BSRR = BIT(9)
#define VGA_B_LOW  (GPIOA_BASE)->BRR  = BIT(9)
#define VGA_V_HIGH (GPIOA_BASE)->BSRR = BIT(6)
#define VGA_V_LOW  (GPIOA_BASE)->BRR  = BIT(6)
#define VGA_H_HIGH (GPIOA_BASE)->BSRR = BIT(7)
#define VGA_H_LOW  (GPIOA_BASE)->BRR  = BIT(7)

// pick one of these for when isr_start gets triggered, they'll place
// the image appropriately
#define START_IMAGE_FLUSH_LEFT  300
#define START_IMAGE_CENTERED    600
// we'll center
#define START_IMAGE START_IMAGE_CENTERED

// joystick pins
#define LEFT_UP 20
#define LEFT_DOWN 19
#define RIGHT_UP 18
#define RIGHT_DOWN 17

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

typedef enum {
    C_UP, C_DOWN, C_NONE
} command_t;

// vga functions
void isr_porch(void);
void isr_start(void);
void isr_stop(void);
void isr_update(void);

// board update functions
void clear_paddle(uint8);
void draw_paddle(uint8);
void clear_ball();
void draw_ball();

// vga state
uint8 toggle;
uint16 x = 0;       // X coordinate
uint16 y = 0;       // Y coordinate
uint8 v_active = 1; // Are we in the image?

// board state
uint16 pad_left = 6;          // board y-coord of left paddle bottom
uint16 pad_right = 6;         // likewise
command_t left_cmd = C_NONE;  // last left command
command_t right_cmd = C_NONE; // last right command
uint8 pad_left_count = 0;     // number of times had last left command
uint8 pad_right_count = 0;     // likewise
uint8 ball_x = 8, ball_y = 8;
uint8 ball_vx = 1, ball_vy = 1;
uint8 ball_count = 0;

// 1-bit! access like board[y][x], where board[0][0] is upper left, and
// board[17][15] is bottom right
uint8 board[18][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}, };

#define BOARD_WIDTH 16
#define BOARD_HEIGHT 18

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

void loop() {
    toggle ^= 1;
    digitalWrite(LED_PIN, toggle);
    delay(100);

    // Everything happens in the interrupts!
}

// This function gets called at the end of a frame (i.e., after line
// 479).  It's got until line 490 (i.e. 480--489 = 10 lines = 22870
// cycles) to do its business, or weird timing bugs will manifest
void advance_state() {
    clear_ball();
    ball_count++;
    if (ball_count == BALL_OVERFLOW) {
        ball_count = 0;
        if (ball_vx > 0) {
            if (ball_x + ball_vx < BOARD_WIDTH) {
                ball_x += ball_vx;
            } else {
                ball_x = BOARD_WIDTH-1;
                ball_vx *= -1;
            }
        } else {
            if (ball_x + ball_vx > 0) {
                ball_x += ball_vx;
            } else {
                ball_x = 0;
                ball_vx *= -1;
            }
        }
        if (ball_vy > 0) {
            if (ball_y + ball_vy < BOARD_HEIGHT) {
                ball_y += ball_vy;
            } else {
                ball_y = BOARD_HEIGHT-1;
                ball_vy *= -1;
            }
        } else {
            if (ball_y + ball_vy > 0) {
                ball_y += ball_vy;
            } else {
                ball_y = 0;
                ball_vy *= -1;
            }
        }
    }
    draw_ball();

    clear_paddle(LEFT_PADDLE);
    clear_paddle(RIGHT_PADDLE);

    if (digitalRead(LEFT_UP)) {
        if (left_cmd == C_UP) pad_left_count++;
        else pad_left_count = 1;
        left_cmd = C_UP;
    } else if (digitalRead(LEFT_DOWN)) {
        if (left_cmd == C_DOWN) pad_left_count++;
        else pad_left_count = 1;
        left_cmd = C_DOWN;
    } else {
        left_cmd = C_NONE;      // so that old commands don't stack up
        pad_left_count = 0;
    }

    if (pad_left_count == PADDLE_OVERFLOW) {
        pad_left_count = 0;
        if (left_cmd == C_UP && pad_left > 0)
            pad_left--;
        else if (left_cmd == C_DOWN && pad_left + PADDLE_HEIGHT < BOARD_HEIGHT)
            pad_left++;
    }

    if (digitalRead(RIGHT_UP)) {
        if (right_cmd == C_UP) pad_right_count++;
        else pad_right_count = 1;
        right_cmd = C_UP;
    } else if (digitalRead(RIGHT_DOWN)) {
        if (right_cmd == C_DOWN) pad_right_count++;
        else pad_right_count = 1;
        right_cmd = C_DOWN;
    } else {
        right_cmd = C_NONE;
        pad_right_count = 0;
    }

    if (pad_right_count == PADDLE_OVERFLOW) {
        pad_right_count = 0;
        if (right_cmd == C_UP && pad_right > 0)
            pad_right--;
        else if (right_cmd == C_DOWN && pad_right + PADDLE_HEIGHT < BOARD_HEIGHT)
            pad_right++;
    }

    draw_paddle(LEFT_PADDLE);
    draw_paddle(RIGHT_PADDLE);
}

void clear_ball() {
    board[ball_y][ball_x] = 0;
}

void draw_ball() {
    board[ball_y][ball_x] = 1;
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

// This ISR will end horizontal sync for most of the image and
// setup the vertical sync for higher line counts
void isr_porch(void) {
    VGA_H_HIGH;
    y++;

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
        // tell everyone to stop drawing
        v_active = 0;
        return;
    }
    if(y==480) {
        // give the rest of the time to game logic
        advance_state();
        return;
    }
}

// This is the main horizontal sweep
void isr_start(void) {  
    // Skip if we're not in the image at all
    if(!v_active) { return; }

    // Start Red
    VGA_R_LOW;
    VGA_R_HIGH;

    // For each "pixel" (# of screen pixels depends on how fast we do
    // this) go red or white
    for(x=0; x<32; x++) {
        switch (board[y/28][x/2]) {
        case 0:
            VGA_G_LOW;
            VGA_B_LOW;
            break;
        case 1:
            VGA_G_HIGH;
            VGA_B_HIGH;
            break;
        }
    }
    // black out what's left
    VGA_R_LOW;
    VGA_G_LOW;
    VGA_B_LOW;
}

// End of the horizontal line
void isr_stop(void) {
    if(!v_active) { return; }
    VGA_R_LOW;
    VGA_G_LOW;
    VGA_B_LOW;
}

// Setup horizonal sync
void isr_update(void) {
    VGA_H_LOW;
}

int main(void) {
    init();
    setup();

    while (1) {
        loop();
    }
    return 0;
}
