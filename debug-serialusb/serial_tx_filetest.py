
"""
Expects maple to send the same data as with serial_txt_test.py 
(sketch is main-tx-test.cpp) but instead of reading it on the fly
from the serial port, we pipe the serial port to a file, capture the
maple tx dump, and then read it back here. 

This little script fails on lines 1 and 2047, not a big deal and has 
more to do with edge cases of the analysis then a failure of the tx
"""

MAX_SEND_LEN = 2048

data_file = open('maple_tx_test_data_d1.txt','r')
lines = data_file.readlines()
len_last_line = 0
for i,line in enumerate(lines):
    new_len = len(line)
    if len_last_line != new_len -1:
        print "Failed on line %i, expected len %i, got %i" % (i,len_last_line+1,new_len)
    len_last_line = new_len
