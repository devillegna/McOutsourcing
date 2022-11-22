
#include "pksrv_n3488_t64.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "randombytes.h"




void identity_mat(uint8_t* mat, unsigned w_byte, unsigned h)
{
  memset(mat,0,w_byte*h);
  for(unsigned i=0;i<h;i++) {
    unsigned idx=i>>3;
    uint8_t v=(1<<(i&7));
    mat[i*w_byte+idx] |= v;
  }
}

unsigned is_vec_eq(const uint8_t* vec0, const uint8_t* vec1, unsigned len)
{
  uint8_t r = 0;
  for(unsigned i=0;i<len;i++) r |= vec0[i]^vec1[i];
  return (r==0);
}


void get_partial_mat(uint8_t* part_mat,  const uint8_t *mat, unsigned idx)
{
  int h = (768+32);
  int w_m = 768/8;
  int w_p = 32/8;

  for(int i=0;i<h;i++) {
    for(int j=0;j<w_p;j++) part_mat[i*w_p+j] = mat[i*w_m+idx*w_p+j];
  }
}

void set_partial_mat( uint8_t *mat, const uint8_t* part_mat , unsigned idx)
{
  int h = 768;
  int w_m = 768/8;
  int w_p = 32/8;

  for(int i=0;i<h;i++) {
    for(int j=0;j<w_p;j++) mat[i*w_m+idx*w_p+j] = part_mat[i*w_p+j];
  }
}



#define TEST_RUN 10


int main()
{
  printf("test pksrv_n3488_t64\n\n");


  uint8_t mat0[(768+32)*768/8];
  uint8_t mat1[(768+32)*768/8];
  uint8_t part_mat[ (768+32)*4 ];

  uint8_t mat_i[768*768/8];
  identity_mat( mat_i , 768/8 , 768 );


  int test_ok = 1;
  for(int i=0;i<TEST_RUN;i++) {
    uint32_t token = pksrv_init( 1 );
    printf("initialize. token: %d\n", token );

    randombytes(mat0, sizeof(mat0));

    for(int j=0;j<(768/32);j++) {
      get_partial_mat( part_mat , mat0 , j);
      unsigned r = pksrv_store_prepk( token , part_mat , j );
      printf("store prepk idx %d: %d\n", j , r );
    }

    unsigned comp_pk = pksrv_compute_pk(token);
    printf("compute pk: %d\n" , comp_pk );
    if( 0 != comp_pk ) continue;

    for(int j=0;j<(768/32);j++) {
      get_partial_mat( part_mat , mat0 , j);
      unsigned r = pksrv_store_prepk( token , part_mat , (768/32)+j );
      printf("store prepk idx %d: %d\n", (768/32)+j , r );
    }

    for(int j=0;j<(768/32);j++) {
      unsigned r = pksrv_get_pk( token , part_mat , j );
      printf("get pk idx %d: %d\n", j , r );

      set_partial_mat( mat1 , part_mat , j);
    }

    unsigned is_eq = is_vec_eq( mat_i , mat1, (768*768/8));
    printf("is pk eq identity? %d\n", is_eq );
    if( !is_eq ) { test_ok=0; break; }
  }

  printf( (test_ok)? "OK\n": "FAIL\n" );

  if( !test_ok ) {
    int w = 768/8;
    for(int i=0;i<768;i++) {
      uint8_t *row_i = mat1+i*w;
      for(int j=0;j<w;j++) { printf("%02x",row_i[j]); if(3==(j%4)) printf(" "); }
      printf("\n");
    }
  }


  return 0;
}
