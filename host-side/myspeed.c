#include "api.h"
#include "hal.h"
#include "sendfn.h"

#include <stdint.h>
#include <string.h>

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


static inline
void print_num(unsigned long long n)
{
  unsigned char msg[64];
  msg[0] = ':';
  msg[1] = ':';
  int j = 0;
  for(int i=44;i>=0;i-=4) {
    unsigned long long m = 0xf;
    m <<= i;
    m &= n;
    m >>= i;
    if( m >= 10 ) m += 7;
    msg[2+j] = '0'+m;
    j++;
  }
  msg[2+j] = '\0';
  hal_send_str(msg);
}


int main(void)
{
  unsigned char key_a[MUPQ_CRYPTO_BYTES], key_b[MUPQ_CRYPTO_BYTES];
  unsigned char sk[MUPQ_CRYPTO_SECRETKEYBYTES];
  unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES];
  unsigned char ct[MUPQ_CRYPTO_CIPHERTEXTBYTES];
  unsigned long long t0, t1;
  unsigned long long t_k, t_e, t_d;
  int i;

  //hal_setup(CLOCK_BENCHMARK);
  hal_setup(CLOCK_FAST);

  hal_send_str("::==========================");

  for(i=0;i<MUPQ_ITERATIONS; i++)
  {
    // Key-pair generation
    t0 = hal_get_time();
    MUPQ_crypto_kem_keypair(pk, sk);
    t1 = hal_get_time();
    //printcycles("keypair cycles:", t1-t0);
    t_k = t1-t0;
    hal_send_str("::keypair cycles:");

    // Encapsulation
    t0 = hal_get_time();
    MUPQ_crypto_kem_enc(ct, key_a, pk);
    t1 = hal_get_time();
    //printcycles("encaps cycles:", t1-t0);
    t_e = t1-t0;
    hal_send_str("::encaps cycles:");

    // Decapsulation
    t0 = hal_get_time();
    MUPQ_crypto_kem_dec(key_b, ct, sk);
    t1 = hal_get_time();
    //printcycles("decaps cycles:", t1-t0);
    t_d = t1-t0;
    hal_send_str("::decaps cycles:");

    if (memcmp(key_a, key_b, MUPQ_CRYPTO_BYTES)) {
      hal_send_str("::ERROR KEYS");
    }
    else {
      hal_send_str("::OK KEYS");
    }
    hal_send_str("::keypair cycles:");
    print_num(t_k);
    hal_send_str("::encaps cycles:");
    print_num(t_e);
    hal_send_str("::decaps cycles:");
    print_num(t_d);
  }

  hal_send_str("::#");

  return 0;
}
