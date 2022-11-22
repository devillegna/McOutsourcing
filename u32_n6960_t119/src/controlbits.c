/* This file is for implementing the Nassimi-Sahni algorithm */
/* See David Nassimi, Sartaj Sahni "Parallel algorithms to set up the Benes permutationnetwork" */
/* See also https://cr.yp.to/papers/controlbits-20200923.pdf */

#include <string.h>
#include "controlbits.h"
#include "sort_int32.h"
typedef int16_t int16;
typedef int32_t int32;







/* parameters: 1 <= w <= 10; n = 2^w */
/* input: permutation pi of {0,1,...,n-1} */
/* output: (2m-1)n/2 control bits at positions pos,pos+step,... */
/* output position pos is by definition 1&(out[pos/8]>>(pos&7)) */
/* caller must 0-initialize positions first */
/* temp must have space for int32[2*n] */
static void cbrecursion(unsigned char *out,long long pos,long long step,const int16 *pi,long long w,long long n,int32 *temp)
{
#define A temp
#define B (temp+n)
#define q ((int16 *) (temp+n+n/4))
/* q can start anywhere between temp+n and temp+n/2 */

  long long x,i,j,k;

  if (w == 1) {
    out[pos>>3] ^= pi[0]<<(pos&7);
    return;
  }

  for (x = 0;x < n;++x) A[x] = ((pi[x]^1)<<16)|pi[x^1];
  int32_sort(A,n); /* A = (id<<16)+pibar */

  for (x = 0;x < n;++x) {
    int32 Ax = A[x];
    int32 px = Ax&0xffff;
    int32 cx = px;
    if (x < cx) cx = x;
    B[x] = (px<<16)|cx;
  }
  /* B = (p<<16)+c */

  for (x = 0;x < n;++x) A[x] = (A[x]<<16)|x; /* A = (pibar<<16)+id */
  int32_sort(A,n); /* A = (id<<16)+pibar^-1 */

  for (x = 0;x < n;++x) A[x] = (A[x]<<16)+(B[x]>>16); /* A = (pibar^(-1)<<16)+pibar */
  int32_sort(A,n); /* A = (id<<16)+pibar^2 */

  //if (w <= 10)
  {
    for (x = 0;x < n;++x) B[x] = ((A[x]&0xffff)<<10)|(B[x]&0x3ff);

    for (i = 1;i < w-1;++i) {
      /* B = (p<<10)+c */

      for (x = 0;x < n;++x) A[x] = ((B[x]&~0x3ff)<<6)|x; /* A = (p<<16)+id */
      int32_sort(A,n); /* A = (id<<16)+p^{-1} */

      for (x = 0;x < n;++x) A[x] = (A[x]<<20)|B[x]; /* A = (p^{-1}<<20)+(p<<10)+c */
      int32_sort(A,n); /* A = (id<<20)+(pp<<10)+cp */

      for (x = 0;x < n;++x) {
        int32 ppcpx = A[x]&0xfffff;
        int32 ppcx = (A[x]&0xffc00)|(B[x]&0x3ff);
        if (ppcpx < ppcx) ppcx = ppcpx;
        B[x] = ppcx;
      }
    }
    for (x = 0;x < n;++x) B[x] &= 0x3ff;
  }

  for (x = 0;x < n;++x) A[x] = (((int32)pi[x])<<16)+x;
  int32_sort(A,n); /* A = (id<<16)+pi^(-1) */

  for (j = 0;j < n/2;++j) {
    long long x = 2*j;
    int32 fj = B[x]&1; /* f[j] */
    int32 Fx = x+fj; /* F[x] */
    int32 Fx1 = Fx^1; /* F[x+1] */

    out[pos>>3] ^= fj<<(pos&7);
    pos += step;

    B[x] = (A[x]<<16)|Fx;
    B[x+1] = (A[x+1]<<16)|Fx1;
  }
  /* B = (pi^(-1)<<16)+F */

  int32_sort(B,n); /* B = (id<<16)+F(pi) */

  pos += (2*w-3)*step*(n/2);

  for (k = 0;k < n/2;++k) {
    long long y = 2*k;
    int32 lk = B[y]&1; /* l[k] */
    int32 Ly = y+lk; /* L[y] */
    int32 Ly1 = Ly^1; /* L[y+1] */

    out[pos>>3] ^= lk<<(pos&7);
    pos += step;

    A[y] = (Ly<<16)|(B[y]&0xffff);
    A[y+1] = (Ly1<<16)|(B[y+1]&0xffff);
  }
  /* A = (L<<16)+F(pi) */

  int32_sort(A,n); /* A = (id<<16)+F(pi(L)) = (id<<16)+M */

  pos -= (2*w-2)*step*(n/2);

  for (j = 0;j < n/2;++j) {
    q[j] = (A[2*j]&0xffff)>>1;
    q[j+n/2] = (A[2*j+1]&0xffff)>>1;
  }

  cbrecursion(out,pos,step*2,q,w-1,n/2,temp);
  cbrecursion(out,pos+step,step*2,q+n/2,w-1,n/2,temp);
}
#undef A
#undef B
#undef q


#include "util.h"




static inline
uint32_t __reverse_bits2(uint32_t a)
{
  uint32_t a0 = a&0x55555555;
  uint32_t a1 = (a>>1)&0x55555555;
  return a1|(a0<<1);
}

static void __cpy( uint32_t *b , const uint32_t *a, unsigned w, unsigned n )
{
  for(unsigned i=0; i<(n/32)*w ; i++) { b[i]=a[i]; }
}

static void __min( uint32_t *c , const uint32_t * _c0 , const uint32_t * _c1 , unsigned w , unsigned n )
{
  for(unsigned i=0;i<n/32;i++) {
    uint32_t borrow = (_c1[0]&~_c0[0]);
    for(unsigned j=1;j<w;j++) { borrow = (borrow&~(_c0[j]^_c1[j])) | (_c1[j]&~_c0[j]); }
    for(unsigned j=0;j<w;j++) { c[j]   = (_c0[j]&borrow) | (_c1[j]&~borrow);  }
    c += w;
    _c0 += w;
    _c1 += w;
  }
}


static uint32_t __combine( uint32_t l, uint32_t h)
{
  uint32_t l0 = l&~0x11111111;
  uint32_t h0 = h&~0x11111111;
  l = (l&0x11111111) ^ (l0>>1);
  h = (h&0x11111111) ^ (h0>>1);

  l0 = l&~0x03030303;
  h0 = h&~0x03030303;
  l = (l&0x03030303) ^ (l0>>2);
  h = (h&0x03030303) ^ (h0>>2);

  l0 = l&~0x000f000f;
  h0 = h&~0x000f000f;
  l = (l&0x000f000f) ^ (l0>>4);
  h = (h&0x000f000f) ^ (h0>>4);

  l0 = l&~0xff;
  h0 = h&~0xff;
  l = (l&0xff) ^ (l0>>8);
  h = (h&0xff) ^ (h0>>8);

  return l^(h<<16);
}

#include "sort_bs32.h"


/* parameters: 6 <= w <= 14; n = 2^w */
/* input: permutation pi of {0,1,...,n-1} */
/* output: (2m-1)n/2 control bits at positions pos,pos+step,... */
/* output position pos is by definition 1&(out[pos/8]>>(pos&7)) */
/* caller must 0-initialize positions first */
/* temp must have space for int32[2*n] */
static void cbrecursion_bs(unsigned char *out,long long pos,long long step,const uint32_t *pi_bs,unsigned w,unsigned n,uint32_t *temp)
{
#define t2 temp
#define t0 (temp+(n>>5)*w)
#define t1 (temp+(n>>5)*w*2)
#define c  (temp+(n>>5)*w*3)
#define q_bs  (temp+(n>>5)*w*2)
/* q can start anywhere between temp+n and temp+n/2 */

  unsigned i,j,k;

  //for (x = 0;x < n;++x) A[x] = ((pi[x]^1)<<16)|pi[x^1];
  //int32_sort(A,n); /* A = (id<<16)+pibar */

  __cpy( t0 , pi_bs , w , n );
  for(i=0;i<n/32;i++) { t0[i*w] ^= 0xffffffff; }  // pi[x]^1
  //for(i=0;i<n/32;i++) { for(j=0;j<w;j++) t1[i*w+j] = __reverse_bits2(pi_bs[i*w+j]); }
  for(i=0;i<(n/32)*w;i++) { t1[i] = __reverse_bits2(pi_bs[i]); }   // pi[x^1]
  //__sort_1( t0 , t1 ,w , n );  // t0 = 0,1,2,3....     t1: pibar
  sort_bs32_1payload( t0 ,w , n , t1 );  // t0 = 0,1,2,3....     t1: pibar

  __min( c , t0 , t1 , w , n );

  //for (x = 0;x < n;++x) A[x] = (A[x]<<16)|x; /* A = (pibar<<16)+id */
  //int32_sort(A,n); /* A = (id<<16)+pibar^-1 */
  __cpy( t2 , t1 , w , n );
  //for(i=0;i<n/32;i++) { for(j=0;j<w;j++) t2[i*w+j] = t1[i*w+j]; }
  //__sort_1( t2 , t0 , w , n );  //  t2 = 0,1,2,3...    t0: pibar^-1
  sort_bs32_1payload( t2 , w , n , t0 );  //  t2 = 0,1,2,3...    t0: pibar^-1


  //for (x = 0;x < n;++x) A[x] = (A[x]<<16)+(B[x]>>16); /* A = (pibar^(-1)<<16)+pibar */
  //int32_sort(A,n); /* A = (id<<16)+pibar^2 */
  //__sort_1( t0 , t1 , w , n );  //  t0=0,1,2,3...  t1:pibar^2
  sort_bs32_1payload( t0 , w , n , t1 );  //  t0=0,1,2,3...  t1:pibar^2

  {
    //for (x = 0;x < n;++x) B[x] = ((A[x]&0xffff)<<10)|(B[x]&0x3ff);
    // reserved data:  c  and  t1: pibar^2

    for (i = 1;i < w-1;++i) {
      /* B = (p<<10)+c */

      //for (x = 0;x < n;++x) A[x] = ((B[x]&~0x3ff)<<6)|x; /* A = (p<<16)+id */
      //int32_sort(A,n); /* A = (id<<16)+p^{-1} */

      __cpy( t2 , t1 , w , n );
      //__sort_1( t2 , t0 , w , n ); // t2=0,1,2,3...  t0: p^-1
      sort_bs32_1payload( t2 , w , n , t0 ); // t2=0,1,2,3...  t0: p^-1


      //for (x = 0;x < n;++x) A[x] = (A[x]<<20)|B[x]; /* A = (p^{-1}<<20)+(p<<10)+c */
      //int32_sort(A,n); /* A = (id<<20)+(pp<<10)+cp */
      __cpy( t2 , c , w , n );
      //__sort_2( t0 , t1 , t2 , w , n );  // t0=0,1,2,3...   t1:pp  , t2:cp
      sort_bs32_2payload( t0 , w , n , t1 , t2 );  // t0=0,1,2,3...   t1:pp  , t2:cp

      __min( c , c , t2 , w , n );
    }
  }

  //for (x = 0;x < n;++x) A[x] = (((int32)pi[x])<<16)+x;
  //int32_sort(A,n); /* A = (id<<16)+pi^(-1) */
  __cpy( t2 , pi_bs , w , n );
  //__sort_1( t2 , t0 , w , n );  // t2=0,1,2,3...  t0: pi^-1
  sort_bs32_1payload( t2 , w , n , t0 );  // t2=0,1,2,3...  t0: pi^-1

  for(i=0;i<(n/32);i++) {
    uint32_t c_odd  = c[i*w]     &0x55555555;  // even terms
    uint32_t c_even = (c_odd<<1) ^0xaaaaaaaa;
    t2[i*w] = c_even^c_odd;

    for(j=0;j<32;j+=2) {
      uint32_t fj = (c_odd>>j)&1;
      out[pos>>3] ^= fj<<(pos&7);
      pos += step;
    }
  }  // release c
  //__sort_1( t0 , t2 , w , n );  // t0=0,1,2,3...  t2:F(pi)
  sort_bs32_1payload( t0 , w , n , t2 );  // t0=0,1,2,3...  t2:F(pi)

  pos += (2*w-3)*step*(n/2);

  for(i=0;i<(n/32);i++) {
    uint32_t l_odd  = t2[i*w]    &0x55555555;  // even terms
    uint32_t l_even = (l_odd<<1) ^0xaaaaaaaa;
    t0[i*w] = l_even^l_odd;

    for(j=0;j<32;j+=2) {
      uint32_t lj = (l_odd>>j)&1;
      out[pos>>3] ^= lj<<(pos&7);
      pos += step;
    }
  }  // release c
  //__sort_1( t0 , t2 , w , n );  // t0=0,1,2,3...  t2:F(pi(L)) = M
  sort_bs32_1payload( t0 , w , n , t2 );  // t0=0,1,2,3...  t2:F(pi(L)) = M

  pos -= (2*w-2)*step*(n/2);

  if(w>=7) {
    for(j=0;j<(n/2);j+=32) {
      for(k=0;k<w-1;k++) {
        q_bs[(j>>5)*(w-1)+k]              = __combine( t2[2*(j>>5)*w+k+1]     &0x55555555  , t2[(2*(j>>5)+1)*w+k+1]     &0x55555555 );
        q_bs[(j>>5)*(w-1)+k+(n/64)*(w-1)] = __combine( (t2[2*(j>>5)*w+k+1]>>1)&0x55555555  , (t2[(2*(j>>5)+1)*w+k+1]>>1)&0x55555555 );
      }
    }
    cbrecursion_bs(out,pos,step*2,q_bs,w-1,n/2,temp);
    cbrecursion_bs(out,pos+step,step*2,q_bs+(n/64)*(w-1),w-1,n/2,temp);
  } else {
    uint16_t qq[n];  // use stack

    for (j = 0;j < n/2;++j) {
      uint16_t aa[32];
      if( 0 == (j&15) ) { bs32_to_u16x32( aa , t2 + ((j*2)>>5)*w+1 , w-1 ); }
      qq[j]     = aa[(j&15)*2];
      qq[j+n/2] = aa[(j&15)*2+1];
    }
    cbrecursion(out,pos,step*2,qq,w-1,n/2,temp);
    cbrecursion(out,pos+step,step*2,qq+n/2,w-1,n/2,temp);
  }
}
#undef t0
#undef t1
#undef t2
#undef c
#undef q_bs









/* parameters: 1 <= w <= 14; n = 2^w */
/* input: permutation pi of {0,1,...,n-1} */
/* output: (2m-1)n/2 control bits at positions 0,1,... */
/* output position pos is by definition 1&(out[pos/8]>>(pos&7)) */
void controlbitsfrompermutation(unsigned char *out,const uint32_t *pi_bs,long long w,long long n, uint32_t *buffer_2n)
{
  //int32 temp[2*n];
  uint32_t *temp = buffer_2n;
  //int16_t pi[n];
  //bs32_to_u16v( pi , pi_bs , w , n );

  memset(out,0,(((2*w-1)*n/2)+7)/8);
  cbrecursion_bs(out,0,1,pi_bs,w,n,temp);
}




int permutationfromweight(uint32_t* pi_bs, uint32_t * weight,unsigned w, long long n, uint16_t *buffer_n)
{
    uint16_t *pi = buffer_n;
    for (unsigned i = 0; i < n; i++) { pi[i] = i; }
    uint32_sort_u16payload(weight, pi,n);
    for (int i = 1; i < n;i++) { if(weight[i-1]==weight[i]) { return -1; } }
    for (int i=0;i<n;i+=32) u16x32_to_bs32( pi_bs + (i>>5)*w , pi + i , w );
    return 0;
}


