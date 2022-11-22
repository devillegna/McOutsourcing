
//#include "randombytes.h"
#include "params.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "stdio.h"
#include "stdint.h"

#include "randombytes.h"
#include "operations.h"


void print_u32(const unsigned char *data, unsigned len )
{
	const uint32_t * d32 = (const uint32_t *)data;
	unsigned l32 = (len+3)>>2;

	for(unsigned i=0;i<l32;i++) {
		if( 0 == (i&7) ) printf("%8d: ",i);
		printf("0x%08x,", d32[i] );
		if( 3 == (i&3) ) printf(" ");
		if( 7 == (i&7) ) printf("\n");
	}
}



#define TEST_RUN 1000

extern int gen_gaussian_ops( int16_t row_swap[] , uint32_t ops[] );

extern int genpoly_gen(gf *out, gf *f);

int mytest()
{
	uint32_t ops[768*24];
	int16_t row_swap[768];

	int r=0;
	while( 1 ) {
		r++;
		randombytes(&ops[0],sizeof(ops));
		if( 0 == gen_gaussian_ops(row_swap,ops) ) break;
	}
	return r;
}


int main()
{


	double accu = 0;
	for(int i=0;i<TEST_RUN;i++){
	int r = mytest();
	accu += r;
	if(99==(i%100)) { printf("it: %d. avg: %lf\n", i+1 , accu/(i+1) ); }
	}

	printf("\n\navg pk return: %lf.\n", accu/TEST_RUN );



	return 0;
}

