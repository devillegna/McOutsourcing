#!/usr/bin/env python3
import serial
import sys
import platform
import time


if platform.system() == "Darwin":
    dev = serial.Serial("/dev/tty.usbserial-0001", 38400)
else:
    dev = serial.Serial("/dev/ttyUSB0", 38400)

print("> Returned data:", file=sys.stderr)

#while True:
#    x = dev.read()
#    sys.stdout.buffer.write(x)
#    sys.stdout.flush()



readOut = 0   #chars waiting from laser range finder

connected = False
commandToSend = 1 # get the distance in mm

while True:
    print ("Writing: ",  commandToSend)
    dev.write(str(commandToSend).encode())
    time.sleep(1)
    while True:
        try:
            print ("Write and Wait for somthing:")
            dev.write(str(commandToSend).encode())
            readOut = dev.readline().decode('ascii')
            time.sleep(1)
            print ("Reading: ", readOut) 
            break
        except e:
            print (e)
            pass
    #print ("Restart")
    dev.flush() #flush the buffer
