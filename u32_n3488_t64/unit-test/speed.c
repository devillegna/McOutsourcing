#include "params.h"
#include "randombytes.h"
#include "operations.h"

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

//	unsigned char s[ PK_NROWS / 8 ];
	unsigned char pk[ PK_NROWS * PK_ROW_BYTES ];
	unsigned char sk[ SK_BYTES ];
//	unsigned char e_out[ 1 << (GFBITS-3) ];
//	unsigned char e[ 1 << (GFBITS-3) ];

	randombytes(sk, sizeof(sk));
	randombytes(pk, sizeof(pk));

	fprintf(stderr, "---------- crypto_kem_keypair ----------\n");
	for (i = 0; i < NRUNS; i++) {
		ts[i] = get_cycles();
		fprintf(stderr, "i=%d\n", i);

		for (j = 0; j < REP; j++) {
			crypto_kem_keypair( pk, sk );
		}
	}

	for (i = 0; i < NRUNS-1; i++) {
		ts[i] = (ts[i+1] - ts[i]) / REP;
		fprintf(stderr, "%llu ", ts[i]);
	}

	fprintf(stderr, "\n");\
	fprintf(stderr, "median: %llu\n", median(ts, NRUNS-1));

	return 0;
}

