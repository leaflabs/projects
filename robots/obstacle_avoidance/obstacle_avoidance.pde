/*
  Obstacle Avoidance Routine for Mobile Autonomous Robot
  (OAR for MAR)

  by Brian Tovar
  contact betovar@leaflabs.com
  created  21 Jul 2011
  modified 23 Aug 2011
*/

// motor control pins
#define LEFT_DIR 21    // brown
#define LEFT_BRK 22    // black
#define LEFT_PWM 26    // orange
#define RIGHT_DIR 18   // brown
#define RIGHT_BRK 19   // black
#define RIGHT_PWM 25   // orange

// sharp rangefinder (SRF) data pins
#define SRF_CENTER    0  // green 
#define SRF_RIGHT_90  6  // red
#define SRF_RIGHT_45  5  // blue
#define SRF_RIGHT     7  // yellow
#define SRF_LEFT      8  // purple
#define SRF_LEFT_45   4  // orange
#define SRF_LEFT_90   3  // brown

// misc program parameters and global variables
#define NUM_OF_SENSORS 6        // six sensors being used
#define SENSOR_CLOCK_SPEED 72   // new sensor vector every XX Hertz
#define TURN_LIMIT 1000
uint16 leftmotorspeed = 0;   
uint16 rightmotorspeed = 0;
int sensor[NUM_OF_SENSORS+1];   // one inactive sensor not being used

HardwareTimer timer_sensor(TIMER_CH4);

void setup() {
  // initialize motor control pins as outputs
    pinMode(LEFT_PWM,  PWM   );
    pinMode(LEFT_DIR,  OUTPUT);
    pinMode(LEFT_BRK,  OUTPUT);
    pinMode(RIGHT_PWM, PWM   );
    pinMode(RIGHT_DIR, OUTPUT);
    pinMode(RIGHT_BRK, OUTPUT);
    
  // control pins defaulted to forward motion
    digitalWrite(LEFT_BRK,  LOW );
    digitalWrite(LEFT_DIR,  HIGH);
    digitalWrite(RIGHT_BRK, LOW );
    digitalWrite(RIGHT_DIR, HIGH);

  // start running motors at speed
    pwmWrite(LEFT_PWM,   leftmotorspeed);
    pwmWrite(RIGHT_PWM, rightmotorspeed);  

  // setup rangefinders as analog inputs
  //pinMode(SRF_CENTER,   INPUT_ANALOG);
    pinMode(SRF_RIGHT_90, INPUT_ANALOG);
    pinMode(SRF_RIGHT_45, INPUT_ANALOG);
    pinMode(SRF_RIGHT,    INPUT_ANALOG);
    pinMode(SRF_LEFT,     INPUT_ANALOG);
    pinMode(SRF_LEFT_45,  INPUT_ANALOG);
    pinMode(SRF_LEFT_90,  INPUT_ANALOG);
    
    
    timer_setup();
}

void loop() {
//    uint32 i = random(TURN_LIMIT);
//    turn_left(i);
//    turn_right(i);
    debug_rangefinder();
    delay(100);
}

void handler_sensor(void) {
  //sensor[0] = map(analogRead(SRF_CENTER   ), 0, 4095, 0, 16);
    sensor[1] = map(analogRead(SRF_RIGHT_90 ), 0, 4095, 0, 16);
    sensor[2] = map(analogRead(SRF_RIGHT_45 ), 0, 4095, 0, 16);
    sensor[3] = map(analogRead(SRF_RIGHT    ), 0, 4095, 0, 16);
    sensor[4] = map(analogRead(SRF_LEFT     ), 0, 4095, 0, 16);
    sensor[5] = map(analogRead(SRF_LEFT_45  ), 0, 4095, 0, 16);
    sensor[6] = map(analogRead(SRF_LEFT_90  ), 0, 4095, 0, 16);
    timer_sensor.pause();
}

void timer_setup(void) {    
  timer_sensor.pause();
  timer_sensor.setPrescaleFactor(1);
  timer_sensor.setOverflow(1000000/SENSOR_CLOCK_SPEED);
  timer_sensor.setChannel1Mode(TIMER_OUTPUT_COMPARE);
  timer_sensor.setCompare(TIMER_CH4, 1);
  timer_sensor.attachCompare1Interrupt(handler_sensor);
}

void debug_rangefinder(void) {
    timer_sensor.resume();
    for( int i=1; i<=NUM_OF_SENSORS; i++ ) {
      SerialUSB.print(i, DEC);
      SerialUSB.print(": ");
      SerialUSB.print(sensor[i], DEC);
      SerialUSB.print("   ");
    }
    SerialUSB.println();
}
