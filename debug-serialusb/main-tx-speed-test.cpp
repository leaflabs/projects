/*
 * Stress test code for SerialUSB module of wirish
 * Intended to work with serial_tx_test.py
 * Tests the TX (maple -> host) only. 
 */

#include "wirish.h"
#include "usb.h"

#define BOARD_LED_PIN 13
#define BOARD_BUT_PIN 38

#define MAX_SEND_LEN  65
#define TOTAL_PACKETS 32264

void test_libmaple_tx(void) {
    digitalWrite(BOARD_LED_PIN,HIGH);

    uint8 sendBuf[MAX_SEND_LEN];
    
    // setup the packet
    for (int i=0;i<MAX_SEND_LEN;i++) {
        sendBuf[i] = (i % 52) + 64;
    }

    for (int j=0;j<TOTAL_PACKETS;j++) {
        int bytes_sent = 0;
        while (bytes_sent < MAX_SEND_LEN) {
            bytes_sent += usbSendBytes(&sendBuf[bytes_sent],MAX_SEND_LEN-bytes_sent);
        }
    }


    digitalWrite(BOARD_LED_PIN,LOW);
}

void setup() {
    while (!digitalRead(BOARD_BUT_PIN)); // wait for press

    test_libmaple_tx();
}

void loop() {
    /* indicates successful test completed */
    digitalWrite(BOARD_LED_PIN,HIGH);
    delay(100);
    digitalWrite(BOARD_LED_PIN,LOW);
    delay(100);

    if (digitalRead(BOARD_BUT_PIN)) {
        test_libmaple_tx();
    }
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
