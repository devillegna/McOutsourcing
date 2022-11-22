
#include "pksrv_n6960_t119.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "randombytes.h"


static
void vec_set_ele( uint8_t *vec , unsigned idx , uint8_t v )
{
  uint8_t bit_clear_1 = 1;
  bit_clear_1 <<= (idx&7);
  vec[ idx>>3 ] &= ~bit_clear_1;

  v &= 1;
  v <<= (idx&7);
  vec[ idx>>3 ] |= v;
}

static
uint8_t vec_get_ele( const uint8_t * vec , unsigned idx )
{
  uint8_t r = vec[idx>>3];
  r >>= (idx&7);
  return r&1;
}

static
void vec_add( uint8_t *v1 , const uint8_t *v0, unsigned len ) { for(unsigned i=0;i<len;i++) v1[i] ^= v0[i]; }



//
// assuming a (ext-)pk matrix of dimension NUM_ROW_PK x NUM_CHUNK_EXTPK (1547x(218x32bit))
//
// 1547 = 48x32 + 11
// 6960 = 217x32 + 16
//
static
void set_extpk( uint8_t * mat )
{
  for(int i=0;i<NUM_ROW_PK;i++) {
    uint8_t * r_i = mat + i*(NUM_CHUNK_EXTPK*4);
    memset( r_i , 0 , 48*4 );
    randombytes( r_i + 48*4 , (218-48)*4 );

    r_i[48*4] = 0;
    r_i[48*4+1] &= ~7;  // 3 bits
    r_i[217*4+2] = 0;
    r_i[217*4+3] = 0;

    vec_set_ele( r_i , i , 1 );
  }
}


// vec_c = mat x vec_a
static
void mat_mul_vec( uint8_t * vec_c , unsigned len_c , const uint8_t *mat , const uint8_t * vec_a , unsigned num_vec_in_mat )
{
  memset( vec_c , 0 , len_c );
  for(unsigned i=0;i<num_vec_in_mat;i++) {
    if( ! vec_get_ele( vec_a , i ) ) continue;

    vec_add( vec_c , mat+i*len_c , len_c );
  }
}




unsigned is_vec_eq(const uint8_t* vec0, const uint8_t* vec1, unsigned len)
{
  uint8_t r = 0;
  for(unsigned i=0;i<len;i++) r |= vec0[i]^vec1[i];
  return (r==0);
}


void get_partial_mat(uint8_t* part_mat, unsigned n_vec_in_a_mat , const uint8_t *mat, unsigned vec_len , unsigned idx)
{
  for(int i=0;i<n_vec_in_a_mat;i++) {
    for(int j=0;j<4;j++) part_mat[i*4+j] = mat[i*vec_len + idx*4 + j];
  }
}


void get_partial_mat2(uint8_t* part_mat, unsigned n_vec_in_a_mat , const uint8_t *mat, unsigned vec_len , unsigned bit_idx)
{
  for(int i=0;i<n_vec_in_a_mat;i++) {
    unsigned idx_byte = bit_idx >> 3;
    unsigned idx_rem  = bit_idx & 7;

    const uint8_t * r_i = mat + i*vec_len;

    for(int j=0;j<4;j++) {
      uint8_t rij0 = (idx_byte+j   >=vec_len)? 0: r_i[idx_byte+j];
      uint8_t rij1 = (idx_byte+j+1 >=vec_len)? 0: r_i[idx_byte+j+1];
      part_mat[i*4+j] = (rij0>>idx_rem)|(rij1<<(8-idx_rem));
    }
  }
}




#define TEST_RUN 10


int main()
{
  printf("test pksrv_n6960_t119\n\n");


  uint8_t mat0[ NUM_ROW_PK    * NUM_CHUNK_EXTPK * 4];
  uint8_t mat1[ NUM_ROW_PREPK * NUM_CHUNK_EXTPK * 4];

  uint8_t part_mat0[ NUM_ROW_PREPK*4 ];
  uint8_t part_mat1[ NUM_ROW_PREPK*4 ];


  int test_ok = 1;
  int genpk_fail = 0;
  for(int i=0;i<TEST_RUN;i++) {
    uint32_t token = pksrv_init( 4 );
    printf("initialize. token: %d\n", token );

    set_extpk( mat0 );

    // generate a random combination
    for(int j=0;j<NUM_ROW_PREPK;j++) {
      randombytes( part_mat0 , (NUM_ROW_PK+7)>>3 );
      mat_mul_vec( &mat1[j*NUM_CHUNK_EXTPK*4] , NUM_CHUNK_EXTPK*4 , mat0 , part_mat0 , NUM_ROW_PK );
    }

    for(int j=0;j<NUM_CHUNK_I;j++) {
//void get_partial_mat(uint8_t* part_mat, unsigned n_vec_in_a_mat , const uint8_t *mat, unsigned vec_len , unsigned idx)
      get_partial_mat( part_mat0 , NUM_ROW_PREPK , mat1 , NUM_CHUNK_EXTPK*4 , j);
      unsigned r = pksrv_store_prepk( token , part_mat0 , j );
      printf("store prepk idx %d: %d\n", j , r );
    }

    unsigned comp_pk = pksrv_compute_pk(token);
    printf("compute pk: %d\n" , comp_pk );
    if( 0 != comp_pk ) { genpk_fail++; continue; }

    for(int j=NUM_CHUNK_I;j<NUM_CHUNK_EXTPK;j++) {
      get_partial_mat( part_mat0 , NUM_ROW_PREPK , mat1 , NUM_CHUNK_EXTPK*4 , j);
      unsigned r = pksrv_store_prepk( token , part_mat0 , j );
      printf("store prepk idx %d: %d\n", j , r );
    }

    for(int j=0;j<NUM_CHUNK_PK;j++) {
      unsigned r = pksrv_get_pk( token , part_mat0 , j );
      printf("get pk idx %d: %d\n", j , r );
      //get_partial_mat2( part_mat0 , NUM_ROW_PK , mat0 , NUM_CHUNK_EXTPK*4 , NUM_ROW_PK+j*4 );

      get_partial_mat2( part_mat1 , NUM_ROW_PK , mat0 , NUM_CHUNK_EXTPK*4 , NUM_ROW_PK+j*32 );

      unsigned is_eq = is_vec_eq( part_mat0 , part_mat1 , NUM_ROW_PK*4 );
      //printf("is pk eq identity? %d\n", is_eq );
      if( !is_eq ) {
        printf("pk chunk identity check fails: (%d,%d)\n",i,j);
        printf("%02x %02x %02x %02x: %02x %02x %02x %02x\n", part_mat0[3],part_mat0[2],part_mat0[1],part_mat0[0] , part_mat1[3],part_mat1[2],part_mat1[1],part_mat1[0]);
        test_ok=0; break; }
    }
    if( !test_ok ) break;
  }

  printf( "# genpk_fail: %d\n", genpk_fail );
  printf( (test_ok)? "test_ok: OK\n": "test_ok: FAIL\n" );

  return 0;
}
