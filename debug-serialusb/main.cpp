/*
 * Stress test code for the SerialUSB module of wirish
 *
 * Intended to work with serialUSB_test.py
 *
 *
 * tests should include:
 *    - usbSendBytes    - not dropping packets at any speed/size
 *    - usbReceiveBytes - not dropping packets at any speed/size
 *
 *    - SerialUSB.print  - blocking write, always works
 *    - SerialUSB.read() - blocking read, returns one byte in sequence and drops none
 *
 * This code is released into the public domain.
 */

#include "wirish.h"
#include "usb.h"

#define ERROR_CHAR ('*')
#define CLEAR_CHAR ('_')

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);
}

uint8 choose_fill(int len) {
    switch (len) {
    case 1:
        return'a';
    case 2:
        return'b';
    case 4:
        return'c';
    case 8:
        return'd';
    case 16:
        return'e';
    case 32:
        return'f';
    case 64:
        return'g';
    case 128:
        return'h';
    default:
        return 'X';
    }
}

void fill_buf(uint8 *buf, int len, uint8 c) {
    for (int i = 0; i < len; i++) buf[i] = c;
}

void send_buf(uint8 *buf, int len) {
    uint8 sent = 0;
    while (sent < len) {
        int n = usbSendBytes(buf + sent, len - sent);
        sent += n;
    }
}

void recv_buf(uint8 *buf, int len) {
    uint8 recvd = 0;
    uint8 expected = choose_fill(len);
    while (recvd < len) {
        int n = usbReceiveBytes(buf + recvd, len - recvd);
        for (int i = 0; i < n; i++) {
            if (buf[recvd + i] != expected) buf[recvd + i] = ERROR_CHAR;
        }
        recvd += n;
        toggleLED();
    }
}

void loop() {
    static uint8 buf[128];
    static int len = 1;
    uint8 fill = choose_fill(len);

    // let the host know what we think is about to happen
    waitForButtonPress(0);
    SerialUSB.print("size = ");
    SerialUSB.print(len);
    SerialUSB.print(", fill = ");
    SerialUSB.println(fill);

    // clear out last run, then fill with correct char for current len
    fill_buf(buf, len, fill);

    // round-trip the buffer once, marking places where received
    // wasn't expected
    send_buf(buf, len);
    recv_buf(buf, len);

    // send it again, with errors marked
    send_buf(buf, len);

    len *= 2;
    if (len > 128) len = 1;
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
