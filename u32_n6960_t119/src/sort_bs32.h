#ifndef _SORT_BS32_H_
#define _SORT_BS32_H_

#include "stdint.h"


// assert(0==(n&(n-1)).  assert(n>=32)
void sort_bs32( uint32_t * bs , int w , unsigned n);


void sort_bs32_1payload( uint32_t * bs , int w , unsigned n, uint32_t *payload);


void sort_bs32_2payload( uint32_t * bs , int w , unsigned n, uint32_t *payload1, uint32_t *payload2);





#endif  //ndef _SORT_BS32_H_
