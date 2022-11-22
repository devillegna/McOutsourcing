
#include "api.h"
#include "randombytes.h"
#include "crypto_kem.h"
#include "stdio.h"

#include <string.h>


#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE (128*1024)
#endif

#ifndef STACK_SIZE_INCR
#define STACK_SIZE_INCR (16*1024)
#endif



// use different names so we can have empty namespaces
#define MUPQ_CRYPTO_BYTES           CRYPTO_BYTES
#define MUPQ_CRYPTO_PUBLICKEYBYTES  CRYPTO_PUBLICKEYBYTES
#define MUPQ_CRYPTO_SECRETKEYBYTES  CRYPTO_SECRETKEYBYTES
#define MUPQ_CRYPTO_CIPHERTEXTBYTES CRYPTO_CIPHERTEXTBYTES
#define MUPQ_CRYPTO_ALGNAME         CRYPTO_ALGNAME

#define MUPQ_crypto_kem_keypair crypto_kem_keypair
#define MUPQ_crypto_kem_enc     crypto_kem_enc
#define MUPQ_crypto_kem_dec     crypto_kem_dec


unsigned int canary_size;
volatile unsigned char *p;
unsigned int c;
uint8_t canary = 0x42;

unsigned char key_a[MUPQ_CRYPTO_BYTES], key_b[MUPQ_CRYPTO_BYTES];
unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES];
unsigned char sendb[MUPQ_CRYPTO_CIPHERTEXTBYTES];
unsigned char sk_a[MUPQ_CRYPTO_SECRETKEYBYTES];
unsigned int stack_key_gen, stack_encaps, stack_decaps;

#define FILL_STACK()                                                           \
  p = &a;                                                                      \
  while (p > &a - canary_size)                                                 \
    *(p--) = canary;
#define CHECK_STACK()                                                          \
  c = canary_size;                                                             \
  p = &a - canary_size + 1;                                                    \
  while (*p == canary && p < &a) {                                             \
    p++;                                                                       \
    c--;                                                                       \
  }





static int test_keys(void) {
  volatile unsigned char a;
  // Alice generates a public key
  FILL_STACK()
  MUPQ_crypto_kem_keypair(pk, sk_a);
  CHECK_STACK()
  if(c >= canary_size) return -1;
  stack_key_gen = c;

  // Bob derives a secret key and creates a response
  FILL_STACK()
  MUPQ_crypto_kem_enc(sendb, key_b, pk);
  CHECK_STACK()
  if(c >= canary_size) return -1;
  stack_encaps = c;

  // Alice uses Bobs response to get her secret key
  FILL_STACK()
  MUPQ_crypto_kem_dec(key_a, sendb, sk_a);
  CHECK_STACK()
  if(c >= canary_size) return -1;
  stack_decaps = c;

  if (memcmp(key_a, key_b, MUPQ_CRYPTO_BYTES)){
    return -1;
  } else {
    printf("keypair stack usage: %d\n", stack_key_gen);
    printf("encaps stack usage: %d\n", stack_encaps);
    printf("decaps stack usage: %d\n", stack_decaps);
    puts("OK KEYS\n");
    return 0;
  }
}





int main(void) {

  // marker for automated benchmarks
  puts("==========================");
  canary_size = STACK_SIZE_INCR;
  while(test_keys()){
    if(canary_size == MAX_STACK_SIZE) {
      puts("failed to measure stack usage.\n");
      break;
    }
    canary_size += STACK_SIZE_INCR;
    if(canary_size >= MAX_STACK_SIZE) {
      canary_size = MAX_STACK_SIZE;
    }
  }
  // marker for automated benchmarks
  puts("#");

  return 0;
}
