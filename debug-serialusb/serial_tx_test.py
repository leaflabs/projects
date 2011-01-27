import serial

BAUD = 115200
PORT = "/dev/ttyACM0"
MAX_SEND_LEN = 2048

def verify_packet(ser, packet_len):
    """
    Expects to read packet of length packet_len where each 
    byte is of value packet_len % 256. Returns true or false
    """

    bytes_in = bytearray()
    num_bytes = 0
    while (num_bytes < packet_len):
        new_bytes = ser.inWaiting()
        bytes_in += ser.read(new_bytes)
        num_bytes += new_bytes

    if bytes_in[0] != 10:
        print "Tx failed to receive newline, got %i" % (bytes_in[0])

    for byte in bytes_in[1:]:
        if byte != (packet_len % 52) + 64:
            print "Tx test failed, expected %i, got %i" % ((packet_len % 52)+64,byte)
            return False
    return True

if __name__ == '__main__':
    raw_input("Hit any key and start the sketch")
    ser = serial.Serial(PORT,baudrate=BAUD)

    first_fail = -1
    for i in xrange(MAX_SEND_LEN-1):
        if not verify_packet(ser,i+1):
            if first_fail < 0:
                first_fail = i
            print "Test failed on packet #%i" % (i)
        else:
            print "Success, #%i" % (i)
        if (first_fail >= 0):
            print "First Fail: %i" % (first_fail)

    
