#!/usr/bin/env python3
import serial
import sys
import platform
import time

#from mc348864keysrv import MC348864KeySrv
from mc348864keysrv import MCKeySrv
from mc348864keysrv import MC348864PK
from mc348864keysrv import MC460896PK
from mc348864keysrv import MC6688128PK
from mc348864keysrv import MC6960119PK
from mc348864keysrv import MC8192128PK

from Crypto.Hash import SHAKE256


if len(sys.argv) < 2 :
  print("Usage: exec serial-port-name")
  print("For ex.: exec /dev/ttyUSB0 38400")
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



#mysrv = MC348864KeySrv()
mysrv = MCKeySrv()
#print( mysrv )


def data_to_send( rsp ):
  # return ("/".join( [ hex(e) for e in rsp] )).encode('utf-8')
  return bytes( rsp )


def mk_response( i , rsp ):
  return "[%d] <-- [%s / %s]"%( i , hex(rsp[0]) , hex(rsp[1]))

ser.timeout = 4
i = 0

print( ser )

timeout_count = 0

while True:
  i += 1

  cmd = ser.read(1)

  # time out
  if( 1 != len(cmd) ) :
    if 0 == timeout_count : print("[%d] ... "%(i) )
    timeout_count += 1
    continue
  else : timeout_count = 0

  # standard ascii
  if( cmd[0] < 128 ) :
    if( cmd[0] == b'\n' ) : print(" :: [0]" )
    else :
      dd = ser.readline()
      print( " :: [%d]: %s"%( len(dd)+1 , str(cmd+dd)  ) )
    continue

  cmd1 = ser.read(1)
  cmd = cmd + cmd1
  print(" --> [2]: %s/%s :  "%( hex(cmd[0]) , hex(cmd[1]) ) , end = "" , flush = True )

  # wrong cmd format
  if( 0xf0 != (cmd[0]&0xf0) ) :
    print("Wrong cmd format" )
    continue

  # retrive a generated key
  if (0 == (cmd[0]&12)) and (0xff == cmd[1]) :
    print("retrive a previous key with 32 bytes pkhash" )
    # read 32 bytes more data (hash)
    pkhash = ser.read( 32 )
    new_token = mysrv.retrive_key( pkhash )

    msg = 0
    if -1 == new_token : msg = 255
    else : token = new_token
    print("Retrive_key(): return token %d" % (token) )
    rsp = bytes([ 0xf0 + token , msg ])
    print( mk_response(i,rsp) )
    ser.write( rsp );
    continue

  # init
  if 0 == (cmd[0]&12) :
    cls = None
    if 0 == cmd[1] :
        cls = MC348864PK
    elif 1 == cmd[1] :
        cls = MC348864PK
    elif 2 == cmd[1] :
        cls = MC460896PK
    elif 3 == cmd[1] :
        cls = MC6688128PK
    elif 4 == cmd[1] :
        cls = MC6960119PK
    elif 5 == cmd[1] :
        cls = MC8192128PK

    if cls is None :
      print("Wrong cmd format")
      continue

    token = mysrv.init( cls )
    print("Initialize(): return token %d" % (token) )
    rsp = bytes([ 0xf0 + token , 0 ])
    print( mk_response(i,rsp) )
    ser.write( rsp );
    continue

  # compute inv mat
  if( 4 == (cmd[0]&12) ) :
    token = cmd[0]&3
    rsp = mysrv.compute( token )
    print("Compute( %d )  ------>  %s" % ( token , str(rsp) ) )
    rsp = bytes( [0xf0 + 4 + token , int(rsp)&255 ] )
    print( mk_response(i,rsp) )
    ser.write( rsp );
    continue

  # store received data
  if( 8 == (cmd[0]&12) ) :
    token = cmd[0]&3
    idx = cmd[1]
    print("Store( %d , %d ) " % ( token , idx ) )
    print( mk_response(i,cmd) )
    ser.write( cmd );

    #size = MC348864KeySrv.param_block_size
    size = mysrv.get_active_cls(token).block_extsize
    dd = ser.read( size )
    print( "[%d] --> [%d:...]"%( i , len(dd) ) )
    if( size != len(dd) ) : print( "[%d] wrong size [%d:%d] (timeout=%d)"%( i , size,len(dd), ser.timeout) )
    else :
      try :
        print( "[%d] store %d:[%d]"%(i,idx,len(dd)) )
        mysrv.set( token , idx , dd )
      except Exception:
        print( "[%d] Exception while writing storage service"%i )
        pass
    #print( mysrv )
    continue

  # output stored data
  if( 12 == (cmd[0]&12) ) :
    token = cmd[0]&3
    idx = cmd[1]
    print("Read( %d , %d ) " % ( token , idx ) )
    #print( mk_response(i,cmd) )
    dd = mysrv.get( token , idx )

    #hash = SHAKE256.new()
    #hash.update( dd )
    #hv = hash.read(32)
    #print( "[%d] <-- [%s/%s] <-- [%d:...(%s:%d)]"%(i,hex(cmd[0]),hex(cmd[1]),len(dd),hex(hv[1]*256+hv[0]),hv[1]*256+hv[0]) )
    print( "[%d] <-- [%s/%s] <-- [%d:...]"%(i,hex(cmd[0]),hex(cmd[1]),len(dd)) )
    ser.write( cmd )
    ser.write( dd )
    continue



#print( mysrv )
print( ser )


