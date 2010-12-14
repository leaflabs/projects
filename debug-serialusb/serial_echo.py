from __future__ import print_function

import optparse
import sys

import serial

BAUD = 9600
MAX_PACKET_SIZE = 128
ERROR_CHAR = '*'

def serial_tx_only(port,baudrate=BAUD):
    ser = serial.Serial(port, baudrate=baudrate)

    size = 1
    val = 1
    while True:
        print('size = {0}'.format(size))
        s = str(val)*size
        print('writing packet - len,val: {0},{1}'.format(len(s),s))
        ser.write(s)
        print('wrote packet')
        print('got back {0}'.format(ser.readline().strip()))
        
        size *= 2
        val += 1
        if size > MAX_PACKET_SIZE:
            size = 1
            val = 1

def serial_echo(port, baudrate=BAUD):
    ser = serial.Serial(port, baudrate=baudrate)


    size = 1
    while True:
        print('Handshake: "{0}"'.format(ser.readline().strip()))
        print('size = {0}'.format(size))
        s = ser.read(size)
        print('read packet: "{1}"'.format(len(s), s))
        ser.write(s)
        print('wrote packet back.')

        s = ser.read(size)
        print('read packet response: "{1}"'.format(len(s), s))
        if ERROR_CHAR in s: print('-- errors detected.')

        print()

        size *= 2
        if size > MAX_PACKET_SIZE:
            size = 1


if __name__ == '__main__':
    parser = optparse.OptionParser(usage='usage: %prog [options] port')
    parser.add_option('-b', '--baud-rate', default=BAUD,
                      help='Baud rate (Kbps)')
    parser.add_option('-t', '--only-send', default=False,
                      help='Test program will only send bytes')


    opts, args = parser.parse_args()

    def usage(msg, fatal=True):
        if fatal: print ('Error: ', end='')
        print(msg)
        parser.print_help()
        if fatal: sys.exit(1)

    if len(args) != 1:
        usage('Must specify exactly one serial port.')

    port = args[0]

    print('Using port {0}'.format(port))

    if opts.only_send:
        serial_tx_only(port, baudrate=opts.baud_rate)
    else:
        serial_echo(port, baudrate=opts.baud_rate)
