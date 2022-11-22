#!/usr/bin/env python3

from gf2matlib import *


from Crypto.Hash import SHAKE256

class MC348864PK:
  # 3488/32 = 109
  # 768/32  = 24
  n_cols = 3488
  n_rows = 768
  n_rows_spare = 32

  block_width   = 32
  block_size    = 4*768
  block_extsize = (4*768)+(4*32)
  n_blocks_full = 3488//32        # 109
  n_blocks_pk    = (3488-768)//32  # 85
  n_blocks_sys = 768//32        # 24

  def __init__(self):
    self.rawdata = [ None for i in range(MC348864PK.n_blocks_full) ]
    self.invmat  = gf2mat_from_bytes(  bytes( (MC348864PK.n_rows)*(MC348864PK.n_rows+MC348864PK.n_rows_spare)//8 ) , (MC348864PK.n_rows+MC348864PK.n_rows_spare)//8 );
    self.pkdata  = [ None for i in range(MC348864PK.n_blocks_pk) ]
    self.hash_blocks = None

  def set_raw( self , idx , data ):
    if( len(data) != MC348864PK.block_extsize ): raise Exception('Wrong data size')
    self.rawdata[idx] = gf2mat_from_bytes( data , MC348864PK.block_width//8 )
    return bool(self.rawdata[idx])

  def compute_invmat( self ):
    #print("create mat")
    c = gf2mat_create( MC348864PK.n_rows , MC348864PK.n_rows + MC348864PK.n_rows_spare )
    for i in range( MC348864PK.n_blocks_sys ): gf2mat_set_submat( c , i*MC348864PK.block_width , self.rawdata[i] )
    #print("calculate inv")
    ii = gf2mat_inv( c )
    if not ii : return False
    self.invmat = ii
    return True

  def get_pkblock( self , idx ) :
    if self.pkdata[idx] is None :
        if self.rawdata[ MC348864PK.n_blocks_sys +idx]  is None : return None

        ii = gf2mat_mul( self.invmat , self.rawdata[ MC348864PK.n_blocks_sys +idx] )
        self.pkdata[idx] = bytes( gf2mat_to_bytearray(ii) )
    return self.pkdata[idx]

  def is_pkdata_ready( self ):
    for i in range(len(self.pkdata)):
      if self.pkdata[i] is None:
        return False
    return True

  def get_pkhash( self ):
    if self.hash_blocks  is None :
      if( not self.is_pkdata_ready() ): return None
      hashs = []
      for i in range(len(self.pkdata)):
        hash = SHAKE256.new()
        hash.update( self.pkdata[i] )
        hashs.append( hash.read(32) )
      hash = SHAKE256.new()
      hash.update( b''.join(hashs) )
      self.hash_blocks = hash.read(32)
    return self.hash_blocks


from Crypto.Cipher import AES
from Crypto.Util   import Counter

import time

def test_MC348864PK():
  print( "create an MC348864Key" )
  mypk = MC348864PK()


  aes = AES.new( b'\x00'*32 , AES.MODE_CTR , counter=Counter.new(128,initial_value=0) )
  print( "asign 24 random blocks" )
  ss = []
  for i in range(24):
    #d = bytes_rand(4*800)
    d = aes.encrypt( b'\x00'*3200 )
    ss.append( mypk.set_raw( i , d ) )
  print( ss )

  print( "comput inv mat:" )
  st = time.time()
  r = mypk.compute_invmat()
  print( "use %s sec."%(time.time()-st) )
  print( "invmat? %d"%r )
  if not r : return False

  print( "asign 85 random blocks" )
  ss = []
  for i in range(85):
    #d = bytes_rand(4*800)
    d = aes.encrypt( b'\x00'*3200 )
    ss.append( mypk.set_raw( 24+i , d ) )
  print( ss )

  #print( "get pkhash: " )
  #print( mypk.get_pkhash() )  #Exception!!

  print( "retrieve 85 pk blocks" )
  tmsg = []
  for i in range(85):
    st = time.time()
    d = mypk.get_pkblock( i )
    tmsg.append( time.time()-st )
    #print( "%d: got %d bytes"%(i,len(d)) )
  print( tmsg )

  print( "get pkhash: " )
  print( mypk.get_pkhash() )

  print( "retrieve one pk blocks" )
  d = mypk.get_pkblock( 0 )
  print( "got %d bytes"%(len(d)) )








###############################



class MC460896PK:
  # 4608/32 = 144
  # (13*96=1248)/32 = 39
  n_cols = 4608
  n_rows = 1248
  n_rows_spare = 32

  block_width   = 32
  block_size    = 4*1248           # 4992
  block_extsize = (4*1248)+(4*32)  # 5120
  n_blocks_full = 4608//32         # 144
  n_blocks_pk   = (4608-1248)//32  # 105
  n_blocks_sys  = 1248//32         # 39

  def __init__(self):
    self.rawdata = [ None for i in range(MC460896PK.n_blocks_full) ]
    self.invmat  = gf2mat_from_bytes(  bytes( (MC460896PK.n_rows)*(MC460896PK.n_rows+MC460896PK.n_rows_spare)//8 ) , (MC460896PK.n_rows+MC460896PK.n_rows_spare)//8 );
    self.pkdata  = [ None for i in range(MC460896PK.n_blocks_pk) ]
    self.hash_blocks = None

  def set_raw( self , idx , data ):
    if( len(data) != MC460896PK.block_extsize ): raise Exception('Wrong data size')
    self.rawdata[idx] = gf2mat_from_bytes( data , MC460896PK.block_width//8 )
    return bool(self.rawdata[idx])

  def compute_invmat( self ):
    #print("create mat")
    c = gf2mat_create( MC460896PK.n_rows , MC460896PK.n_rows + MC460896PK.n_rows_spare )
    for i in range( MC460896PK.n_blocks_sys ): gf2mat_set_submat( c , i*MC460896PK.block_width , self.rawdata[i] )
    #print("calculate inv")
    ii = gf2mat_inv( c )
    if not ii : return False
    self.invmat = ii
    return True

  def get_pkblock( self , idx ) :
    if self.pkdata[idx] is None :
        if self.rawdata[ MC460896PK.n_blocks_sys +idx]  is None : return None

        ii = gf2mat_mul( self.invmat , self.rawdata[ MC460896PK.n_blocks_sys +idx] )
        self.pkdata[idx] = bytes( gf2mat_to_bytearray(ii) )
    return self.pkdata[idx]

  def is_pkdata_ready( self ):
    for i in range(len(self.pkdata)):
      if self.pkdata[i] is None:
        return False
    return True

  def get_pkhash( self ):
    if self.hash_blocks  is None :
      if( not self.is_pkdata_ready() ): return None
      hashs = []
      for i in range(len(self.pkdata)):
        hash = SHAKE256.new()
        hash.update( self.pkdata[i] )
        hashs.append( hash.read(32) )
      hash = SHAKE256.new()
      hash.update( b''.join(hashs) )
      self.hash_blocks = hash.read(32)
    return self.hash_blocks







###############################



class MC6960119PK:
  # 6960/32 = 217.5
  # (13*119=1547)/32 = 48.34
  n_cols = 6960
  n_rows = 1547
  n_rows_spare = 32
  n_rows_mod_32 = 11

  block_width   = 32
  block_size    = 4*1547               # 6188
  block_extsize = (4*1547)+(4*32)      # 6316
  n_blocks_full = (6960+31)//32        # 218
  n_blocks_pk   = (6960-1547+31)//32   # 170
  n_blocks_sys  = (1547+31)//32        # 49
  n_byte_sys    = ((1547+7)//8)        # 194   (1552/8)
  n_byte_sys_ext   = (((1547+7)//8)+4)  # 198

  def __init__(self):
    self.rawdata = [ None for i in range(MC6960119PK.n_blocks_full) ]
    self.invmat  = gf2mat_from_bytes(  bytes(MC6960119PK.n_byte_sys_ext*MC6960119PK.n_rows) , MC6960119PK.n_byte_sys_ext );
    self.pkdata  = [ None for i in range(MC6960119PK.n_blocks_pk) ]
    self.hash_blocks = None

  def set_raw( self , idx , data ):
    if( len(data) != MC6960119PK.block_extsize ): raise Exception('Wrong data size')
    self.rawdata[idx] = gf2mat_from_bytes( data , MC6960119PK.block_width//8 )
    return bool(self.rawdata[idx])

  def compute_invmat( self ):
    #print("create mat")
    c = gf2mat_create( (MC6960119PK.n_byte_sys)*8 , MC6960119PK.n_rows + MC6960119PK.n_rows_spare )  #// XXX
    for i in range( MC6960119PK.n_blocks_sys-1 ): gf2mat_set_submat( c , i*MC6960119PK.block_width , self.rawdata[i] )
    # last block. remaining terms.
    idx = MC6960119PK.n_blocks_sys-1
    for i in range( MC6960119PK.n_rows+MC6960119PK.n_rows_spare ): c[i].set_part(
		idx*MC6960119PK.block_width, self.rawdata[idx][i].get_part(0,MC6960119PK.n_rows-idx*MC6960119PK.block_width) )

    #print("calculate inv")
    ii = gf2mat_inv2( c , MC6960119PK.n_rows )   #// XXX: check
    if not ii : return False
    self.invmat = ii
    return True

  def get_pkblock( self , idx ) :
    if self.pkdata[idx] is None :
        if self.rawdata[ MC6960119PK.n_blocks_sys-1 +idx]  is None : return None
        if (idx != MC6960119PK.n_blocks_pk-1) and (self.rawdata[ MC6960119PK.n_blocks_sys +idx]  is None) : return None

        premat = [ int.from_bytes( bytes(row), 'little' ) for row in self.rawdata[MC6960119PK.n_blocks_sys-1+idx] ]
        if MC6960119PK.n_blocks_pk-1 != idx :
            premat = [ premat[i] + (int.from_bytes(bytes(self.rawdata[MC6960119PK.n_blocks_sys+idx][i]),'little')<<32) for i in range(len(premat)) ]
        mat = [ gf2vec( ((ri>>11)&0xffffffff).to_bytes(4,'little') ) for ri in premat ]
        ii = gf2mat_mul( self.invmat , mat )
        self.pkdata[idx] = bytes( gf2mat_to_bytearray(ii) )
    return self.pkdata[idx]

  def is_pkdata_ready( self ):
    for i in range(len(self.pkdata)):
      if self.pkdata[i] is None:
        return False
    return True

  def get_pkhash( self ):
    if self.hash_blocks  is None :
      if( not self.is_pkdata_ready() ) : return None
      hashs = []
      for i in range(len(self.pkdata)):
        hash = SHAKE256.new()
        hash.update( self.pkdata[i] )
        hashs.append( hash.read(32) )
      hash = SHAKE256.new()
      hash.update( b''.join(hashs) )
      self.hash_blocks = hash.read(32)
    return self.hash_blocks







###############################





class MC6688128PK:
  # 6688/32 = 209
  # (13*128=1664)/32 = 52
  n_cols = 6688
  n_rows = 1664
  n_rows_spare = 32

  block_width   = 32
  block_size    = 4*1664           # 6656
  block_extsize = (4*1664)+(4*32)  # 6784
  n_blocks_full = 6688//32         # 209
  n_blocks_pk   = (6688-1664)//32  # 157
  n_blocks_sys  = 1664//32         # 52

  def __init__(self):
    self.rawdata = [ None for i in range(MC6688128PK.n_blocks_full) ]
    self.invmat  = gf2mat_from_bytes(  bytes( (MC6688128PK.n_rows)*(MC6688128PK.n_rows+MC6688128PK.n_rows_spare)//8 ) , (MC6688128PK.n_rows+MC6688128PK.n_rows_spare)//8 );
    self.pkdata  = [ None for i in range(MC6688128PK.n_blocks_pk) ]
    self.hash_blocks = None

  def set_raw( self , idx , data ):
    if( len(data) != MC6688128PK.block_extsize ): raise Exception('Wrong data size')
    self.rawdata[idx] = gf2mat_from_bytes( data , MC6688128PK.block_width//8 )
    return bool(self.rawdata[idx])

  def compute_invmat( self ):
    #print("create mat")
    c = gf2mat_create( MC6688128PK.n_rows , MC6688128PK.n_rows + MC6688128PK.n_rows_spare )
    for i in range( MC6688128PK.n_blocks_sys ): gf2mat_set_submat( c , i*MC6688128PK.block_width , self.rawdata[i] )
    #print("calculate inv")
    ii = gf2mat_inv( c )
    if not ii : return False
    self.invmat = ii
    return True

  def get_pkblock( self , idx ) :
    if self.pkdata[idx] is None :
        if self.rawdata[ MC6688128PK.n_blocks_sys +idx]  is None : return None
        ii = gf2mat_mul( self.invmat , self.rawdata[ MC6688128PK.n_blocks_sys +idx] )
        self.pkdata[idx] = bytes( gf2mat_to_bytearray(ii) )
    return self.pkdata[idx]

  def is_pkdata_ready( self ):
    for i in range(len(self.pkdata)):
      if self.pkdata[i] is None:
        return False
    return True

  def get_pkhash( self ):
    if self.hash_blocks  is None :
      if( not self.is_pkdata_ready() ): return None
      hashs = []
      for i in range(len(self.pkdata)):
        hash = SHAKE256.new()
        hash.update( self.pkdata[i] )
        hashs.append( hash.read(32) )
      hash = SHAKE256.new()
      hash.update( b''.join(hashs) )
      self.hash_blocks = hash.read(32)
    return self.hash_blocks




###############################



class MC8192128PK:
  # 8192/32 = 256
  # 1664/32 = 52
  n_cols = 8192
  n_rows = 1664
  n_rows_spare = 32

  block_width   = 32
  block_size    = 4*1664
  block_extsize = (4*1664)+(4*32)
  n_blocks_full = 8192//32         # 256
  n_blocks_pk   = (8192-1664)//32  # 204
  n_blocks_sys  = 1664//32         # 52

  def __init__(self):
    self.rawdata = [ None for i in range(MC8192128PK.n_blocks_full) ]
    self.invmat  = gf2mat_from_bytes(  bytes( (MC8192128PK.n_rows)*(MC8192128PK.n_rows+MC8192128PK.n_rows_spare)//8 ) , (MC8192128PK.n_rows+MC8192128PK.n_rows_spare)//8 );
    self.pkdata  = [ None for i in range(MC8192128PK.n_blocks_pk) ]
    self.hash_blocks = None

  def set_raw( self , idx , data ):
    if( len(data) != MC8192128PK.block_extsize ): raise Exception('Wrong data size')
    self.rawdata[idx] = gf2mat_from_bytes( data , MC8192128PK.block_width//8 )
    return bool(self.rawdata[idx])

  def compute_invmat( self ):
    #print("create mat")
    c = gf2mat_create( MC8192128PK.n_rows , MC8192128PK.n_rows + MC8192128PK.n_rows_spare )
    for i in range( MC8192128PK.n_blocks_sys ): gf2mat_set_submat( c , i*MC8192128PK.block_width , self.rawdata[i] )
    #print("calculate inv")
    ii = gf2mat_inv( c )
    if not ii : return False
    self.invmat = ii
    return True

  def get_pkblock( self , idx ) :
    if self.pkdata[idx] is None :
        if self.rawdata[ MC8192128PK.n_blocks_sys +idx]  is None : return None
        ii = gf2mat_mul( self.invmat , self.rawdata[ MC8192128PK.n_blocks_sys +idx] )
        self.pkdata[idx] = bytes( gf2mat_to_bytearray(ii) )
    return self.pkdata[idx]

  def is_pkdata_ready( self ):
    for i in range(len(self.pkdata)):
      if self.pkdata[i] is None:
        return False
    return True

  def get_pkhash( self ):
    if self.hash_blocks  is None :
      if( not self.is_pkdata_ready() ): return None
      hashs = []
      for i in range(len(self.pkdata)):
        hash = SHAKE256.new()
        hash.update( self.pkdata[i] )
        hashs.append( hash.read(32) )
      hash = SHAKE256.new()
      hash.update( b''.join(hashs) )
      self.hash_blocks = hash.read(32)
    return self.hash_blocks




###############################



def test_MCXXXXPK( cls ):
  print( "create an %s instance"% cls.__name__ )
  mypk = cls()

  aes = AES.new( b'\x00'*32 , AES.MODE_CTR , counter=Counter.new(128,initial_value=0) )
  print( "asign %d random blocks"%cls.n_blocks_sys )
  ss = []
  for i in range(cls.n_blocks_sys):
    #d = bytes_rand(4*800)
    d = aes.encrypt( b'\x00'*cls.block_extsize )
    ss.append( mypk.set_raw( i , d ) )
  print( ss )

  print( "comput inv mat:" )
  st = time.time()
  r = mypk.compute_invmat()
  print( "use %s sec."%(time.time()-st) )
  print( "invmat? %d"%r )
  if not r : return False

  print( "asign %d random blocks"%cls.n_blocks_pk )
  ss = []
  for i in range(cls.n_blocks_pk):
    #d = bytes_rand(4*800)
    d = aes.encrypt( b'\x00'*cls.block_extsize )
    ss.append( mypk.set_raw( cls.n_blocks_sys+i , d ) )
  print( ss )

  #print( "get pkhash: " )
  #print( mypk.get_pkhash() )  #Exception!!

  print( "retrieve %d pk blocks"%cls.n_blocks_pk )
  tmsg = []
  for i in range(cls.n_blocks_pk):
    st = time.time()
    d = mypk.get_pkblock( i )
    tmsg.append( time.time()-st )
    #print( "%d: got %d bytes"%(i,len(d)) )
  print( tmsg )

  print( "get pkhash: " )
  print( mypk.get_pkhash() )

  print( "retrieve one pk blocks" )
  d = mypk.get_pkblock( 0 )
  print( "got %d bytes"%(len(d)) )





###############################





class MCKeySrv:
  def __init__(self):
    self.keys = []
    self.active_index = 0
    self.active_token = 0

  def init(self, cls):
    self.keys.append( (cls,cls()) )
    self.active_token = (self.active_token+1)&3
    self.active_index = len(self.keys)-1
    return self.active_token

  def retrive_key( self , pkhash ) :
    for i in range(len(self.keys)) :
      if pkhash == self.keys[i][1].get_pkhash() :
        self.active_token = (self.active_token+1)&3
        self.active_index = i
        return self.active_token
    return -1

  def get_active_cls( self , token ):
    if( token != self.active_token ): raise Exception('Wrong token!')
    return self.keys[self.active_index][0]

  def set( self , token , idx , data ) :
    if( token != self.active_token ): raise Exception('Wrong token!')
    return self.keys[self.active_index][1].set_raw( idx , data )

  def compute( self , token ) :
    if( token != self.active_token ) : raise Exception('Wrong token!')
    return self.keys[self.active_index][1].compute_invmat()

  def get( self , token , idx ) :
    if( token != self.active_token ): raise Exception('Wrong token!')
    return self.keys[self.active_index][1].get_pkblock( idx )

  def get_pkhash( self , token ) :
    if( token != self.active_token ) : raise Exception('Wrong token!')
    return self.keys[self.active_index][1].get_pkhash()

  def __repr__( self ) :
    if( 0 == len(self.keys) ) : return 'MCKeySrv [0]'
    return f'MCKeySrv [{len(self.keys)}], active: {self.active_token} -> [{self.active_index}/{len(self.keys)}]: {self.keys[self.active_index][0].__name__}\n'







def test_MCKeySrv( cls ):
  #cls = MC348864PK

  print( "create an MCKeySrv" )
  mysrv = MCKeySrv()

  print( "srv.init( %s ) --> token"% cls.__name__ )
  token = mysrv.init( cls )
  print( "token: %d"%token )

  print( "asign %d random blocks"% cls.n_blocks_sys )
  ss = []
  for i in range( cls.n_blocks_sys ):
    d = bytes_rand( cls.block_extsize )
    ss.append( mysrv.set( token , i , d ) )
  print( ss )

  print( "comput inv mat:" )
  r = mysrv.compute( token )
  print( "invmat? %d"%r )
  if not r : return False


  print( "asign the %d-th random block"% cls.n_blocks_sys )
  d = bytes_rand( cls.block_extsize )
  print( mysrv.set( token , cls.n_blocks_sys , d ) )

  print( "retrieve one pk blocks" )
  d = mysrv.get( token , 0 )
  print( "got %d bytes"%(len(d)) )

  print( "retrieve an un-retrivable pk block" )
  d = mysrv.get( token , 1 )
  print( "got %d bytes"%(len(d)) )





def test_MCKeySrv2( cls ):
  #cls = MC348864PK

  print( "create an MCKeySrv" )
  mysrv = MCKeySrv()
  print( mysrv )

  print( "srv.init( %s ) --> token"% cls.__name__ )
  token = mysrv.init( cls )
  print( "token: %d"%token )

  print( "assign %d random blocks"% cls.n_blocks_sys )
  ss = []
  for i in range( cls.n_blocks_sys ):
    d = bytes_rand( cls.block_extsize )
    ss.append( mysrv.set( token , i , d ) )
  print( ss )

  print( "comput inv mat:" )
  r = mysrv.compute( token )
  print( "invmat? %d"%r )
  if not r : return False

  print( "assign full %d blocks"% cls.n_blocks_full )
  for i in range( cls.n_blocks_sys , cls.n_blocks_full ):
    d = bytes_rand( cls.block_extsize )
    ss.append( mysrv.set( token , i , d ) )
  print( ss )

  print( "retrieve all pk blocks [%d]"%cls.n_blocks_pk )
  d = []
  for i in range( cls.n_blocks_pk ) :
    dd = mysrv.get( token , i )
    d.append( len(dd) )
  print( d )

  print( "get pk hash" )
  pkhash = mysrv.get_pkhash( token )
  print( pkhash )
  print( mysrv )

  print( "init new key:" )
  token = mysrv.init( cls )
  print( "token: %d"%token )
  print( mysrv )

  print( "retrive original key:" )
  token = mysrv.retrive_key( pkhash )
  print( "token: %d"%token )
  print( mysrv )

  print( "retrive one pk block:" )
  dd = mysrv.get( token , 0 )
  print( "get %d bytes" % len(dd) )


###############################


'''
class MC348864KeySrv:
  def __init__(self):
    self.pks = None
    self.token = 0

  def init(self):
    self.pks = MC348864PK()
    self.token += 1
    self.token &= 3
    return self.token

  def set( self , token , idx , data ) :
    if( token != self.token ): raise Exception('Wrong token!')
    return self.pks.set_raw( idx , data )

  def compute( self , token ) :
    if( token != self.token ) : raise Exception('Wrong token!')
    return self.pks.compute_invmat()

  def get( self , token , idx ) :
    if( token != self.token ): raise Exception('Wrong token!')
    return self.pks.get_pkblock( idx )

  #def __repr__( self ) :
  #  return ( 'FileSrv %d\n'%( self.token ) ) + "".join( [ '[{}]: {}\n'.format( i , self.data[i] ) for i in range(len(self.data)) ] )




def test_MC348864KeySrv():
  print( "create an MC348864KeySrv" )
  mysrv = MC348864KeySrv()

  print( "srv.init() --> token" )
  token = mysrv.init()
  print( "token: %d"%token )

  print( "asign 25 random blocks" )
  ss = []
  for i in range(25):
    d = bytes_rand(4*800)
    ss.append( mysrv.set( token , i , d ) )
  print( ss )

  print( "comput inv mat:" )
  r = mysrv.compute( token )
  print( "invmat? %d"%r )
  if not r : return False

  print( "retrieve one pk blocks" )
  d = mysrv.get( token , 0 )
  print( "got %d bytes"%(len(d)) )

'''


if __name__ == '__main__' :
  #test_MC348864PK()
  #test_MCXXXXPK( MC8192128PK )
  #test_MCXXXXPK( MC348864PK )
  #test_MCXXXXPK( MC460896PK )
  #test_MCXXXXPK( MC6688128PK )

  #test_MCKeySrv( MC348864PK )
  #test_MCKeySrv( MC460896PK )
  #test_MCKeySrv( MC6688128PK )
  #test_MCKeySrv( MC6960119PK )
  test_MCKeySrv2( MC348864PK )
  #test_MC348864KeySrv()
