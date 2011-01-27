#include "wirish.h"
#include "usb.h"

const uint32 ARITHMETIC_BUF_SIZE = 256;
const uint32 FIXED_BUF_SIZE = 2048;

#define RESPONSE_COMM SerialUSB

void fill_arithmetic(void);
bool arithmetic_buf_ok(uint8*, int,  int);

void fill_fixed(void);
bool fixed_buf_ok(uint8*);

void setup(void) {
    pinMode(BOARD_BUTTON_PIN, INPUT);
    pinMode(BOARD_LED_PIN, OUTPUT);
    Serial1.begin(9600);
}


void loop(void) {
    fill_fixed();               // or fill_arithmetic()
}

void fill_fixed(void) {
    static uint8 buf[FIXED_BUF_SIZE];
    uint32 recvd = 0;

    while (recvd < FIXED_BUF_SIZE) {
        recvd += usbReceiveBytes(buf + recvd, FIXED_BUF_SIZE - recvd);
    }

    RESPONSE_COMM.println("received packet.");

    if (!fixed_buf_ok(buf)) {
        digitalWrite(BOARD_LED_PIN, HIGH);
        RESPONSE_COMM.println("STOP");
        while (true) continue;
    }

    RESPONSE_COMM.println("OK");
}

bool fixed_buf_ok(uint8 *buf) {
    for (uint32 i = 0; i < FIXED_BUF_SIZE; i++) {
        if (buf[i] != (uint8)i) return false;
    }
    return true;
}

void fill_arithmetic(void) {
    static uint8 buf[ARITHMETIC_BUF_SIZE];
    static uint32 size = 1;
    bool reads_ok = true;
    uint32 recvd = 0;

    // read size bytes
    while(recvd < size) {
        recvd += usbReceiveBytes(buf + recvd, size - recvd);
    }

    // we expect size-1 copies of (uint8)size, then a newline
    for (uint32 i = 0; i < size; i++) {
        if (!arithmetic_buf_ok(buf, size, i)) reads_ok = false;
    }

    // on read error, stop
    while (!reads_ok) {
        digitalWrite(BOARD_LED_PIN, HIGH);
        continue;
    }

    RESPONSE_COMM.print(size);
    RESPONSE_COMM.println(" OK");

    size++;
    if (size > ARITHMETIC_BUF_SIZE) size = 1;
}

bool arithmetic_buf_ok(uint8 *buf, int size, int i) {
    if (i == size - 1) {
        if (buf[i] != (uint8)'\n') {
            Serial1.print("BAD READ: size = ");
            Serial1.print(size);
            Serial1.print(", expected newline, got ");
            Serial1.println((int)buf[i]);
            return false;
        }
    } else if (buf[i] != (uint8)size) {
        Serial1.print("BAD READ: size = ");
        Serial1.print(size);
        Serial1.print(", buf[");
        Serial1.print(i);
        Serial1.print("] = ");
        Serial1.println((int)buf[i]);
        return false;
    }
    return true;
}

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
