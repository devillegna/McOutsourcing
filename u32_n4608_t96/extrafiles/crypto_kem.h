
#ifndef _CRYPTO_KEM_H_
#define _CRYPTO_KEM_H_

int crypto_kem_enc(
       unsigned char *c,
       unsigned char *key,
       const unsigned char *pk );


int crypto_kem_dec(
       unsigned char *key,
       const unsigned char *c,
       const unsigned char *sk );


int crypto_kem_keypair(
       unsigned char *pk,
       unsigned char *sk );






#endif  //ndef _CRYPTO_KEM_H_
