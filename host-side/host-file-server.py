#!/usr/bin/env python3
import serial
import sys
import platform
import time

from filesrv import FileSrv


if len(sys.argv) < 2 :
  print("Usage: exec serial-port-name")
  exit(-1)

#if platform.system() == "Darwin":
#    dev = serial.Serial("/dev/tty.usbserial-0001", 38400)
#else:
#    dev = serial.Serial("/dev/ttyUSB0", 38400)

try:
  ser = serial.Serial( *sys.argv[1:])
except :
  print( 'Exception: something wrong' )
  exit(-1)

print( ser )

mysrv = FileSrv()
print( mysrv )


def data_to_send( rsp ):
  return ("/".join( [ hex(e) for e in rsp] )).encode('utf-8')
  # return bytes( rsp )

while True:
  print("Waiting for instruction:")
  cmd_and_data = ser.readline()
  print("[%d] %s"%( len(cmd_and_data) , cmd_and_data) )

  #print("cpr q(%d) with %c(%d)  %d"%( ord('q') , cmd_and_data[0] , cmd_and_data[0] , 'q'==cmd_and_data[0] ) )
  if( 4 != len(cmd_and_data) ) : print("Wrong instruction: (4bytes)"); continue
  if( 0xbb != cmd_and_data[0] ) : print("Wrong instruction: (0xbb...)"); continue

  cmd = int( cmd_and_data[1] ) >> 6
  idx = int( cmd_and_data[1] ) & 0x3f
  pkg_idx = int( cmd_and_data[2] )

  if( idx>16 and pkg_idx==0xbb ) :
    print( "got exit signal!\n" )
    ser.close()
    break

  # init()
  if( 0 == cmd ):
    token = mysrv.init()
    response = bytearray( 4 )
    response[0] = 0xbb
    response[1] = token
    print( "init(): rsp: [%s  (%d)] "%( str(response) , token ) )
    ser.write( data_to_send(response) )

  # compute_pk()
  if( 1 == cmd ):
    r = mysrv.compute( idx )
    response = bytearray( 4 )
    response[0] = 0xbb
    response[1] = cmd_and_data[1]
    response[2] = int(r)
    print( "compute( %d ): [ %s ] "%(idx , str(response) ) )
    ser.write( data_to_send( response ) )

  # store_prepk()
  if( 2 == cmd ):
    print("receiving data")
    # should set timeout
    dd = ser.read(8)

    r = mysrv.set( idx ,pkg_idx , dd )
    response = bytearray( cmd_and_data )
    response[3] = r
    print( "store( %d , %d , [%d] ): return ack [%s]"%(idx, pkg_idx , r , str(response)) )
    ser.write( data_to_send( response )  )

  # get_pk()
  if( 3 == cmd ):
    r = mysrv.get( idx , pkg_idx )
    # succ
    response = cmd_and_data
    print( "read( %d , %d ): return ack [%s] and data:"%(idx, pkg_idx, str(response)) )
    ser.write( data_to_send( response)  )
    ser.write( data_to_send( r ) )
    # XXX: mysrv.get() fails


print( mysrv )
print( ser )





'''



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


'''
