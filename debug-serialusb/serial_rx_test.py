from __future__ import print_function

import optparse
import sys

import serial

BAUD = 115200
PORT = "/dev/ttyACM0"
MAX_PACKET_SIZE = 1024
ERROR_CHAR = '*'
MIN_PAKT_CHAR = 33
MAX_PAKT_CHAR = 126

if __name__ == '__main__':
    ser = serial.Serial(PORT, baudrate=BAUD)
    pakt_size = 1
    pakt_char = MIN_PAKT_CHAR

    raw_input("Hit any key once youve started sketch")

    num_bytes = ser.inWaiting()
    print('got back {0}'.format(ser.read(num_bytes).strip()))

    while pakt_size < MAX_PACKET_SIZE:
        print('pakt_size = {0}'.format(pakt_size))
        s = bytearray([pakt_char]*pakt_size)
        print('writing packet - len,val: {0},{1}'.format(len(s),s))
        raw_input("Ready?")
        ser.write(s)
        print('wrote packet')
        print('got back {0}'.format(ser.readline().strip()))
        while (ser.inWaiting() > 0):
            print('got back {0}'.format(ser.readline().strip()))

        pakt_char += 1
        if (pakt_char > MAX_PAKT_CHAR):
            pakt_char = MIN_PAKT_CHAR

        pakt_size += 1
        print('\n')
