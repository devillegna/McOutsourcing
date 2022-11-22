
#include "params.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "stdio.h"
#include "stdint.h"


#include "operations.h"

#include "test_keypair.h"

// #define _PRINT_LN_


void print_u32(const unsigned char *data, unsigned len )
{
	const uint32_t * d32 = (const uint32_t *)data;
	unsigned l32 = (len+3)>>2;

	unsigned i;
	for(i=0;i<l32;i++) {
#if defined(_PRINT_LN_)
		if( 0 == (i&7) ) printf("%8d: ",i);
#endif
		printf("0x%08x,", d32[i] );
		if( 3 == (i&3) ) printf(" ");
		if( 7 == (i&7) ) printf("\n");
	}
	if(0 != (l32&7)) printf("\n");
}




#include "hash_shake256.h"
#include "encrypt.h"


int main()
{

	const unsigned char * pk = get_test_pk();
	const unsigned char * sk = get_test_sk();

	unsigned char c[CIPHERTEXT_BYTES];
	unsigned char key[32];
	//unsigned char e[SYS_N/8];

	//crypto_kem_enc( c , k , pk );

/// code copy from encap()
	uint32_t _buffer[ 1 + (SYS_N+31)/32 + (SYND_BYTES+3)/4 + 32/4 ];
	uint32_t _buf_c[(SYND_BYTES+3)/4];

	unsigned char *two_e  = ((uint8_t*)_buffer) + 3;
	unsigned char *e      = ((uint8_t*)_buffer) + 4; // making e 32-bit aligned
	unsigned char *one_ec = ((uint8_t*)_buffer) + 3;

	//

	gen_e(e);

	two_e[0] = 2;
	crypto_hash_32b(c + SYND_BYTES, two_e, 1 + SYS_N/8 );

	encrypt((uint8_t*)_buf_c, pk, e);
	memcpy(c, _buf_c, SYND_BYTES );
	memcpy(one_ec + 1 + SYS_N/8, c, SYND_BYTES + 32);

	one_ec[0] = 1;
	crypto_hash_32b(key, one_ec, 1 + SYS_N/8 + SYND_BYTES + 32 );
/// code copy from encap()

	printf("\n\n//test_e len: %d.\n", SYS_N/8 );
	printf("const static uint32_t __test_e[%d] = {\n", (SYS_N/8)/4);
	print_u32(e,SYS_N/8);
	printf("};\n");

	printf("\n\n//ciphertext len: %d.\n", CIPHERTEXT_BYTES );
	printf("const static uint32_t __test_c[%d] = {\n", (CIPHERTEXT_BYTES)/4);
	print_u32(c,CIPHERTEXT_BYTES);
	printf("};\n");

	printf("\n\n//shared key len: %d.\n", 32 );
	printf("const static uint32_t __test_sharedkey[%d] = {\n", (32)/4);
	print_u32(key,32);
	printf("};\n");

	unsigned char key2[32];
	crypto_kem_dec( key2 , c , sk );
	uint8_t err = 0;
	for(int i=0;i<32;i++) err |= key[i]^key2[i];
	printf("//ERROR CODE: %x\n", err );

	return 0;
}

