/*
 * Stress test code for SerialUSB module of wirish
 * Intended to work with serial_tx_test.py
 * Tests the TX (maple -> host) only. 
 */

#include "wirish.h"
#include "usb.h"

#define BOARD_LED_PIN 13
#define BOARD_BUT_PIN 38

#define MAX_SEND_LEN  2048

/* simple sketch will send 
   MAX_SEND_LEN byte packets
   and then wait for an ACK,
   and then send it again
*/

uint8 sendBuf[MAX_SEND_LEN];
void tx_packet(void) {

    digitalWrite(BOARD_LED_PIN,HIGH);

    int bytes_sent = 0;
    while (bytes_sent < MAX_SEND_LEN) {
        bytes_sent += usbSendBytes(&sendBuf[bytes_sent],MAX_SEND_LEN-bytes_sent);
    }
    
    digitalWrite(BOARD_LED_PIN,LOW);
}

void wait_ack(void) {
    while (1) {
        while (usbBytesAvailable() < 1);
        if (SerialUSB.read() == 0x1E) {
            break;
        }
        if (digitalRead(BOARD_BUT_PIN)) {
            break;
        }
    }
}

void setup() {
    sendBuf[0] = 10;
    for (int j=1;j<MAX_SEND_LEN;j++) {
        sendBuf[j] = (j%52) + 64; 
    }

    while (!digitalRead(BOARD_BUT_PIN)); // wait for press

}

void loop() {
    tx_packet();
    wait_ack();
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
