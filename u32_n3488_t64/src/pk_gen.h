/*
  This file is for public-key generation
*/

#ifndef PK_GEN_H
#define PK_GEN_H

#include "gf.h"

//#define _F_PARAM_

int pk_gen(unsigned char * pk, const unsigned char * irr, const unsigned char *seed, const unsigned char * control_bits, uint16_t *alpha);

#endif

