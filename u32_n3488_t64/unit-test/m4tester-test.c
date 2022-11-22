
//#include "randombytes.h"
#include "params.h"
#include "util.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>


#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

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


void print_u32(const unsigned char *data, unsigned len )
{
	const uint32_t * d32 = (const uint32_t *)data;
	unsigned l32 = (len+3)>>2;

	for(unsigned i=0;i<l32;i++) {
		if( 0 == (i&7) ) printf("%3d: ",i);
		printf("0x%08x,", d32[i] );
		if( 3 == (i&3) ) printf(" ");
		if( 7 == (i&7) ) printf("\n");
	}
}



uint8_t check_eq( const uint8_t *vec0, const uint8_t *vec1, unsigned len)
{
	uint8_t diff = 0;
	for(unsigned i=0;i<len;i++){
		diff |= vec0[i]^vec1[i];
	}
	return diff==0;
}



#define NRUNS 100
#define REP 1




#include "my_test.h"




int main()
{
	long long t1,t2;
	unsigned r;

	init_data();

	long long ts[ NRUNS ];

	uint32_t rs = 0;
	printf("spped1(): %s.\n", speed1_name );
	for (int j = 0; j < NRUNS; j++) {
		t1 = get_cycles();
		rs = speed1();
		t2 = get_cycles();
		ts[j] = t2-t1;
	}
	printf("ret: %x\n", rs);
	printf("%lld .. %lld cycles.\n", ts[0] , ts[NRUNS-1] );
	fprintf(stderr, "median: %llu\n\n", median(ts, NRUNS-1));

	printf("spped2(): %s.\n", speed2_name );
	for (int j = 0; j < NRUNS; j++) {
		t1 = get_cycles();
		rs = speed2();
		t2 = get_cycles();
		ts[j] = t2-t1;
	}
	printf("ret: %x\n", rs);
	printf("%lld .. %lld cycles.\n", ts[0] , ts[NRUNS-1] );
	fprintf(stderr, "median: %llu\n\n", median(ts, NRUNS-1));

	printf("test1(): %s.\n", test1_name );
	r = test1();
	printf("result: 0x%x.\n\n", r );

	printf("test2(): %s.\n", test2_name );
	r = test2();
	printf("result: 0x%x.\n\n", r );


	return 0;
}
