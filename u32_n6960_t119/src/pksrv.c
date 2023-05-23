
#include "pksrv.h"
#include "string.h"

#if !defined(_PKSRV_OUTSOURCING_)

static uint32_t _client_token = 0;

#define NUM_CHUNK_PREPK   (NUM_CHUNK_EXTPK)

static uint8_t _pksrv_buffer[NUM_CHUNK_PREPK+1][SIZE_CHUNK_PREPK];
static uint8_t _pksrv_received[NUM_CHUNK_PREPK];

static uint8_t _pksrv_invmat_computed;
static uint8_t _pksrv_invmat[(NUM_ROW_PREPK)*(NUM_CHUNK_I+1)*4];

static uint8_t _pksrv_pkbuffer[NUM_CHUNK_PK][SIZE_CHUNK_PK];
static uint8_t _pksrv_pkcomputed[NUM_CHUNK_PK];








static void _pksrv_initialize()
{
  // initialize
  memset( _pksrv_received , 0 , sizeof(_pksrv_received) );
  memset( _pksrv_pkcomputed , 0 , sizeof(_pksrv_pkcomputed) );
  _pksrv_invmat_computed = 0;

  memset( &_pksrv_buffer[NUM_CHUNK_PREPK][0] , 0 , SIZE_CHUNK_PREPK );
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
int _compute_invmat(uint32_t *invmat, uint32_t *mat, const int h, const int w_u32, const int h_ext)
{
  const int w = w_u32;
  const int w_ext = w_u32+1;

  // identity matrix
  memset(invmat, 0, h_ext*w_ext*4 );
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
  uint8_t mat[NUM_ROW_PREPK*NUM_CHUNK_I*4];
  for(int j=0;j<NUM_ROW_PREPK;j++) {
    for(int i=0;i<NUM_CHUNK_I;i++) memcpy( &mat[j*(NUM_CHUNK_I*4)+i*4] , &_pksrv_buffer[i][j*4] , 4 );
  }

  int r = _compute_invmat( _pksrv_invmat , mat , NUM_ROW_PK , NUM_CHUNK_I , NUM_ROW_PREPK );
  if( 0 == r ) { _pksrv_invmat_computed = 1; return 0; }
  else { _pksrv_invmat_computed = 0; return -1; }
}


//////////////////////////////////////////////////



static
void _compute_pk( uint8_t * pk_chunk , const uint8_t *invmat , const uint8_t * prepk_chunk )
{
  matrix_mul2( pk_chunk , invmat , NUM_ROW_PK , NUM_CHUNK_I+1 , prepk_chunk , 1 );
  //matrix_mul2_32( pk_chunk , invmat , NUM_ROW_PK , NUM_CHUNK_I+1 , prepk_chunk , 1 );
}


unsigned pksrv_get_pk(uint32_t token, uint8_t * pk_chunk , unsigned idx_pk )
{
  if( token != _client_token ) return -1;
  if( NUM_CHUNK_PK <= idx_pk ) return -1;

  if( 1 != _pksrv_invmat_computed ) return -1;
  if( 1 != _pksrv_received[NUM_CHUNK_I-1+idx_pk] ) return -1;
  if( (idx_pk+1!=NUM_CHUNK_PK) && (1 != _pksrv_received[NUM_CHUNK_I+idx_pk]) ) return -1;

  if( 0 == _pksrv_pkcomputed[idx_pk] ) {
    _pksrv_pkcomputed[idx_pk] = 1;

    uint32_t _prepk[(NUM_CHUNK_I+1)*32] = {0};
    for(int i=0;i<NUM_ROW_PREPK;i++) {
      uint64_t row_i;
      uint8_t * row_i8 = (uint8_t*)&row_i;
      row_i8[0] = _pksrv_buffer[NUM_CHUNK_I-1+idx_pk][i*4];
      row_i8[1] = _pksrv_buffer[NUM_CHUNK_I-1+idx_pk][i*4+1];
      row_i8[2] = _pksrv_buffer[NUM_CHUNK_I-1+idx_pk][i*4+2];
      row_i8[3] = _pksrv_buffer[NUM_CHUNK_I-1+idx_pk][i*4+3];
      row_i8[4] = _pksrv_buffer[NUM_CHUNK_I+idx_pk][i*4];
      row_i8[5] = _pksrv_buffer[NUM_CHUNK_I+idx_pk][i*4+1];
      row_i8[6] = _pksrv_buffer[NUM_CHUNK_I+idx_pk][i*4+2];
      row_i8[7] = _pksrv_buffer[NUM_CHUNK_I+idx_pk][i*4+3];

      row_i >>= 11;
      _prepk[i] = row_i&0xffffffff;
    }

    _compute_pk( &_pksrv_pkbuffer[idx_pk][0] , _pksrv_invmat , &_prepk );
//printf("pk chunk %d computed.\n", idx_pk );
  }
  memcpy( pk_chunk , &_pksrv_pkbuffer[idx_pk][0] , SIZE_CHUNK_PK );

  return 0;

}



#endif  // #if !defined(_PKSRV_STM32F4_OUTSOURCING_)

