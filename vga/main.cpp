/*
 * VGA pong, September 2010.
 *
 * Adapted from the CrudeVGA demo by Marti Bolivar
 *
 * This code is released into the public domain.
 */

#include "wirish.h"

//----------------------- VGA defines/types/prototypes ------------------------

// Pinouts -- you also must change the GPIO macros below if you change
// these
#define VGA_R 6     // STM32: A8
#define VGA_G 7     // STM32: A9
#define VGA_B 8     // STM32: A10
#define VGA_V 11    // STM32: A6
#define VGA_H 12    // STM32: A7

// These low level (and STM32 specific) macros make GPIO writes much
// faster
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
// set has priority, so clear every bit and set a given bit:
#define VGA_BIT(b) (*ABSRR = BIT((b)) |                                \
                    BIT(RBIT+16) | BIT(GBIT+16) | BIT(BBIT+16))

#define VGA_V_HIGH *ABSRR = BIT(6)
#define VGA_V_LOW  *ABRR  = BIT(6)
#define VGA_H_HIGH *ABSRR = BIT(7)
#define VGA_H_LOW  *ABRR  = BIT(7)

// when in the 0-2286 horizontal line cycle range we start drawing the board
#define START_IMAGE 400

// controls the board size
#define BOARD_WIDTH 54
#define BOARD_HEIGHT 48

// board color scheme
#define BORDER_COLOR RBIT  // color of the bands around the game board
#define ON_COLOR BBIT      // default "filled" color
#define OFF_COLOR GBIT     // default "empty" color

void isr_porch(void);
void isr_start(void);
void isr_update(void);

// this is (hackishly) used to force delays
volatile uint8 volatile_int = 0;

// vga state
uint16 x = 0;       // X coordinate
uint16 y = 0;       // Y coordinate
uint8 v_active = 1; // Are we in the image?

// each board cell is an OR of RBIT, GBIT, and BBIT
//
// access like board[y][x], where board[0][0] is upper left, and
// board[BOARD_HEIGHT-1][BOARD_WIDTH-1] is bottom right
uint8 board[BOARD_HEIGHT][BOARD_WIDTH];
uint8 *board_row;    // cache our current row at hsync time, for speed

//----------------------- Pong defines/types/prototypes -----------------------

// joystick pins
#define LEFT_UP 20
#define LEFT_DOWN 19
#define RIGHT_UP 17
#define RIGHT_DOWN 15

// paddle x positions, in board coordinates
#define LEFT_PADDLE_X 0
#define RIGHT_PADDLE_X (BOARD_WIDTH-1)
// controls how fast paddles move
#define PADDLE_OVERFLOW 6
// how big, in board pixels, to draw a paddle
#define PADDLE_HEIGHT 7
// controls how fast the ball moves
#define BALL_OVERFLOW 3
// ball color when a goal is scored
#define GOAL_SCORED_COLOR RBIT

// joystick commands -- we only read up and down
typedef enum {
    C_UP, C_DOWN, C_NONE
} command_t;

// paddle data structure
typedef struct _paddle {
    int8 x;                     // board x-coord
    int8 y;                     // board y-coord
    command_t cmd;              // last joystick command
    uint8 count;                // # repeats of last joystick command
    uint8 up_pin;
    uint8 down_pin;
} paddle_t;

// ball data structure
typedef struct _ball {
    int8 x;
    int8 y;
    int8 vx;
    int8 vy;
    int8 count;
} ball_t;

// board update functions
void advance_state();
void update_paddle(paddle_t*);
void clear_paddle(paddle_t*);
void draw_paddle(paddle_t*);
void update_ball(ball_t*);
void clear_ball(ball_t*);
void draw_ball(ball_t*);
boolean is_blocking(int8, int8, int8, int8);

// board state
paddle_t left_paddle = { /* .x = */ LEFT_PADDLE_X, /* .y = */ 6,
                         /* .cmd = */ C_NONE, /* .count = */ 0,
                         /* .up_pin = */ LEFT_UP, /* down_pin = */ LEFT_DOWN };
paddle_t right_paddle = { /* .x = */ RIGHT_PADDLE_X, /* .y = */ 6,
                          /* .cmd = */ C_NONE, /* .count = */ 0,
                          /* .up_pin = */RIGHT_UP,/* down_pin = */ RIGHT_DOWN};
ball_t ball = { /* .x = */ BOARD_WIDTH / 2, /* .y = */ BOARD_HEIGHT / 2,
                /* .vx = */1, /* .vy = */ 1,
                /* .count = */ 0 };

//------------------------------ setup()/loop() -------------------------------

// true board cells become ON_COLOR, false become OFF_COLOR
void setup_board() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x]) {
                board[y][x] = ON_COLOR;
            } else {
                board[y][x] = OFF_COLOR;
            }
        }
    }
}

void setup() {
    // Setup our pins
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

    // This gets rid of interrupt artifacts
    SerialUSB.end();
    systick_disable();

    // convert the board from binary into [RGB]BIT values
    setup_board();

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
    Timer4.setCompare3(1);      // Could be zero I guess
    Timer4.attachCompare3Interrupt(isr_update);

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
    board_row = board[y / (480/BOARD_HEIGHT)];
    // Back to the top
    if(y >= 523) {
        y = 1;
        v_active = 1;
        board_row = board[0];
        return;
    }
    // Other vsync stuff below the image
    if(y >= 492) {
        VGA_V_HIGH;
        return;
    }
    if(y >= 490) {
        VGA_V_LOW;
        return;
    }
    if(y == 479) {
        v_active = 0;           // stop drawing
        return;
    }
    // give lines 480--489 to game logic
    if(y >= 480) {
        advance_state();
        return;
    }
}

// This is the main horizontal sweep
void isr_start(void) {
    // Skip if we're not in the image at all
    if(!v_active) { return; }

    // Start with the border.  Overhead setting up the main loop is
    // going to make this appear as a wide band.
    VGA_BIT(BORDER_COLOR);

    // main loop: draw the contents of the board as fast as we can.
    // use VGA_BIT so per-iteration time is deterministic.
    for(x=0; x < BOARD_WIDTH; x++) {
        VGA_BIT(board_row[x]);
    }

    // delay just a little so the last board pixel doesn't look so narrow
    volatile_int++;

    // add another band to mark off the edge
    VGA_BIT(BORDER_COLOR);
    volatile_int++;
    volatile_int++;
    volatile_int++;
    volatile_int++;
    volatile_int++;
    volatile_int++;

    // black out what's left, or vsync won't work
    VGA_BLACK;
}

// Setup horizonal sync
void isr_update(void) {
    VGA_H_LOW;
}

//-------------------------------- Game logic ---------------------------------

// the interrupt handlers will give lines 480--489 to this function,
// which must finish its work before isr_stop on each line (which is
// why we only update one thing (ball, paddle) at a time
void advance_state() {
    switch (y) {
    case 480:
        update_ball(&ball);
        break;
    case 481:
        update_paddle(&left_paddle);
        break;
    case 482:
        update_paddle(&right_paddle);
        break;
    default:
        break;
    }
    return;
}

void update_ball(ball_t *ball) {
    clear_ball(ball);
    (ball->count)++;

    if (ball->count >= BALL_OVERFLOW) {
        ball->count = 0;
        int8 next_x = ball->x + ball->vx, next_y = ball->y + ball->vy;
        boolean blocked_left = is_blocking(left_paddle.x, left_paddle.y,
                                           next_x, next_y);
        boolean blocked_right = is_blocking(right_paddle.x, right_paddle.y,
                                            next_x, next_y);

        // update x-coord
        if (ball->vx > 0) {
            if (blocked_right) {
                ball->x = right_paddle.x - 1;
                ball->vx *= -1;
            } else if (next_x < BOARD_WIDTH) {
                ball->x = next_x;
            } else {            // GOL! GOOOL GOOOL GOOOOOL!
                ball->x = BOARD_WIDTH-1;
                ball->vx *= -1;
                // make the ball stay where it is for a few overflow
                // periods so you can tell a goal was scored
                ball->count = - 4 * BALL_OVERFLOW;
            }
        } else {
            if (blocked_left) {
                ball->x = left_paddle.x + 1;
                ball->vx *= -1;
            } else if (next_x > 0) {
                ball->x = next_x;
            } else {            // GOL! GOOOL GOOOL GOOOOOL!
                ball->x = 0;
                ball->vx *= -1;
                ball->count = - 4 * BALL_OVERFLOW;
            }
        }

        // update y-coord
        if (!(blocked_left || blocked_right)) {
            if (ball->vy > 0) {
                if (next_y < BOARD_HEIGHT) {
                    ball->y = next_y;
                } else {
                    ball->y = BOARD_HEIGHT-1;
                    ball->vy *= -1;
                }
            } else {
                if (next_y > 0) {
                    ball->y = next_y;
                } else {
                    ball->y = 0;
                    ball->vy *= -1;
                }
            }
        }
    }

    draw_ball(ball);
}

inline void clear_ball(ball_t *ball) {
    board[ball->y][ball->x] = OFF_COLOR;
}

inline void draw_ball(ball_t *ball) {
    if (ball->x == 0 || ball->x == BOARD_WIDTH - 1) {
        board[ball->y][ball->x] = GOAL_SCORED_COLOR;
    } else {
        board[ball->y][ball->x] = ON_COLOR;
    }
}

inline boolean is_blocking(int8 paddle_x, int8 paddle_y, int8 x, int8 y) {
    return x == paddle_x && paddle_y <= y && y < paddle_y + PADDLE_HEIGHT;
}

void update_paddle(paddle_t *paddle) {
    clear_paddle(paddle);

    if (digitalRead(paddle->up_pin)) {
        if (paddle->cmd == C_UP) {
            paddle->count++;
        } else {
            paddle->cmd = C_UP;
            paddle->count = 1;
        }
    } else if (digitalRead(paddle->down_pin)) {
        if (paddle->cmd == C_DOWN) {
            paddle->count++;
        } else {
            paddle->cmd = C_DOWN;
            paddle->count = 1;
        }
    } else {
        paddle->cmd = C_NONE;      // so that old commands don't stack up
        paddle->count = 0;
    }

    if (paddle->count == PADDLE_OVERFLOW) {
        paddle->count = 0;
        if (paddle->cmd == C_UP && paddle->y > 0) {
            paddle->y--;           // board y-axis grows down
        } else if (paddle->cmd == C_DOWN &&
                   paddle->y + PADDLE_HEIGHT < BOARD_HEIGHT) {
            paddle->y++;
        }
    }

    draw_paddle(paddle);
}

void clear_paddle(paddle_t *paddle) {
    for (int y = 0; y < PADDLE_HEIGHT; y++) {
        board[paddle->y + y][paddle->x] = OFF_COLOR;
    }
}

void draw_paddle(paddle_t *paddle) {
    for (int y = 0; y < PADDLE_HEIGHT; y++) {
        board[paddle->y + y][paddle->x] = ON_COLOR;
    }
}

//---------------------------------- main() -----------------------------------

int main(void) {
    init();
    setup();

    while (1) {
        // no loop -- everything happens in the interrupts
    }

    return 0;
}
