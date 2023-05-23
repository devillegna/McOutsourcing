/*
  This file is for Niederreiter encryption
*/

#include "encrypt.h"

#include "randombytes.h"
#include "params.h"
#include "util.h"
//#include "uint16_sort.h"
#include "uint16_qsort.h"

#include <stdint.h>

#include "run_config.h"



#if !( defined( _M4_ASM_ ) && ( 3488==SYS_N || 4608==SYS_N )  )

static
void write_e(unsigned char *e, const uint16_t * index )
{

	uint32_t mask;
	uint32_t val[ SYS_T ];
	uint32_t e_int[ (SYS_N+31)/32 ];


	for (int j = 0; j < SYS_T; j++)
		val[j] = 1 << (index[j] & 31);

	for (int i = 0; i < (SYS_N+31)/32; i++) 
	{
		e_int[i] = 0;

		for (int j = 0; j < SYS_T; j++)
		{
			mask = i ^ (index[j] >> 5);
			mask -= 1;
			mask >>= 31;
			mask = -mask;

			e_int[i] |= val[j] & mask;
		}
	}

	// output
#if 0 == (SYS_N%32)
	for (int i = 0; i < SYS_N/32; i++) store4(e+i*4, e_int[i]);
#else
	// only parameter is SYS_N = 6960
	int i;
	for (i = 0; i < (SYS_N)/32; i++) {
		store4(e, e_int[i]);
		e += 4;
	}
	for (int j=0; j<(SYS_N&31); j+=8)
		e[j/8] = (e_int[i]>>j) & 0xff;
#endif

}

#endif



/* output: e, an error vector of weight SYS_T */
//static
void gen_e(unsigned char *e)
{
#if (1<<GFBITS) != SYS_N
#define TAU (2*SYS_T)
	int count;
#else
#define TAU (SYS_T)
#endif
	union {
		uint16_t nums[TAU];
		unsigned char bytes[TAU*sizeof(uint16_t)];
	} buf;

	uint16_t ind[ SYS_T];

	while (1)
	{
		randombytes(buf.bytes, sizeof(buf.bytes));

#if TAU == 2*SYS_T
		count = 0;
		for(int i=0; i<TAU && count < SYS_T ;i++) {
			buf.nums[i] = load_gf(buf.bytes + i*2);
			if(buf.nums[i] < SYS_N) ind[ count++ ] = buf.nums[i];
		}
		if( count < SYS_T ) continue;
#else
		for(int i=0; i<SYS_T; i++) ind[i] = load_gf(buf.bytes + i*2);
#endif

		// check for repetition

		//uint16_sort(ind, SYS_T);
		uint16_qsort(ind, 0, SYS_T-1);

		int eq = 0;
		for (int i = 1; i < SYS_T; i++)
			if (ind[i-1] == ind[i])
				eq = 1;

		if (eq == 0)
			break;
	}

#if defined( _M4_ASM_ ) && ( 3488==SYS_N )
	idx_to_vec_64_3488_asm( e , ind );
#elif defined( _M4_ASM_ ) && ( 4608==SYS_N )
	idx_to_vec_96_4608_asm( e , ind );
#else
	write_e( e , ind );
#endif
}




#include "pksrv.h"


static inline
uint32_t _reduce_x4(uint32_t b0, uint32_t b1, uint32_t b2, uint32_t b3 ) {
  b0 ^= b0 >> 2;
  b1 ^= b1 >> 2;
  b2 ^= b2 >> 2;
  b3 ^= b3 >> 2;

  b0 ^= b0 >> 1;
  b1 ^= b1 >> 1;
  b2 ^= b2 >> 1;
  b3 ^= b3 >> 1;

  b0 &= 0x11111111;
  b1 &= 0x11111111;
  b2 &= 0x11111111;
  b3 &= 0x11111111;

  b0 ^= (b1<<1)^(b2<<2)^(b3<<3);
  b0 ^= b0>>4;
  b0 ^= b0>>8;
  b0 ^= b0>>16;
  return b0&0xf;
}


#include "hash_shake256.h"

/* input: public key pk , error vector e */
/* output: syndrome s = pk * e */
int encrypt(unsigned char *s, const unsigned char *pk, const unsigned char *e)
{

#ifdef KAT
  {
    int k;
    printf("encrypt e: positions");
    for (k = 0;k < SYS_N;++k)
      if (e[k/8] & (1 << (k&7)))
        printf(" %d",k);
    printf("\n");
  }
#endif
	//syndrome(s, pk, e);

	for (int i = 0; i < SYND_BYTES; i++) s[i] = e[i];

	uint32_t dest[NUM_ROW_PK] = {0};
	uint32_t chunk[SIZE_CHUNK_PK/4];

	uint8_t hash_32b[32];
	uint32_t token = pksrv_retrive_pk( pk );
	if( -1 == token ) return -1;
	pk += 32;

	for(int i=0;i<NUM_CHUNK_PK;i++) {
		uint32_t src =    e[SYND_BYTES+i*4+0];
		src |= ((uint32_t)e[SYND_BYTES+i*4+1])<<8;
		src |= ((uint32_t)e[SYND_BYTES+i*4+2])<<16;
		src |= ((uint32_t)e[SYND_BYTES+i*4+3])<<24;

		pksrv_get_pk( token , chunk , i );

		// check if a received pk is equal to its hash
		uint8_t hash_sum = 0;
		crypto_hash_32b( hash_32b , chunk , SIZE_CHUNK_PK );
		for(int j=0;j<32;j++) hash_sum |= pk[j]^hash_32b[j];
		if( hash_sum ) return -1;
		pk += 32;

		for(int j=0;j<NUM_ROW_PK;j++) dest[j] ^= chunk[j]&src;
	}

	for(int j=0;j<NUM_ROW_PK;j+=8) {
		s[j>>3] ^= _reduce_x4( dest[j+0] , dest[j+1] , dest[j+2] , dest[j+3] ) ^
			(_reduce_x4( dest[j+4] , dest[j+5] , dest[j+6] , dest[j+7] )<<4);
	}
	return 0;
}