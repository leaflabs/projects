from __future__ import print_function

import contextlib
import sys

import serial

ARITHMETIC_MAX_SIZE = 256
FIXED_SIZE = 2048
TIMEOUT = 1.0

def arithmetic_send(usb_serial, usart):
    size = 1
    while True:
        line = chr(size) * (size - 1) + '\n'
        try:
            nbytes = usb_serial.write(line)
            if nbytes == len(line):
                print('size = {0} sent'.format(size))
            else:
                print('serial.Serial.write not working; aborting')
        except serial.SerialTimeoutException:
            print('usb_serial write timeout')
            return
        finally:
            if usart.inWaiting():
                print('usart:')
                print('\t')
                while usart.inWaiting(): print(usart.read(), end='')
                print()

        print('size', size, 'response:')
        print('\t', repr(usb_serial.readline()))

        size += 1
        if size > ARITHMETIC_MAX_SIZE: size = 1

def fixed_send(usb_serial, usart):
    data = bytearray(i % 256 for i in xrange(FIXED_SIZE))
    print('sending:', repr(data))
    while True:
        n = usb_serial.write(data)
        if (n != len(data)):
            print('only sent', n, 'bytes out of', FIXED_SIZE, '; quitting')
            return

        print('sent all', n, 'bytes. responses:')

        usb_line = usb_serial.readline().strip()
        while usb_line != 'OK' and usb_line != 'STOP':
            print('\tusb:', repr(usb_line))
            usb_line = usb_serial.readline().strip()
            
        if usb_line == 'OK':
            print('got OK; continuing')
        elif usb_line == 'STOP':
            print('got STOP; quitting')
            return


def main(usb_port, usart_port):
    usb_serial = serial.Serial(usb_port, writeTimeout=TIMEOUT)
    with contextlib.closing(usb_serial):
        usart = serial.Serial(usart_port)
        with contextlib.closing(usart):
            fixed_send(usb_serial, usart)

if __name__ == '__main__':
    usb_port, usart_port = sys.argv[1:3]
    main(usb_port, usart_port)
