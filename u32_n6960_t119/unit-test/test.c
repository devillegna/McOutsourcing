#include "operations.h"

//#include "randombytes.h"
#include "params.h"
#include "sk_gen.h"
#include "pk_gen.h"
#include "encrypt.h"
#include "decrypt.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "api.h"
#define CRYPTO_PUBLICKEYBYTES  PK_BYTES


#define TEST_RUN 2
#define TEST_ENCDEC 10

int main()
{
	int iter, run;

	unsigned char key[32];
	unsigned char key_cmp[32];

	unsigned char pk[ PK_BYTES ];
	unsigned char sk[ SK_BYTES ];

//	unsigned char m[59];
//	unsigned char m_out[ sizeof(m) ];
	unsigned char c[ SYND_BYTES + 32 ];

	//

	for (iter = 0; iter < TEST_RUN; iter++)
	{
		printf("iteration %d\n", iter);

		crypto_kem_keypair(pk, sk);

		for (run = 0; run < TEST_ENCDEC; run++)
		{
//			randombytes(m, sizeof(m));

			crypto_kem_enc(c, key, pk);

			assert(0 == crypto_kem_dec(key_cmp, c, sk));
			assert(0 == memcmp(key, key_cmp, sizeof(key)));
		}


	}

	printf("%d keypairs  + %d x %d en/decap  passed.\n", TEST_RUN , TEST_RUN , TEST_ENCDEC );

	//

	return 0;
}

