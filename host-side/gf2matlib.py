#!/usr/bin/env python3


import random

def bytes_rand(l) :
  return bytes( [random.randrange(256) for i in range(l)] )


#######################################

class gf2vec:
  def __init__(self, src ):
    if isinstance(src,int):
      self.dim = src
      self.v = bytearray( (src+7)//8 )
    elif isinstance(src,bytes):
      self.dim = len(src)*8
      self.v = bytearray( src )
    elif isinstance(src,bytearray):
      self.dim = len(src)*8
      self.v = bytearray( src )
    elif isinstance(src,gf2vec) :
      self.dim = src.dim
      self.v = bytearray(src.v)
    else :
      raise Exception("don't know how to construct gf2vec")

  def set_part(self,idx,v2):
    if(idx&7) :raise Exception("Suport only byte boundary")
    vidx = idx>>3
    self.v[vidx:vidx+len(v2.v)] = v2.v
    return self

  def get_part(self,idx,dim):
    if(idx&7) :raise Exception("Suport only byte boundary")
    vidx = idx>>3
    r = gf2vec( self.v[vidx:vidx+((dim+7)>>3)] )
    r.dim = dim
    return r

  def __getitem__(self,idx):
    return ((self.v[idx>>3])>>(idx&7))&1;

  def __setitem__(self,idx,ele):
    vidx = idx>>3
    ridx = idx&7
    v8 = self.v[vidx] & ~(1<<ridx)
    v8 |= (ele&1)<<ridx
    self.v[vidx] = v8

  def __iadd__(self,e):
    for i in range(len(e.v)): self.v[i] ^= e.v[i]
    return self

  def __add__(self,e):
    r = gf2vec(self)
    r += e
    return r

  def __bytes__(self):
    return bytes(self.v)

  def __len__(self):
    return self.dim

  def __str__(self):
    return "[{}] ".format(self.dim) + "/".join( map(hex,self.v) )


def test_gf2vec():
  a = gf2vec(15)
  print( a )
  a[3] = 1
  a[5] = 1
  print( a )
  a[3] = 0
  print( a )
  print( "a[5]: %d"%a[5] )


################################################


def gf2mat_dump( mat ):
  for i in range(len(mat)):
    print( "[%d]: %s"%(i,str(mat[i])) )

def gf2mat_create( w , h ):
  return [ gf2vec(w) for i in range(h) ]

def gf2mat_set_submat( mat_a , idx , mat_b ):
  for i in range(len(mat_b)):
    mat_a[i].set_part(idx,mat_b[i])
  return mat_a

def gf2mat_get_submat( mat_a , idx , dim ):
  mat_c = []
  for i in range(len(mat_a)):
    mat_c.append( mat_a[i].get_part(idx,dim) )
  return mat_c



def _get_element(mat,r,c,w_byte):
  c0 = c>>3
  c1 = c&7
  d = mat[r*w_byte+c0]>>c1
  return d&1

def _row_add(mat,dest,src,w_byte):
  for i in range(w_byte):
    mat[dest*w_byte+i] ^= mat[src*w_byte+i]

def _bytearray_gf2mat_gauss_elim( mat , d , w , h ):
  w8 = (w+7)>>3
  for i in range(d):
    pivot = _get_element(mat,i,i,w8)
    if not pivot :
      for j in range(i+1,h) :
        if _get_element(mat,j,i,w8) :
          pivot = 1
          _row_add(mat,i,j,w8)
          break
    if not pivot : return False
    for j in range(h):
      if i==j : continue
      if not _get_element(mat,j,i,w8) : continue
      _row_add(mat,j,i,w8)
  return True


def _1_gf2mat_gauss_elim( mat , w ):
  h = len(mat)
  d = w
  w8 = (w+h+7)//8
  mm = bytearray( h*w8 )
  for i in range(h):
    mm[i*w8:(i+1)*w8] = bytes(mat[i])
  r = _bytearray_gf2mat_gauss_elim( mm , d , d+h , h )
  if r :
    for i in range(d):
      mat[i].v[:] = mm[i*w8:(i+1)*w8]
  return r



import ctypes
import pathlib

#// XXX: for 6960t119, w is a padded value
def _2_gf2mat_gauss_elim( mat , w ):
  h = len(mat)
  d = w
  #w8 = (w+h+7)//8     #// XXX: use  len(mat[0])//8
  w8 = (len(mat[0])+7)//8
  mm = bytearray( h*w8 )
  for i in range(h):
    mm[i*w8:(i+1)*w8] = bytes(mat[i])
  cmat = ctypes.create_string_buffer( bytes(mm) , len(mm) )
  clib = ctypes.CDLL(pathlib.Path().resolve() / "gausselim32.so")
  clib.gauss_elim_32.argtypes = [ ctypes.c_void_p , ctypes.c_int , ctypes.c_int , ctypes.c_int ]
  clib.gauss_elim_32.restypes = ctypes.c_int

  #// int gauss_elim_32( void * mm , int d , int w , int h );  // if( w&31 ) return 0;
  assert 0==(w8%4),'matrix width has to be a multiple of 32.'
  r = clib.gauss_elim_32( cmat , d , w8*8 , h )  #// replace (d+h) by w8*8
  if 1==r :
    mm = bytes(cmat.raw)
    for i in range(d):
      mat[i].v[:] = mm[i*w8:(i+1)*w8]
    return True
  return False


def gf2mat_mul32_C( mat_a , w_mat_a , h_mat_a , mat_b ):
  tmp_c = ctypes.create_string_buffer( 4*h_mat_a )
  tmp_b = ctypes.create_string_buffer( bytes(gf2mat_to_bytearray( mat_b )) , size = 4*w_mat_a )
  tmp_a = ctypes.create_string_buffer( bytes(gf2mat_to_bytearray( mat_a )) , size = h_mat_a*((w_mat_a+7)>>3) )

  clib = ctypes.CDLL(pathlib.Path().resolve() / "gausselim32.so")
  clib.gf2mat_mul32.argtypes = [ ctypes.c_void_p , ctypes.c_void_p , ctypes.c_int , ctypes.c_int , ctypes.c_void_p ]
  clib.gf2mat_mul32( tmp_c , tmp_a , w_mat_a , h_mat_a , tmp_b )

  return gf2mat_from_bytes( bytes(tmp_c.raw) , 4 )



def _0_gf2mat_gauss_elim( mat , w ):
  h = len(mat)
  for i in range(w):
    if mat[i][i] == 0 :
      for j in range(i+1,h):
        if mat[j][i] == 1 :
          mat[j] , mat[i] = mat[i] , mat[j]
          break
    if mat[i][i] == 0 : return False
    for j in range(h):
      if i==j : continue
      if mat[j][i] :  mat[j] += mat[i]
  return True


def gf2mat_gauss_elim( mat , w ):
  return _2_gf2mat_gauss_elim( mat , w )



def gf2mat_inv( mat_a ):
  h = len( mat_a )
  w = len( mat_a[0] )
  w_8 = (((w+7)>>3)<<3)
  h_8 = (((h+7)>>3)<<3)

  tmp = gf2mat_create( w_8 + h_8 , h )
  tmp = gf2mat_set_submat( tmp , 0 , mat_a )
  for i in range(h) :  tmp[i][w_8+i] = 1

  r = gf2mat_gauss_elim( tmp , w )
  if r : return gf2mat_get_submat( tmp , w_8 , h_8 )[:w]
  return []


def gf2mat_inv2( mat_a , w ):
  h = len( mat_a )
  #w = len( mat_a[0] )
  w_pad8 = (((w+7)>>3)<<3)
  h_pad8 = (((h+7)>>3)<<3)

  tmp = gf2mat_create( w_pad8 + h_pad8 , h )
  tmp = gf2mat_set_submat( tmp , 0 , mat_a )
  for i in range(h) :  tmp[i][w_pad8+i] = 1

  r = gf2mat_gauss_elim( tmp , w )
  if r : return gf2mat_get_submat( tmp , w_pad8 , h_pad8 )[:w]
  return []



def gf2mat_mul32( mat_a , w_mat_a , h_mat_a , mat_b ):
  mat_c = gf2mat_create( 32 , h_mat_a )
  for i in range(h_mat_a):
    vec_i = mat_a[i]
    for j in range(w_mat_a):
      if vec_i[j] : mat_c[i] += mat_b[j]
  return mat_c


def gf2mat_mul( mat_a , mat_b ):
  #if 32 == len(mat_b[0])  : return gf2mat_mul32( mat_a , len(mat_a[0]) , len(mat_a) , mat_b )
  if (32 == len(mat_b[0])) and ( 0==(len(mat_a[0])&7) ) : return gf2mat_mul32_C( mat_a , len(mat_a[0]) , len(mat_a) , mat_b )
  mat_c = []
  for i in range(len(mat_a)):
    c_i = gf2vec( len(mat_b[0]) );
    for j in range(len(mat_b)) :
      if( mat_a[i][j] ) : c_i += mat_b[j]
    mat_c.append( c_i )
  return mat_c

def gf2mat_from_bytes( byteseq , vec_bytelen ) :
  mat = []
  for i in range(0,len(byteseq),vec_bytelen):
    mat.append( gf2vec( byteseq[i:i+vec_bytelen] ) )
  return mat

def gf2mat_to_bytearray( mat ) :
  vec_bytes = (len(mat[0])+7)//8
  r = bytearray( vec_bytes*len(mat) )
  for i in range(len(mat)):
    r[i*vec_bytes:(i+1)*vec_bytes] = bytes( mat[i] )
  return r



def test_gf2mat():
  print( "create 4 32x128 sub-matrices" )
  a = [ bytes_rand(4*128) for i in range(4) ]
  print( a )
  b = [ gf2mat_from_bytes(ai,4) for ai in a ]

  print( "combine sub-matrices" )
  c = gf2mat_create( 96 , 128 )
  for i in range(3): gf2mat_set_submat( c , i*32 , b[i] )
  gf2mat_dump( c )

  invmat = gf2mat_inv( c )
  print( "inv mat:" )
  gf2mat_dump( invmat )
  if not invmat : return False

  print( "invmat x mat:" )
  e = gf2mat_mul( invmat , c )
  gf2mat_dump( e )

  print( "invmat x the 4th sub-matrix" )
  f = gf2mat_mul( invmat , b[3] )
  gf2mat_dump( f )
  print( "to bytes:" )
  print( gf2mat_to_bytearray(f) )

  return True


import copy

def test_3_gauss_elim():

  while( True ):
    print( "create 3 identical 32x64 matrices" )
    a = gf2mat_from_bytes( bytes_rand(4*64) , 8 )
    b = copy.deepcopy(a)
    c = copy.deepcopy(a)

    #print( "a", id(a) , "b" , id(b) , "c" , id(c) )
    ar = _0_gf2mat_gauss_elim( a , 32 )
    br = _1_gf2mat_gauss_elim( b , 32 )
    cr = _2_gf2mat_gauss_elim( c , 32 )

    print("ar, br, cr = ", ar , br , cr )
    if ar : break

  print("a")
  for i in range(len(a)):
    print(a[i].v)

  # b == a ?
  eq = True
  for i in range(len(b)):
    if b[i].v !=a[i].v :
      eq = False
      break
  print("b==a ?", eq )

  print("c")
  for i in range(len(c)):
    print(c[i].v)


if __name__ == '__main__' :
  test_3_gauss_elim()
  #test_gf2mat()
