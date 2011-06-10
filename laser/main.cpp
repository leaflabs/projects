/*
  Laser Scanning Demo

*/

#include "wirish.h"
#include "util.h"

typedef struct {
    int16 x;
    int16 y;
} Point;

// num points 875
Point p_array[875] = {{1,1},
{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},{1,10},{1,11},
{1,12},{1,13},{1,14},{1,15},{1,16},{1,17},{1,18},{1,19},{1,20},{1,21},
{1,22},{1,23},{2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},
{2,9},{2,10},{2,11},{2,12},{2,13},{2,14},{2,15},{2,16},{2,17},{2,18},
{2,19},{2,20},{2,21},{2,22},{2,23},{3,1},{3,2},{3,3},{3,4},{3,5},
{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8},{4,9},{5,6},
{5,7},{5,8},{5,9},{5,10},{5,11},{5,12},{6,9},{6,10},{6,11},{6,12},
{6,13},{6,14},{6,15},{6,16},{7,13},{7,14},{7,15},{7,16},{7,17},{7,18},
{7,19},{8,16},{8,17},{8,18},{8,19},{9,13},{9,14},{9,15},{9,16},{9,17},
{9,18},{9,19},{10,9},{10,10},{10,11},{10,12},{10,13},{10,14},{10,15},{10,16},
{11,6},{11,7},{11,8},{11,9},{11,10},{11,11},{11,12},{12,1},{12,2},{12,3},
{12,4},{12,5},{12,6},{12,7},{12,8},{12,9},{13,1},{13,2},{13,3},{13,4},
{13,5},{14,1},{14,2},{14,3},{14,4},{14,5},{14,6},{14,7},{14,8},{14,9},
{14,10},{14,11},{14,12},{14,13},{14,14},{14,15},{14,16},{14,17},{14,18},{14,19},
{14,20},{14,21},{14,22},{14,23},{15,1},{15,2},{15,3},{15,4},{15,5},{15,6},
{15,7},{15,8},{15,9},{15,10},{15,11},{15,12},{15,13},{15,14},{15,15},{15,16},
{15,17},{15,18},{15,19},{15,20},{15,21},{15,22},{15,23},{19,16},{19,17},{19,18},
{19,19},{19,20},{20,9},{20,10},{20,14},{20,15},{20,16},{20,17},{20,18},{20,19},
{20,20},{20,21},{20,22},{21,7},{21,8},{21,9},{21,13},{21,14},{21,15},{21,16},
{21,20},{21,21},{21,22},{21,23},{22,7},{22,8},{22,9},{22,13},{22,14},{22,15},
{22,21},{22,22},{22,23},{22,40},{22,41},{22,42},{22,43},{22,44},{22,45},{22,46},
{22,47},{22,48},{22,49},{22,50},{22,51},{22,52},{22,53},{22,54},{22,55},{22,56},
{22,57},{22,58},{22,59},{22,60},{22,61},{22,62},{23,7},{23,8},{23,9},{23,13},
{23,14},{23,15},{23,21},{23,22},{23,23},{23,40},{23,41},{23,42},{23,43},{23,44},
{23,45},{23,46},{23,47},{23,48},{23,49},{23,50},{23,51},{23,52},{23,53},{23,54},
{23,55},{23,56},{23,57},{23,58},{23,59},{23,60},{23,61},{23,62},{24,7},{24,8},
{24,9},{24,13},{24,14},{24,15},{24,21},{24,22},{24,23},{24,40},{24,41},{24,42},
{24,43},{25,7},{25,8},{25,9},{25,13},{25,14},{25,15},{25,20},{25,21},{25,22},
{25,40},{25,41},{25,42},{25,43},{25,44},{25,45},{25,46},{25,47},{26,7},{26,8},
{26,9},{26,10},{26,13},{26,14},{26,15},{26,18},{26,19},{26,20},{26,21},{26,22},
{26,44},{26,45},{26,46},{26,47},{26,48},{26,49},{26,50},{27,9},{27,10},{27,11},
{27,12},{27,13},{27,14},{27,15},{27,16},{27,17},{27,18},{27,19},{27,20},{27,21},
{27,22},{27,23},{27,47},{27,48},{27,49},{27,50},{27,51},{27,52},{27,53},{27,54},
{28,10},{28,11},{28,12},{28,13},{28,14},{28,15},{28,16},{28,17},{28,18},{28,19},
{28,20},{28,21},{28,22},{28,23},{28,51},{28,52},{28,53},{28,54},{28,55},{28,56},
{28,57},{29,54},{29,55},{29,56},{29,57},{30,51},{30,52},{30,53},{30,54},{30,55},
{30,56},{30,57},{31,47},{31,48},{31,49},{31,50},{31,51},{31,52},{31,53},{31,54},
{32,44},{32,45},{32,46},{32,47},{32,48},{32,49},{32,50},{33,7},{33,8},{33,9},
{33,10},{33,11},{33,12},{33,13},{33,14},{33,15},{33,16},{33,17},{33,18},{33,19},
{33,20},{33,21},{33,22},{33,23},{33,24},{33,25},{33,26},{33,27},{33,28},{33,29},
{33,30},{33,40},{33,41},{33,42},{33,43},{33,44},{33,45},{33,46},{33,47},{34,7},
{34,8},{34,9},{34,10},{34,11},{34,12},{34,13},{34,14},{34,15},{34,16},{34,17},
{34,18},{34,19},{34,20},{34,21},{34,22},{34,23},{34,24},{34,25},{34,26},{34,27},
{34,28},{34,29},{34,30},{34,40},{34,41},{34,42},{34,43},{35,9},{35,10},{35,11},
{35,12},{35,18},{35,19},{35,20},{35,21},{35,22},{35,40},{35,41},{35,42},{35,43},
{35,44},{35,45},{35,46},{35,47},{35,48},{35,49},{35,50},{35,51},{35,52},{35,53},
{35,54},{35,55},{35,56},{35,57},{35,58},{35,59},{35,60},{35,61},{35,62},{36,9},
{36,10},{36,20},{36,21},{36,22},{36,40},{36,41},{36,42},{36,43},{36,44},{36,45},
{36,46},{36,47},{36,48},{36,49},{36,50},{36,51},{36,52},{36,53},{36,54},{36,55},
{36,56},{36,57},{36,58},{36,59},{36,60},{36,61},{36,62},{37,7},{37,8},{37,9},
{37,21},{37,22},{37,23},{38,7},{38,8},{38,9},{38,21},{38,22},{38,23},{39,7},
{39,8},{39,9},{39,21},{39,22},{39,23},{40,7},{40,8},{40,9},{40,10},{40,20},
{40,21},{40,22},{40,23},{41,9},{41,10},{41,11},{41,12},{41,18},{41,19},{41,20},
{41,21},{41,22},{41,38},{41,39},{41,40},{41,41},{41,42},{41,46},{41,47},{41,48},
{41,49},{41,50},{41,51},{41,52},{41,53},{41,54},{41,55},{41,56},{41,57},{41,58},
{41,59},{41,60},{41,61},{41,62},{42,10},{42,11},{42,12},{42,13},{42,14},{42,15},
{42,16},{42,17},{42,18},{42,19},{42,20},{42,38},{42,39},{42,40},{42,41},{42,42},
{42,46},{42,47},{42,48},{42,49},{42,50},{42,51},{42,52},{42,53},{42,54},{42,55},
{42,56},{42,57},{42,58},{42,59},{42,60},{42,61},{42,62},{43,11},{43,12},{43,13},
{43,14},{43,15},{43,16},{43,17},{43,18},{43,19},{47,0},{47,1},{47,2},{47,3},
{47,4},{47,5},{47,6},{47,7},{47,8},{47,9},{47,10},{47,11},{47,12},{47,13},
{47,14},{47,15},{47,16},{47,17},{47,18},{47,19},{47,20},{47,21},{47,22},{47,23},
{47,46},{47,47},{47,48},{47,49},{47,50},{47,51},{47,52},{47,53},{47,54},{47,55},
{47,56},{47,57},{47,58},{47,59},{47,60},{47,61},{47,62},{48,0},{48,1},{48,2},
{48,3},{48,4},{48,5},{48,6},{48,7},{48,8},{48,9},{48,10},{48,11},{48,12},
{48,13},{48,14},{48,15},{48,16},{48,17},{48,18},{48,19},{48,20},{48,21},{48,22},
{48,23},{48,46},{48,47},{48,48},{48,49},{48,50},{48,51},{48,52},{48,53},{48,54},
{48,55},{48,56},{48,57},{48,58},{48,59},{48,60},{48,61},{48,62},{49,47},{49,48},
{49,49},{50,47},{51,46},{51,47},{52,11},{52,12},{52,13},{52,14},{52,15},{52,16},
{52,17},{52,18},{52,19},{52,46},{52,47},{53,10},{53,11},{53,12},{53,13},{53,14},
{53,15},{53,16},{53,17},{53,18},{53,19},{53,20},{53,46},{53,47},{54,9},{54,10},
{54,11},{54,12},{54,14},{54,15},{54,16},{54,18},{54,19},{54,20},{54,21},{54,22},
{54,46},{54,47},{54,48},{54,49},{55,7},{55,8},{55,9},{55,10},{55,14},{55,15},
{55,16},{55,20},{55,21},{55,22},{55,23},{55,47},{55,48},{55,49},{55,50},{55,51},
{55,52},{55,53},{55,54},{55,55},{55,56},{55,57},{55,58},{55,59},{55,60},{55,61},
{55,62},{56,7},{56,8},{56,9},{56,14},{56,15},{56,16},{56,21},{56,22},{56,23},
{56,48},{56,49},{56,50},{56,51},{56,52},{56,53},{56,54},{56,55},{56,56},{56,57},
{56,58},{56,59},{56,60},{56,61},{56,62},{57,7},{57,8},{57,9},{57,14},{57,15},
{57,16},{57,21},{57,22},{57,23},{58,7},{58,8},{58,9},{58,14},{58,15},{58,16},
{58,21},{58,22},{58,23},{59,7},{59,8},{59,9},{59,14},{59,15},{59,16},{59,21},
{59,22},{59,23},{60,9},{60,10},{60,14},{60,15},{60,16},{60,21},{60,22},{60,23},
{61,9},{61,10},{61,11},{61,12},{61,13},{61,14},{61,15},{61,16},{61,21},{61,22},
{61,23},{61,38},{61,39},{61,40},{61,41},{61,42},{61,46},{61,47},{61,48},{61,49},
{61,50},{61,51},{61,52},{61,53},{61,54},{61,55},{61,56},{61,57},{61,58},{61,59},
{61,60},{61,61},{61,62},{62,11},{62,12},{62,13},{62,14},{62,15},{62,16},{62,20},
{62,21},{62,22},{62,38},{62,39},{62,40},{62,41},{62,42},{62,46},{62,47},{62,48},
{62,49},{62,50},{62,51},{62,52},{62,53},{62,54},{62,55},{62,56},{62,57},{62,58},
{62,59},{62,60},{62,61},{62,62}};
/** setup()/loop() ***********************************************************/

#define CTRL_RATE 500    // 10KHz control
#define X_OUT_P 8
#define Y_OUT_P 9
// todo
#define X_OUT_N 10
#define Y_OUT_N 11

#define LASER 5
uint16 tri(uint16 t) {
    /* ranges from 0-512 */
    t = t % 512;
    if (t < 256) {
        return t;
    } else {
        return 512 - t;
    }
}

Point p = {0,0};

void draw_point(void) {
    //    ASSERT(p.x < 256 && p.x > -256);
    //    ASSERT(p.y < 256 && p.y > -256);

    // for now just draw the positive part
    if (p.x <= 128) {
        pwmWrite(X_OUT_P,0);
        pwmWrite(X_OUT_N,128 - p.x);
    } else {
        pwmWrite(X_OUT_P,p.x - 128);
        pwmWrite(X_OUT_N,0);
    }
    if (p.y <= 128) {
        pwmWrite(Y_OUT_P,0);
        pwmWrite(Y_OUT_N,128 - p.y);
    } else {
        pwmWrite(Y_OUT_P,p.y - 128);
        pwmWrite(Y_OUT_N,0);
    }
}

void square(uint16 t) {
    t = t%1024;
    if (t < 256) {
        p.x = t;
        p.y = 0;
    } else if (t >= 256 && t < 512) {
        p.x = 256;
        p.y = t - 256;
    } else if (t >= 512 && t < 768) {
        p.x = 768-t;
        p.y = 256;
    } else {
        p.x = 0;
        p.y = 1024-t;
    }
}

void raster(uint16 t) {
    p.x = t % 256;
    p.y = (t/256) % 256;
}

void random(void) {
    p.x = random(100,150);
    p.y = random(100,150);
}

void random_im(void) {
    int pi = random(0,875);
    Point tp = p_array[pi];
    p.x = 4*(64 - tp.y);// + 64;
    p.y = 4*(64 - tp.x);// + 64;
}

void bound_point(Point tl, Point br) {
    p.x = min(p.x, br.x);
    p.x = max(tl.x,p.x);
    p.y = min(p.y, br.y);
    p.y = max(tl.y,p.y);
}

void random_walk(void) {
    p.x += random(-1,2);
    p.y += random(-1,2);

    if (p.x > 255 || p.x < 0) { p.x = random(0,255);}
    if (p.y > 255 || p.y < 0) { p.y = random(0,255);}
}

void order_walk(uint16 t) {
    t = t%875;
    Point tp = p_array[t];
    p.x = 4*(64 - tp.y);
    p.y = 4*(64 - tp.x);
}

void handler_ctrl(void) {
    // runs at 1KHz
    static uint16 t = 0;
    t++;
    // if (t < 512) {
    //     pwmWrite(X_OUT_P,tri(t));
    // } else {
    //     pwmWrite(Y_OUT_P,tri(t));
    // }

    //square(t);
    random_walk();
    static uint16 t2 = 0;
    if ((t%32 == 0)) {
        t = 0;
        t2++;;
        random_im();
     }
    draw_point();

    //    pwmWrite(LASER,t/256);
}

void setup(void) {
    // setup pwm and control timers
    pinMode(X_OUT_P,PWM);
    pinMode(Y_OUT_P,PWM);
    pinMode(X_OUT_N,PWM);
    pinMode(Y_OUT_N,PWM);
    pinMode(LASER,PWM);

    pwmWrite(LASER,255); // just leave it on for now.

    // pins 3 and 4 are on timer 3, can there be an automated way to find this
    Timer2.setOverflow(0x80); // 8 bit PWM
    Timer2.setPrescaleFactor(1); // timer runs at 72MHz = 282KHz @ 8 bit

    // Setup Laser Timer
    Timer3.setOverflow(0x100);
    Timer3.setPrescaleFactor(1);

    Timer1.setChannel1Mode(TIMER_OUTPUTCOMPARE);
    Timer1.setPeriod(CTRL_RATE);
    Timer1.setCompare1(1);
    Timer1.attachCompare1Interrupt(handler_ctrl);

    pinMode(6,INPUT_ANALOG);
    randomSeed(analogRead(6));
}

void loop(void) {
    SerialUSB.print(p.x);
    SerialUSB.print(",");
    SerialUSB.println(p.y);
    delay(100);
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
