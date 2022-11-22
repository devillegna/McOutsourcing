
/* This file is for implementing the Nassimi-Sahni algorithm */
/* See David Nassimi, Sartaj Sahni "Parallel algorithms to set up the Benes permutationnetwork" */
/* See also https://cr.yp.to/papers/controlbits-20200923.pdf */

#ifndef CONTROLBITS_H
#define CONTROLBITS_H

#include <stdint.h>

//void controlbitsfrompermutation(unsigned char *out,const int16_t *pi,long long w,long long n, int32_t *buffer_2n);
void controlbitsfrompermutation(unsigned char *out,const uint32_t *pi_bs,long long w,long long n, uint32_t *buffer_2n);

//int permutationfromweight(uint16_t* pi, uint32_t * weight,long long n);
int permutationfromweight(uint32_t* pi_bitslice, uint32_t * weight,unsigned w,long long n, uint16_t *buffer_n);

#endif

