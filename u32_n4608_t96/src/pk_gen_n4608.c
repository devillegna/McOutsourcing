/*
  This file is for public-key generation
*/

#include "pk_gen.h"

#include "params.h"

#include "util.h"
#include "vec32.h"

#include "run_config.h"

#include <stdint.h>




static const int nblocks_H = (SYS_N + 31) / 32;  //  256
static const int nblocks_I = (GFBITS * SYS_T + 31) / 32;  //  52



static
void alpha_bitreverse( uint32_t * a_bs )
{
  const int len = (SYS_N/32);  // necessay columns only

  uint32_t tmp[GFBITS];
  for(int i=0;i<len;i++) {
    for(int j=0;j<GFBITS;j++) tmp[j] = a_bs[i*GFBITS + j];
    for(int j=0;j<GFBITS;j++) a_bs[i*GFBITS + (GFBITS-1-j)] = tmp[j];
  }
}



#include "sk_util_n8192_t96.h"

#include "benes.h"

static
void gen_1_over_g_bs( uint32_t * invga , const unsigned char *irr, const unsigned char *control_bits )
{
	uint32_t prod32[256][GFBITS];  // 13,312 bytes

	// compute the inverses of the secret \alphas
	calc_1_over_g( prod32, irr );

	uint32_t bit_i[256];  // 1024 bytes
	for(int i=0;i<GFBITS;i++) {
		for(int j=0;j<(1<<(GFBITS-5));j++) bit_i[j] = prod32[j][i];
		benes( (uint8_t*)bit_i , control_bits , 0 );
		for(int j=0;j<SYS_N/32;j++) invga[j*GFBITS+i] = bit_i[j];  // necessary part only
	}
}



static
void fill_in_matrix_1( uint32_t *mat0 , const uint32_t prod32[GFBITS] , const uint32_t consts32[GFBITS] )
{
	for (int k=0;k<GFBITS;k++) { mat0[ k ] = prod32[ k ]; }
	for (int i = 1; i < SYS_T; i++) {
		vec32_mul( &mat0[i*GFBITS], &mat0[(i-1)*GFBITS],consts32);
	}
}


///////////////////////////////////////

#include "hash_shake256.h"

static
void random_linear_combination( uint32_t * mat1 , const uint32_t *mat0 , const unsigned char * seed )
{
    uint8_t row_seed[ (PK_NROWS/8) + 32 ];
    uint8_t *n_seed = &row_seed[ PK_NROWS/8 ];
    memcpy( n_seed , seed , 32 );

    for(unsigned i=0;i<(PK_NROWS+32);i++) {
        shake256( row_seed , sizeof(row_seed) , n_seed , 32 );

        mat1[i] = 0;
        for(unsigned j=0;j<PK_NROWS;j++) {
            uint32_t mask = -( (uint32_t) ((row_seed[j>>3]>>(j&7))&1) );
            mat1[i] ^= mask&mat0[j];
        }
    }
}


///////////////////////////////////////


static inline
void matmadd_32x4(uint32_t *c, const uint32_t *a, const uint32_t *b) {
  uint32_t c0=c[0];
  uint32_t c1=c[1];
  uint32_t c2=c[2];
  uint32_t c3=c[3];
  uint32_t a0=a[0];
  uint32_t a1=a[1];
  uint32_t a2=a[2];
  uint32_t a3=a[3];
  for(int i=0;i<32;i++) {
    uint32_t bi=b[i];
    c0 ^= bi*((a0>>i)&1);
    c1 ^= bi*((a1>>i)&1);
    c2 ^= bi*((a2>>i)&1);
    c3 ^= bi*((a3>>i)&1);
  }
  c[0] = c0;
  c[1] = c1;
  c[2] = c2;
  c[3] = c3;
}


static
uint32_t validate_pk_with_sk( uint32_t * prepk, const uint32_t * recv_pk ,  const uint32_t * invga_bs , const uint32_t * a_bs , uint32_t *buffer_2xGFBITS )
{
  uint32_t * temp0 = buffer_2xGFBITS;
  uint32_t * temp1 = temp0 + GFBITS;

  for(int k=0;k<PK_NROWS;k+=32) {
    for(int j=0;j<GFBITS;j++) temp0[j]=invga_bs[j];
    matmadd_32x4( prepk    , temp0   , recv_pk+k );
    matmadd_32x4( prepk +4 , temp0+4 , recv_pk+k );
    matmadd_32x4( prepk +8 , temp0+8 , recv_pk+k );
    for(int l=0;l<32;l++) prepk[12] ^= ((temp0[12]>>l)&1)*(recv_pk[k+l]);  /// for GFBITS=13
    for(int i=1; i<SYS_T;i++) {
      vec32_mul( temp1 , temp0 , a_bs );
      uint32_t * temp2 = temp0; temp0 = temp1; temp1 = temp2;
      matmadd_32x4( prepk +i*GFBITS   , temp0   , recv_pk+k );
      matmadd_32x4( prepk +i*GFBITS+4 , temp0+4 , recv_pk+k );
      matmadd_32x4( prepk +i*GFBITS+8 , temp0+8 , recv_pk+k );
      for(int l=0;l<32;l++) prepk[i*GFBITS+12] ^= ((temp0[12]>>l)&1)*(recv_pk[k+l]);  /// for GFBITS=13
    }
    invga_bs += GFBITS;  // next (32) columns
    a_bs += GFBITS;
  }

  uint32_t r = 0;
  for(int i=0;i<PK_NROWS;i++) r |= prepk[i];

  return r;
}


///////////////////////////////////////





#include "pksrv_n4608_t96.h"



int pk_gen(unsigned char * pk, const unsigned char * irr, const unsigned char *extra_prng_seed ,
	const unsigned char *control_bits, uint32_t *alpha_bs, uint32_t *buffer )
{
	uint32_t * consts32 = alpha_bs;
	alpha_bitreverse( consts32 );

	// compute the inverses of the secret \alphas
	//uint32_t * prod32 = consts32 + (SYS_N/32)*GFBITS;
	uint32_t * prod32 = buffer;
	gen_1_over_g_bs( prod32 , irr , control_bits );

	// calcuate the inverse matrix for systematiclizing the public matrix
	//uint32_t __mat[ GFBITS * SYS_T ] = {0}; // part of pk, 6656 bytes
	uint32_t * __mat = prod32 + (SYS_N/32)*GFBITS;
	//uint32_t mat0[ GFBITS * SYS_T + 32 ] = {0}; // linear combination of rows of pk for outsourcing, 6656 + 128 bytes
	uint32_t * mat0 = __mat + (GFBITS*SYS_T);     // linear combination of rows of pk for outsourcing, 6656 + 128 bytes

	uint32_t pksrv_token = pksrv_init(PKSRV_PARAM);
	if( -1 == pksrv_token ) return -1;

	for(int i=0;i<nblocks_I;i++) {
		fill_in_matrix_1( __mat , &prod32[i*GFBITS], &consts32[i*GFBITS] );
		random_linear_combination( mat0 , __mat , extra_prng_seed );
		pksrv_store_prepk( pksrv_token , mat0 , i );
	}

	int r = pksrv_compute_pk( pksrv_token );
	if(r) return r;

	for(int i=0; i<nblocks_H-nblocks_I;i++) {
		fill_in_matrix_1( __mat , &prod32[(nblocks_I+i)*GFBITS], &consts32[(nblocks_I+i)*GFBITS] );
		random_linear_combination( mat0 , __mat , extra_prng_seed );
		pksrv_store_prepk( pksrv_token , mat0 , nblocks_I+i );

		pksrv_get_pk( pksrv_token , mat0 , i );

		// have to do validate here
		r = validate_pk_with_sk( __mat, mat0 ,  prod32 , consts32 , mat0 + PK_NROWS);
		if( r ) {
			//printf("validate pk fail [%d]: %x.\n", i , r );
			return -1;
		}
		// store hash of pk chunks
		crypto_hash_32b( pk+32+i*32, mat0, GFBITS*SYS_T*4 );

		//for (int row = 0; row < PK_NROWS; row++) {
		//	store4(pk+(row*PK_ROW_BYTES+i*4) , mat0[row] );  // out-sourced
		//}
	}
	//store4(  pk , pksrv_token );
	crypto_hash_32b(  pk , pk+32 , (nblocks_H-nblocks_I)*32 );

	return 0;
}


