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
  # return ("/".join( [ hex(e) for e in rsp] )).encode('utf-8')
  return bytes( rsp )


def mk_response( i , rsp ):
  return "[%d] <-- [%s / %s]"%( i , hex(rsp[0]) , hex(rsp[1]))


#def update_array( b ):
#  acc = int(b[0]) + (int(b[1])<<8) + (int(b[2])<<16) + (int(b[3])<<24) + 1
#  for i in range(4):
#    b[i] = acc%256
#    acc >>= 8


ser.timeout = 6
i = 0
rr = 0xf0
ack = 1

while True:
  i += 1
  print("[%d] ..."%(i) )

  cmd = ser.read(2)

  if( 2 != len(cmd) ) :
    print( "[%d] --> [%d]: %s" % ( i , len(cmd),"/".join( map(str,cmd) ) ) )
    if( ack ) :
      rsp = bytes([rr&255,rr&1])
      rr += 1
      if( rr > 256 ) : rr = 0xf0
      print( mk_response(i,rsp) )
      ser.write( rsp );
    continue

  if( b'::' == cmd ) :
    dd = ser.readline()
    print( "[%d] :: [%d]: %s"%( i , len(dd) , str(dd)  ) )
    continue

  print("[%d] --> [%d]: %s"%( i , len(cmd), "/".join( map(hex,cmd) )  ))

  if( 0xf0 != (cmd[0]&0xf0) ) :
    print("[%d] Wrong cmd formant"%i )
    continue

  if( 0 == (cmd[0]&12) ) :
    token = mysrv.init()
    print("[%d] Initialize(): return token %d" % (i , token) )
    rsp = bytes([ 0xf0 + token , 0 ])
    print( mk_response(i,rsp) )
    ser.write( rsp );
    continue

  if( 4 == (cmd[0]&12) ) :
    token = cmd[0]&3
    rsp = mysrv.compute( token )
    print("[%d] Compute( %d ): %s" % ( i, token , str(rsp) ) )
    rsp = bytes( [0xf0 + 4 + token , int(rsp)&255 ] )
    print( mk_response(i,rsp) )
    ser.write( rsp );

    continue

  if( 8 == (cmd[0]&12) ) :
    token = cmd[0]&3
    idx = cmd[1]
    print("[%d] Store( %d , %d ) " % ( i, token , idx ) )
    print( mk_response(i,cmd) )
    ser.write( cmd );

    size = len(mysrv.data[0] )
    dd = ser.read( size )
    print( "[%d] --> [%d: %s]"%(i,len(dd), str(dd)) )
    if( size != len(dd) ) : print( "[%d] wrong size [%d:%d] of timeout"%( i , size,len(dd)) )
    else :
      try :
        print( "[%d] try store [%d]"%(i,len(dd)) )
        mysrv.set( token , idx , dd )
      except Exception:
        pass

    print( mysrv )
    continue

  if( 12 == (cmd[0]&12) ) :
    token = cmd[0]&3
    idx = cmd[1]
    print("[%d] Read( %d , %d ) " % ( i, token , idx ) )
    print( mk_response(i,cmd) )
    ser.write( cmd );

    dd = mysrv.get( token , idx )
    print( "[%d] <-- [%d: %s]"%(i,len(dd), str(dd)) )
    ser.write( dd )
    continue







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
