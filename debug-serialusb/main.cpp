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
#define MAX_TRANS_SIZE 60
#define RECV_TIMEOUT 5000

#define RX_ONLY 0

#define DEBUG_DELAY 500

#ifdef DEBUG_DELAY
#define PAUSE() delay(DELAY_AFTER_RECV_BUF)
#else
#define PAUSE()
#endif

#define DEBUG_USART Serial1

#ifdef DEBUG_USART
#define DEBUG(x) DEBUG_USART.println(x)
#else
#define DEBUG(x)
#endif

void strobe(void);

void setup() {
    pinMode(BOARD_LED_PIN, OUTPUT);

#ifdef DEBUG_USART
    DEBUG_USART.begin(9600);
#endif

    waitForButtonPress(0);
    SerialUSB.println("starting loop");
    DEBUG("starting loop");
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

void send_buf_single(uint8 *buf, int len) {
    for (int i=0;i<len;i++) {
        usbBlockingSendByte((char)buf[i]);
    }
}

void send_buf(uint8 *buf, int len) {
    digitalWrite(13,HIGH);
    uint8 sent = 0;
    while (sent < len) {
        int n = usbSendBytes(buf + sent, len - sent);
        sent += n;
    }
    digitalWrite(13,LOW);
}

void recv_buf(uint8 *buf, uint32 len) {
    uint32 recvd = 0;
    uint8 expected = choose_fill(len);
    uint32 start = millis();
    uint32 now;
    bool timeout = false;

    while (recvd < len) {
        uint32 n = usbReceiveBytes(buf + recvd, len - recvd);
        for (uint32 i = 0; i < n; i++) {
            if (buf[recvd + i] != expected) buf[recvd + i] = ERROR_CHAR;
        }
        recvd += n;

        now = millis();
        if (now - start > RECV_TIMEOUT) {
            timeout = true;
            break;
        }
    }

    if (timeout) {
        DEBUG("number of bytes received:");
        DEBUG(recvd);
        DEBUG("time elapsed:");
        DEBUG(now - start);

        while (1) {
            strobe();
        }
    }
}

void strobe() {
    for (int i = 0; i < 5; i++) {
        toggleLED();
        delay(100);
        toggleLED();
        delay(100);
    }
}

void loop() {
    static uint8 buf[MAX_TRANS_SIZE];
    static uint32 len = 1;
    uint8 fill = choose_fill(len);

#if RX_ONLY
    recv_buf(buf,len);

    /* flush the buffer */
    while (usbBytesAvailable() > 0) {
        strobe();
        SerialUSB.read();
    }

    SerialUSB.print("size = ");
    SerialUSB.print(len);
    SerialUSB.print(", firstchar = ");
    SerialUSB.println((char)buf[0]);

#else
    // handshake
    SerialUSB.print("size = ");
    SerialUSB.print(len);
    // DEBUG("sent size");

    SerialUSB.print(", fill = ");
    SerialUSB.println(fill);
    // DEBUG("sent fill");

    // fill with correct char for current len
    fill_buf(buf, len, fill);
    // DEBUG("filled buf");

    // send buffer
    send_buf_single(buf, len);
    // DEBUG("sent buf byte at a time");

    // read buffer back, marking errors
    recv_buf(buf, len);
    // DEBUG("got buf back");

    // send buffer response
    send_buf_single(buf, len);
    // DEBUG("sent buf back with errors marked");

    // DEBUG("");
#endif

    len *=2;
    if (len > MAX_TRANS_SIZE) len = 1;

    // waitForButtonPress(0);
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
