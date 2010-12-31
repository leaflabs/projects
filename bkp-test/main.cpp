#include "wirish.h"
#include "bkp.h"
#include "rcc.h"

void setup() {
    pinMode(13, OUTPUT);
}

void check_set() {
    if (__get_bits(RCC_APB1ENR, BIT(28))) {
        SerialUSB.println("PWREN set in RCC_APB1ENR");
    } else {
        SerialUSB.println("PWREN cleared in RCC_APB1ENR");
    }

    if (__get_bits(RCC_APB1ENR, BIT(27))) {
        SerialUSB.println("BKPEN set in RCC_APB1ENR");
    } else {
        SerialUSB.println("BKPEN cleared in RCC_APB1ENR");
    }
}

void loop() {
    static int toggle = 0;
    bool ok;
    toggle ^= 1;
    digitalWrite(13, toggle);

    uint32 time = millis();
    uint16 wmask = time & 0xFFFF;
    SerialUSB.print("time = ");
    SerialUSB.println(time);

    SerialUSB.println("enabling backup interface");
    bkp_init();
    check_set();

    ok = true;
    SerialUSB.println("enabling backup writes");
    bkp_enable_writes();
    for (uint8 i = 1; i <= NR_BKP_REGS; i++) {
        uint16 wval = i & wmask;
        bkp_write(i, wval);
        uint16 rval = bkp_read(i);
        if (rval != wval) {
            SerialUSB.print("write failure: wrote 0x");
            SerialUSB.print(wval, HEX);
            SerialUSB.print(" to ");
            SerialUSB.print((int)i);
            SerialUSB.print(", but read: 0x");
            SerialUSB.println(rval, HEX);
            ok = false;
        }
    }
    if (ok) SerialUSB.println("write/read ok!");
    SerialUSB.println();

    ok = true;
    SerialUSB.println("disabling backup writes");
    bkp_disable_writes();
    check_set();
    for (uint8 i = 1; i <= NR_BKP_REGS; i++) {
        bkp_write(i, 0xBAD);
        uint16 rval = bkp_read(i);
        uint16 expected = i & wmask;
        if (rval == 0xBAD) {
            SerialUSB.print("protection failure on register ");
            SerialUSB.print((int)i);
            SerialUSB.println(" (got 0xBAD back)");
            ok = false;
        }
        if (rval != expected) {
            SerialUSB.print("read failure: reg ");
            SerialUSB.print((int)i);
            SerialUSB.print("expected 0x");
            SerialUSB.print(expected, HEX);
            SerialUSB.print(", but got 0x");
            SerialUSB.println(rval, HEX);
            ok = false;
        }
    }
    if (ok) SerialUSB.println("write protection ok!");
    SerialUSB.println();

    SerialUSB.println("disabling backup interface");
    bkp_disable();
    ok = true;
    for (uint8 i = 1; i <= NR_BKP_REGS; i++) {
        bkp_write(i, 0xBAD);
        uint16 rval = bkp_read(i);
        uint16 expected = i & wmask;
        if (rval != expected) {
            SerialUSB.print("read weirdness on reg ");
            SerialUSB.print((int)i);
            SerialUSB.print(". read back: ");
            SerialUSB.println(rval, HEX);
        }
    }
    if (ok) { SerialUSB.println("well, that was ok but strange"); }
    SerialUSB.println();

    SerialUSB.println("disabling writes");
    bkp_disable_writes();
    ok = true;
    for (uint8 i = 1; i <= NR_BKP_REGS; i++) {
        bkp_write(i, 0xBEEF);
        uint16 rval = bkp_read(i);
        uint16 expected = i & wmask;
        if (rval != expected) {
            SerialUSB.print("wrote 0xBEEF to reg ");
            SerialUSB.print((int)i);
            SerialUSB.print(". read back: ");
            SerialUSB.println(rval, HEX);
            ok = false;
        }
    }
    if (ok) { SerialUSB.println("write protection ok!"); }
    SerialUSB.println();

    SerialUSB.println("----------------------------------------\n");
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
