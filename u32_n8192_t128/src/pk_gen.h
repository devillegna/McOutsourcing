/*
  This file is for public-key generation
*/

#ifndef PK_GEN_H
#define PK_GEN_H

#include <stdint.h>

int pk_gen(unsigned char * pk, const unsigned char * irr, const unsigned char * prng_seed,
	const unsigned char *control_bits, uint32_t *alpha_bs, uint32_t *buffer);

#endif

