/*
 * Stress test code for the SerialUSB module of wirish
 *
 * Intended to work with serialUSB_test_recv.py
 *
 * This code is released into the public domain.
 */

#include "wirish.h"
#include "usb.h"

void print_sample(int,uint8,int);

void print_sample(int num_in, uint8 fval, int seq,int errors) {
    SerialUSB.print("\t");
    SerialUSB.print(fval);
    SerialUSB.print(",");
    SerialUSB.print(num_in);
    SerialUSB.print(",");
    SerialUSB.print(seq);
    SerialUSB.print(",");
    SerialUSB.println(errors);

}

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
    pinMode(BOARD_BUT_PIN, INPUT);

    waitForButtonPress(0);
    SerialUSB.println("Starting Receive Test");
    SerialUSB.println("Format:");
    SerialUSB.println("\tFirst Byte, Num, Seq. Num, Errors\n\r");
}

void loop() {
    static int seq_num = 1;
    static int errors = 0;
    digitalWrite(BOARD_LED_PIN,LOW);
    
    /* read bytes out of the buffer one at a time */
    if (usbBytesAvailable() > 0) {
        digitalWrite(BOARD_LED_PIN,HIGH);
        int num_recvd  = 0;
        uint8 first_val = 0;
        uint8 tmp_val  = 0;
        while(usbBytesAvailable() > 0) {
            usbReceiveBytes(&tmp_val,1);
            if (num_recvd == 0) {
                first_val = tmp_val;
            }
            num_recvd++;

            if (tmp_val != first_val) {
                errors++;
            }
        }
        print_sample(num_recvd,first_val,seq_num,errors);
        seq_num++;
    }

    /* todo test, read the bytes out all at once */
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
