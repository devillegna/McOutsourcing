
#include "pksrv.h"
#include "string.h"

#if !defined(_PKSRV_OUTSOURCING_)

static uint32_t _client_token = 0;

#define NUM_CHUNK_PREPK   (NUM_CHUNK_PK+NUM_CHUNK_I)

static uint8_t _pksrv_buffer[NUM_CHUNK_PREPK][SIZE_CHUNK_PREPK];
static uint8_t _pksrv_received[NUM_CHUNK_PREPK];
static uint8_t _pksrv_computed[NUM_CHUNK_PREPK];


static uint8_t _pksrv_invmat_computed;
static uint8_t _pksrv_invmat[NUM_ROW_PREPK*NUM_ROW_PREPK/8];






static void _pksrv_initialize()
{
  // initialize
  memset( _pksrv_received , 0 , sizeof(_pksrv_received) );
  memset( _pksrv_computed , 0 , sizeof(_pksrv_computed) );
  _pksrv_invmat_computed = 0;
}


// return a token
uint32_t pksrv_init( uint32_t parameter )
{
  if(PKSRV_PARAM != parameter ) { _client_token = 0; }
  else { _client_token++; }

  _pksrv_initialize();

  return _client_token;
}

// return a token
uint32_t pksrv_retrive_pk( const uint8_t * pkhash )
{
  return _client_token;  // XXX: fix the fake server
}


unsigned pksrv_store_prepk( uint32_t token , const uint8_t* prepk_chunk , unsigned idx_prepk )
{
  if( token != _client_token ) return -1;
  if( NUM_CHUNK_PREPK <= idx_prepk ) return -1;

  _pksrv_received[idx_prepk] = 1;
  memcpy( &_pksrv_buffer[idx_prepk][0] , prepk_chunk , SIZE_CHUNK_PREPK );
  //for(unsigned i=0; i<SIZE_CHUNK_PREPK; i++) { _pksrv_buffer[idx_prepk][i] = prepk_chunk[i]; }
  return 0;
}


//////////////////////////////////////////////////////


#include "matrix_mul.h"


#include "stdio.h"


// state-less
static
int _compute_invmat(uint32_t *invmat, uint32_t *mat, const int h, const int w, const int h_ext)
{
  const int w_ext = h_ext/32;
  // identity matrix
  memset(invmat, 0, h_ext*h_ext/8 );
  for(int i=0;i<h_ext;i++) {  invmat[i*w_ext+ (i>>5)] = 1<<(i&31); }

  for(int i=0;i<h;i++) {
    int p_idx=i>>5;
    uint32_t tar_bit = 1<<(i&31);

    if( 0 == (mat[i*w + p_idx]&tar_bit) ) {
      for(int j=i+1;j<h_ext;j++) {
        if( 0 == (mat[j*w + p_idx]&tar_bit) ) continue;

        for(int k=0;k<w;k++) { mat[i*w + k] ^= mat[j*w + k]; }
        for(int k=0;k<w_ext;k++) { invmat[i*w_ext + k] ^= invmat[j*w_ext + k]; }

        break;
      }

    }
    if( 0 == (mat[i*w + p_idx]&tar_bit) ) return -1;

    for(int j=0;j<h_ext;j++) {
      if(i==j) continue;
      if( 0==(mat[j*w + p_idx]&tar_bit) ) continue;

      for(int k=0;k<w;k++) { mat[j*w + k] ^= mat[i*w + k]; }
      for(int k=0;k<w_ext;k++) { invmat[j*w_ext + k] ^= invmat[i*w_ext + k]; }
    }
  }

  return 0;
}





///////////////////////////////////////////////////



unsigned pksrv_compute_pk( uint32_t token )  // compute invmat for generating pk actually
{
  if( token != _client_token ) return -1;

  if( 1 == _pksrv_invmat_computed ) return 1;
  uint8_t data_received = 1;
  for(int i=0;i<NUM_CHUNK_I;i++) data_received &= _pksrv_received[i];
  if( 0 == data_received ) return -1;

  // start gaussian elim
  uint8_t mat[NUM_ROW_PREPK*NUM_ROW_PK/8];
  for(int j=0;j<NUM_ROW_PREPK;j++) {
    for(int i=0;i<NUM_CHUNK_I;i++) memcpy( &mat[j*(NUM_CHUNK_I*4)+i*4] , &_pksrv_buffer[i][j*4] , 4 );
  }

  int r = _compute_invmat( _pksrv_invmat , mat , NUM_ROW_PK , NUM_ROW_PK/32 , NUM_ROW_PREPK );
  if( 0 == r ) { _pksrv_invmat_computed = 1; return 0; }
  else { _pksrv_invmat_computed = 0; return -1; }
}


//////////////////////////////////////////////////



static
void _compute_pk( uint8_t * pk_chunk , const uint8_t *invmat , const uint8_t * prepk_chunk )
{
  //matrix_mul2( pk_chunk , invmat , NUM_ROW_PK , NUM_ROW_PREPK/32 , prepk_chunk , 1 );
  matrix_mul2_32( pk_chunk , invmat , NUM_ROW_PK , NUM_ROW_PREPK/32 , prepk_chunk , 1 );
}


unsigned pksrv_get_pk(uint32_t token, uint8_t * pk_chunk , unsigned idx_pk )
{
  if( token != _client_token ) return -1;
  if( NUM_CHUNK_PK <= idx_pk ) return -1;

  if( 1 != _pksrv_invmat_computed ) return -1;
  if( 1 != _pksrv_received[NUM_CHUNK_I+idx_pk] ) return -1;

  if( 0 == _pksrv_computed[idx_pk] ) {
    _compute_pk( pk_chunk , _pksrv_invmat , &_pksrv_buffer[NUM_CHUNK_I+idx_pk][0] );
    memcpy( &_pksrv_buffer[NUM_CHUNK_I+idx_pk][0] , pk_chunk , SIZE_CHUNK_PK );
    _pksrv_computed[idx_pk] = 1; // XXX
//printf("pk chunk %d computed.\n", idx_pk );
  } else {
    memcpy( pk_chunk , &_pksrv_buffer[NUM_CHUNK_I+idx_pk][0] , SIZE_CHUNK_PK );
    //for(unsigned i=0; i<SIZE_CHUNK_PK; i++) { pk_chunk[i] = _pksrv_buffer[NUM_CHUNK_I+idx_pk][i]; }
  }

  return 0;

}



#endif  // #if !defined(_PKSRV_STM32F4_OUTSOURCING_)

