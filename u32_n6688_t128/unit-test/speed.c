#include "params.h"
#include "randombytes.h"
#include "benes.h"
#include "sk_gen.h"
#include "pk_gen.h"
#include "encrypt.h"
#include "decrypt.h"
#include "transpose.h"
#include "fft.h"
#include "fft_tr.h"
#include "bm.h"
#include "gf.h"

#include <stdio.h>
#include <stdlib.h>

int cmp_llu(const void *a, const void*b)
{
    if(*(long long *)a < *(long long *)b) return -1;
    if(*(long long *)a > *(long long *)b) return 1;

    return 0;
}

long long median(long long *l, size_t llen)
{
    qsort(l,llen,sizeof(long long),cmp_llu);

    if(llen%2) return l[llen/2];
    else return (l[llen/2-1]+l[llen/2])/2;
}

long long get_cycles(void)
{
    unsigned long long result;
    asm volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax"
    : "=a" (result) ::  "%rdx");

    return result;
}

#define NRUNS 10
#define REP 10

int main()
{
	int i, j;

	long long ts[ NRUNS ];

	unsigned char s[ PK_NROWS / 8 ];
	unsigned char pk[ PK_NROWS * PK_NCOLS / 8 ];
	unsigned char sk[ SK_BYTES ];
	unsigned char e_out[ 1 << (GFBITS-3) ];
	unsigned char e[ 1 << (GFBITS-3) ];

//	vec128 x[GFBITS];
//	vec128 y[GFBITS];
//	vec128 z[GFBITS];

//	vec128 bs[ 64 ];
//	vec128 cond[ COND_BYTES/16 ];

	uint64_t fft_in[ 2 ][ GFBITS ];
	vec fft_out[ 128 ][ GFBITS ];

	vec fft_tr_in[ 128 ][ GFBITS ];
	uint64_t fft_tr_out[ 4 ][ GFBITS ];

	uint64_t bm_in[ 4 ][ GFBITS ];
	uint64_t bm_out[ 2 ][ GFBITS ];

	gf a, b, c;
	//

//	randombytes(s, sizeof(s));
	randombytes(sk, sizeof(sk));
	randombytes(pk, sizeof(pk));
//	randombytes(e_out, sizeof(e_out));
	
	unsigned char irr[ IRR_BYTES ];
	uint64_t perm[ SYS_N ];

	randombytes(irr, sizeof(irr));
	randombytes(perm, sizeof(perm));

	//

//	irr_gen(irr);
//	perm_gen(perm);

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		fprintf(stderr, "i=%d\n", i);

		for (j = 0; j < REP; j++)
		{
//			sk_part_gen(irr, perm);
		}
	}

	for (i = 0; i < NRUNS-1; i++)\
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");\
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		fprintf(stderr, "i=%d\n", i);

		for (j = 0; j < REP; j++)
		{
			pk_gen(pk, irr, perm);
		}
	}

	for (i = 0; i < NRUNS-1; i++)\
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");\
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		fprintf(stderr, "i=%d\n", i);

		for (j = 0; j < REP; j++)
			crypto_kem_keypair(pk, sk);
	}

	for (i = 0; i < NRUNS-1; i++)\
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");\
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	fprintf(stderr, "---------- gf_inv ----------\n");

	randombytes(&a, sizeof(a));
	randombytes(&b, sizeof(b));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			a = gf_inv(b);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	fprintf(stderr, "---------- gf_mul ----------\n");

	randombytes(&a, sizeof(a));
	randombytes(&b, sizeof(b));
	randombytes(&c, sizeof(c));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			c = gf_mul(a, b);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//
/*
	fprintf(stderr, "---------- gf_sq ----------\n");

	randombytes(&a, sizeof(a));
	randombytes(&b, sizeof(b));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			a = gf_sq(b);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));
*/
	//
/*
	fprintf(stderr, "---------- vec128_mul ----------\n");

	randombytes(x, sizeof(x));
	randombytes(y, sizeof(y));
	randombytes(z, sizeof(z));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			vec128_mul(z, x, y);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));
*/
	//

	fprintf(stderr, "---------- Encryption ----------\n");

	randombytes(pk, sizeof(pk));
	randombytes(s, sizeof(s));
	randombytes(e, sizeof(e));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			encrypt(s, e, pk);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	fprintf(stderr, "---------- Decryption ----------\n");

	randombytes(e_out, sizeof(e_out));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			decrypt(e_out, sk, s);
//			crypto_kem_dec(e_out, sk, s);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//
/*
	fprintf(stderr, "---------- Transpose ----------\n");

	randombytes((unsigned char *) bs, sizeof(bs));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			transpose_64x128_sp(bs);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));
*/
	//
/*
	fprintf(stderr, "---------- Benes ----------\n");

	randombytes((unsigned char *) bs, sizeof(bs));
	randombytes((unsigned char *) cond, sizeof(cond));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			benes(bs, cond, 0);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));
*/
	//

	fprintf(stderr, "---------- FFT ----------\n");

	randombytes((unsigned char *) fft_in, sizeof(fft_in));
	randombytes((unsigned char *) fft_out, sizeof(fft_out));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			fft(fft_out, fft_in);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	fprintf(stderr, "---------- FFT transpose ----------\n");

	randombytes((unsigned char *) fft_tr_in, sizeof(fft_tr_in));
	randombytes((unsigned char *) fft_tr_out, sizeof(fft_tr_out));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			fft_tr(fft_tr_out, fft_tr_in);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	fprintf(stderr, "---------- BM ----------\n");

	randombytes((unsigned char *) bm_in, sizeof(bm_in));
	randombytes((unsigned char *) bm_out, sizeof(bm_out));

	for (i = 0; i < NRUNS; i++)
	{
		ts[i] = get_cycles();

		for (j = 0; j < REP; j++)
		{
			bm(bm_out, bm_in);
		}
	}

	for (i = 0; i < NRUNS-1; i++)
	{
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	//

	return 0;
}

