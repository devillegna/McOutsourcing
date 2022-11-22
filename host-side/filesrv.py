#!/usr/bin/env python3



class FileSrv:
  def __init__(self, num_block = 4 , block_size = 8 ):
    self.size = block_size
    self.data = [ bytearray(self.size) for i in range(num_block) ]
    self.token = 0

  def init(self):
    self.token += 1
    self.token &= 3
    return self.token

  def get( self , token , idx ) :
    if( token == self.token ): return bytes(self.data[idx])
    else : return bytes(self.size)

  def set( self , token , idx , data ) :
    if( token != self.token ): raise Exception('Wrong token!')
    if( len(data) != self.size ): raise Exception('Wrong data size')
    self.data[idx][:] = data
    return len(self.data[idx])

  def compute( self , token ) :
    if( token != self.token ) : return False
    return True


  def __repr__( self ) :
    return ( 'FileSrv %d\n'%( self.token ) ) + "".join( [ '[{}]: {}\n'.format( i , self.data[i] ) for i in range(len(self.data)) ] )



def test_filesrv():
  mysrv = FileSrv()
  print( mysrv )

  mysrv = FileSrv(8,16)
  print( mysrv )

  tkn = mysrv.init()
  mysrv.set( tkn , 1 , b'0123456789abcdef' )
  print( mysrv )

  print( mysrv.get( tkn , 0 ) )
  print( mysrv.get( tkn , 1 ) )


if __name__ == '__main__' :
  test_filesrv()
