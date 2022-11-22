#include "stdint.h"




int gauss_elim_32( void * mm , int d , int w , int h )
{
  if( w&31 ) return 0;
  int w32 = w>>5;
  uint32_t *mat = (uint32_t*)mm;
  for(int i=0;i<d;i++) {
    uint32_t * row_i = mat + i*w32;
    int i0 = i>>5;
    int i1 = i&31;
    uint32_t pivot = (row_i[i0]>>i1)&1;
    if(0==pivot) {
      for(int j=i+1;j<h;j++) {
        uint32_t * row_j = mat + j*w32;
        uint32_t p1 = (row_j[i0]>>i1)&1;
        if( p1 ) {
          pivot = p1;
          for(int k=i0;k<w32;k++) { uint32_t t= row_i[k]; row_i[k] = row_j[k]; row_j[k]=t;  }
          break;
        }
      }
    }
    if(0==pivot) return 0;
#define _TRI_ELIM_
#if defined(_TRI_ELIM_)
    for(int j=i+1;j<h;j++) {
#else
    for(int j=0;j<h;j++) {
      if( i==j ) continue;
#endif
      uint32_t *row_j = mat + j*w32;
      uint32_t p1 = (row_j[i0]>>i1)&1;
      if( 0 == p1 ) continue;
      for(int k=i0;k<w32;k++) row_j[k] ^= row_i[k];
    }
  }
#if  defined(_TRI_ELIM_)
  int d32 = d>>5;
  for(int i=d-2;i>=0;i--) {
    uint32_t *row_i = mat + i*w32;
    for(int j=i+1;j<d;j++) {
      uint32_t pj = row_i[j>>5]&(1<<(j&31));
      if( 0 == pj ) continue;
      uint32_t *row_j = mat + j*w32;
      for(int k=d32;k<w32;k++) row_i[k] ^= row_j[k];
    }
  }
#endif
  return 1;
}




static inline
void fill_in_tab( uint32_t* tab , const uint32_t * v )
{
  uint32_t v0 = v[0];
  uint32_t v1 = v[1];
  uint32_t v2 = v[2];
  uint32_t v3 = v[3];

  tab[0] = 0;
  tab[1] = v0;
  tab[2] = v1;
  tab[3] = v0^v1;
  tab[4] = v2;
  tab[5] = v0^v2;
  tab[6] = v1^v2;
  tab[7] = v0^v1^v2;
  tab[8] = v3;
  tab[9] = v3^tab[1];
  tab[10] = v3^tab[2];
  tab[11] = v3^tab[3];
  tab[12] = v3^tab[4];
  tab[13] = v3^tab[5];
  tab[14] = v3^tab[6];
  tab[15] = v3^tab[7];
}


#include "stdlib.h"


void gf2mat_mul32( void * _mat_c , void * _mat_a , int w_a , int h_a , void * _mat_b )
{
  uint32_t *mat_c = (uint32_t*)_mat_c;
  uint32_t *mat_b = (uint32_t*)_mat_b;

  uint8_t *mat_a = (uint8_t*)_mat_a;

  int w8 = (w_a+7)>>3;
#if 1
  uint32_t *m4ri_tabs = (uint32_t*) malloc( 4*16*2 * w8 );
  if( NULL == m4ri_tabs ) return;
  for(int i=0;i<w8;i++) {
    fill_in_tab( m4ri_tabs + i*32    , mat_b + i*8 );
    fill_in_tab( m4ri_tabs + i*32+16 , mat_b + i*8+4 );
  }

  for(int i=0;i<h_a;i+=4) {
    uint32_t v0 = 0;
    uint32_t v1 = 0;
    uint32_t v2 = 0;
    uint32_t v3 = 0;

    uint8_t * r_ai0 = mat_a + i*w8;
    uint8_t * r_ai1 = mat_a + (i+1)*w8;
    uint8_t * r_ai2 = mat_a + (i+2)*w8;
    uint8_t * r_ai3 = mat_a + (i+3)*w8;
    for(int j=0;j<w8;j++) {
      uint32_t * tab_l = m4ri_tabs + j*32;
      uint32_t * tab_h = m4ri_tabs + j*32 + 16;
      v0 ^= tab_l[r_ai0[j]&15] ^ tab_h[r_ai0[j]>>4];
      v1 ^= tab_l[r_ai1[j]&15] ^ tab_h[r_ai1[j]>>4];
      v2 ^= tab_l[r_ai2[j]&15] ^ tab_h[r_ai2[j]>>4];
      v3 ^= tab_l[r_ai3[j]&15] ^ tab_h[r_ai3[j]>>4];
    }
    mat_c[i] = v0;
    mat_c[i+1] = v1;
    mat_c[i+2] = v2;
    mat_c[i+3] = v3;
  }
  free( m4ri_tabs );

#else
  for(int i=0;i<h_a;i++) {
    uint32_t vec = 0;
    for(int j=0;j<w8;j++) {
      uint8_t aij = mat_a[i*w8+j];
      for(int k=0;k<8;k++) vec ^= ((aij>>k)&1)*mat_b[j*8+k];
    }
    mat_c[i] = vec;
  }
#endif
}
