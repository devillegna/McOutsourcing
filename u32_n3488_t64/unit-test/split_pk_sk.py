#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""This script splits pk and sk from keypair-dump-test"""

import sys, os, fcntl


def output( fn , lines , st ):
  ed = -1
  with open( fn , "w" ) as file:
    for i in range(st,len(lines)):
      pos = lines[i].find(':')
      if( 0 <= pos ):
        file.write( lines[i][pos+1:] + '\n' )
      else:
        ed = i
        break
  return ed




#if __name__ == "__main__"





if 2 != len(sys.argv) :
  print( "ext filename\n" )
  sys.exit(2)

with open( sys.argv[1] ) as file:
  lines = [ line.strip() for line in file ]

# find start of sk
st = -1
for i in range(0,len(lines)) :
  if( lines[i].startswith( 'sk ' ) ):
    st = i
    break

if( 0 > st ) : sys.exit( "no sk" )
ed = output( 'sk.txt', lines , st+1 )
if( 0 < ed ) : print( f"out sk [{st}:{ed}]" )

# find start of pk
st = ed
for i in range(st,len(lines)) :
  if( lines[i].startswith( 'pk ' ) ):
    st = i
    break

if( ed == st ) : sys.exit( "no pk" )
ed = output( 'pk.txt', lines , st+1 )
if( ed != st ) : print( f"out pk [{st}:{ed}]" )

print("success\n")



