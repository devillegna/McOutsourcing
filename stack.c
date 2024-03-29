#include "api.h"
#include "randombytes.h"
#include "hal.h"
#include "sendfn.h"

#include <string.h>

#ifndef MAX_STACK_SIZE
#define MAX_STACK_SIZE hal_get_stack_size()
#endif

#ifndef STACK_SIZE_INCR
#define STACK_SIZE_INCR 0x1000
#endif

// https://stackoverflow.com/a/1489985/1711232
#define PASTER(x, y) x##y
#define EVALUATOR(x, y) PASTER(x, y)
#define NAMESPACE(fun) EVALUATOR(MUPQ_NAMESPACE, fun)

// use different names so we can have empty namespaces
#define MUPQ_CRYPTO_BYTES           NAMESPACE(CRYPTO_BYTES)
#define MUPQ_CRYPTO_PUBLICKEYBYTES  NAMESPACE(CRYPTO_PUBLICKEYBYTES)
#define MUPQ_CRYPTO_SECRETKEYBYTES  NAMESPACE(CRYPTO_SECRETKEYBYTES)
#define MUPQ_CRYPTO_CIPHERTEXTBYTES NAMESPACE(CRYPTO_CIPHERTEXTBYTES)
#define MUPQ_CRYPTO_ALGNAME NAMESPACE(CRYPTO_ALGNAME)

#define MUPQ_crypto_kem_keypair NAMESPACE(crypto_kem_keypair)
#define MUPQ_crypto_kem_enc NAMESPACE(crypto_kem_enc)
#define MUPQ_crypto_kem_dec NAMESPACE(crypto_kem_dec)

#define send_stack_usage(S, U) send_unsigned((S), (U))

unsigned int canary_size;
volatile unsigned char *p;
unsigned int c;
uint8_t canary = 0x42;



#if defined(STM32F407VG)  //write key pair into CCM
unsigned char * sk_a  = 0x10000000 + 8192;
unsigned char * pk    = 0x10000000 + 8192 + MUPQ_CRYPTO_SECRETKEYBYTES;
#else
unsigned char sk_a[MUPQ_CRYPTO_SECRETKEYBYTES];
unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES];
#endif
unsigned char sendb[MUPQ_CRYPTO_CIPHERTEXTBYTES];
unsigned int stack_key_gen, stack_encaps, stack_decaps;
unsigned char key_a[MUPQ_CRYPTO_BYTES], key_b[MUPQ_CRYPTO_BYTES];

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
    send_stack_usage("keypair stack usage:", stack_key_gen);
    send_stack_usage("encaps stack usage:", stack_encaps);
    send_stack_usage("decaps stack usage:", stack_decaps);
    hal_send_str("OK KEYS\n");
    return 0;
  }
}

int main(void) {
  hal_setup(CLOCK_FAST);

  // marker for automated benchmarks  
  hal_send_str("==========================");
  //canary_size = STACK_SIZE_INCR;
  canary_size=110000;

  while(test_keys()){
    if(canary_size == MAX_STACK_SIZE) {
      hal_send_str("failed to measure stack usage.\n");
      break;
    }
    canary_size += STACK_SIZE_INCR;
    if(canary_size >= MAX_STACK_SIZE) {
      canary_size = MAX_STACK_SIZE;
    }
  }
  // marker for automated benchmarks
  hal_send_str("#");

  return 0;
}
