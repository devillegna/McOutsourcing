#include "stdio.h"
#include "stdint.h"

static int n_cp = 0;

static inline
void minmax(int i0, int i1 ) {
  n_cp ++;
  printf("   cp_swap(%d,%d)\n", i0,i1 );
}


static inline void int32_sort(int32_t *x,long long n)
{
  long long top,p,q,r,i;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top;p > 0;p >>= 1) {
    printf("p=%d\n",p);
    for (i = 0;i < n - p;++i)
      if (!(i & p))
        //int32_MINMAX(x[i],x[i+p]);
        minmax(i,i+p);
    i = 0;
    for (q = top;q > p;q >>= 1) {
      printf(" q=%d\n",q);
      for (;i < n - q;++i) {
        if (!(i & p)) {
          printf("  c(%d,%d)\n", p , q );
          int32_t a = x[i + p];
          printf("  a_i=[%d]\n", i+p );
          for (r = q;r > p;r >>= 1)
            //int32_MINMAX(a,x[i+r]);
            minmax( -1 , i+r );
          printf("  [%d]<-a\n",i+p);
          x[i + p] = a;
        }
      }
    }
  }
}




#define int16_MINMAX(a,b) \
do { \
  int16_t ab = b ^ a; \
  int16_t c = b - a; \
  c ^= ab & (c ^ b); \
  c >>= 31; \
  c &= ab; \
  a ^= c; \
  b ^= c; \
} while(0)

static inline void int16_sort(int16_t *x,unsigned n)
{ 
  unsigned top,p,q,r,i;

  if (n < 2) return;
  top = 1;
  while (top < n - top) top += top;

  for (p = top;p > 0;p >>= 1) {
    for (i = 0;i < n - p;++i)
      if (!(i & p))
        int16_MINMAX(x[i],x[i+p]);
    i = 0;
    for (q = top;q > p;q >>= 1) {
      for (;i < n - q;++i) {
        if (!(i & p)) {
          int16_t a = x[i + p];
          for (r = q;r > p;r >>= 1)
            int16_MINMAX(a,x[i+r]);
          x[i + p] = a;
        }
      }
    }
  }
}


static void merge_i(int n, int base, int step)
{
  //uint64_t px = (uint64_t)x;
  //uint64_t pb = (uint64_t)base;
  printf("m(%d (%d),%d)\n", n*2 , base , step );
  int i;
  if (n == 1) {
    //minmax(&x[0],&x[step]);
    minmax( 0+base, step+base );
  } else {
    merge_i(n / 2 ,base        ,step * 2);
    merge_i(n / 2 ,base + step ,step * 2);
    printf(" c(%d (%d),%d)\n", n*2 , base , step );
    for (i = 1;i < 2*n-1;i += 2) {
      //minmax(&x[i * step],&x[(i + 1) * step]);
      minmax( i * step + base, (i + 1) * step + base);
    }
  }
}

/* sort x[0],x[1],...,x[n-1] in place */
/* requires n to be a power of 2 */

static void sort_i(int n, int base )
{
  if (n <= 1) return;
  sort_i(n/2, base);
  sort_i(n/2, base + n/2);
  merge_i( n/2, base, 1);
}




////////////////////////////////////


uint8_t cmp_eq(const uint8_t* v1, const uint8_t* v2, int n)
{
  uint8_t r=0;
  for(int i=0;i<n;i++) r |= v1[i]^v2[i];
  return (0==r);
}


#include "sort_bs32.h"


static shuffle(uint16_t *v, int n)
{
  int s = 7;
  for(int i=0;i<n;i++) {
    s *= 2;
    int d = s%n;
    uint16_t t = v[i];
    v[i] = v[d];
    v[d] = t;
  }
}


static
void dump_v(uint16_t *v,int n)
{
  for(int i=0;i<n;i++) {
    printf("%d,",v[i]);
    if(3==(i%4)) printf(" ");
    if(31==(i%32)) printf("\n");
  }
  printf("\n");
}



///////////////////////////////////////


#include "util.h"

static
void test_256()
{
#define N 256
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );

  int w = 8;
  int l_bs = 8;
  uint32_t x_bs[w*8];
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( x_bs+i*w   , x+i*32    , w );
  }
  sort_bs32( x_bs , w , n );
  uint16_t y[N];
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , x_bs+i*w   , w );
  }
  dump_v( y , n );

  int pass = 1;
  for(int i=0;i<n;i++) {
    if( y[i] != i ) pass = 0;
  }
  printf((pass)?"TEST PASSED.\n":"TEST FAILED.\n");
#undef N
}


static
void test_128()
{
#define N 128
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );

  int w = 7;
  int l_bs = 4;
  uint32_t x_bs[w*4];
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( x_bs+i*w   , x+i*32    , w );
  }
  //sort128_bs32( x_bs , w );
  sort_bs32( x_bs , w , n );
  uint16_t y[N];
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , x_bs+i*w   , w );
  }
  dump_v( y , n );
#undef N
}


static
void test_128_payload()
{
#define N 128
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );
  uint16_t y[N];
  for(int i=0;i<n;i++) y[i]=i;
  dump_v( y , n );

  int w = 7;
  int l_bs = 4;
  uint32_t x_bs[w*4];
  uint32_t y_bs[w*4];
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( x_bs+i*w   , x+i*32    , w );
  }
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( y_bs+i*w   , y+i*32    , w );
  }

  sort_bs32_1payload( x_bs , w , n , y_bs );

  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , y_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( x+i*32    , x_bs+i*w   , w );
  }
  dump_v( x , n );
  dump_v( y , n );

  sort_bs32_1payload( y_bs , w , n , x_bs );

  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , y_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( x+i*32    , x_bs+i*w   , w );
  }
  dump_v( x , n );
  dump_v( y , n );

#undef N
}


static
void test_128_payload2()
{
#define N 128
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );
  uint16_t y[N];
  for(int i=0;i<n;i++) y[i]=i;
  dump_v( y , n );
  uint16_t z[N];
  for(int i=0;i<n;i++) z[i]=i;
  dump_v( z , n );

  int w = 7;
  int l_bs = 4;
  uint32_t x_bs[w*4];
  uint32_t y_bs[w*4];
  uint32_t z_bs[w*4];
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( x_bs+i*w   , x+i*32    , w );
  }
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( y_bs+i*w   , y+i*32    , w );
  }
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( z_bs+i*w   , z+i*32    , w );
  }

  sort_bs32_2payload( x_bs , w , n , y_bs , z_bs );

  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , y_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( x+i*32    , x_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( z+i*32    , z_bs+i*w   , w );
  }
  dump_v( x , n );
  dump_v( y , n );
  dump_v( z , n );
  printf(" y == z ? %d\n", cmp_eq(y,z,n*2) );

  memcpy( z_bs , y_bs , sizeof(y_bs) );
  sort_bs32_2payload( y_bs , w , n , x_bs , z_bs );

  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , y_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( x+i*32    , x_bs+i*w   , w );
  }
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( z+i*32    , z_bs+i*w   , w );
  }
  dump_v( x , n );
  dump_v( y , n );
  dump_v( z , n );

#undef N
}




static
void test_64()
{
#define N 64
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );

  int w = 6;
  int l_bs = 2;
  uint32_t x_bs[w*2];
  for(int i=0;i<l_bs;i++) {
    u16x32_to_bs32( x_bs+i*w   , x+i*32    , w );
  }
  //sort64_bs32( x_bs , w );
  sort_bs32( x_bs , w , n );
  uint16_t y[N];
  for(int i=0;i<l_bs;i++) {
    bs32_to_u16x32( y+i*32    , x_bs+i*w   , w );
  }
  dump_v( y , n );
#undef N
}


static
void test_32()
{
#define N 32
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  for(int i=0;i<n;i++) { printf("%d,",x[i]); if(3==(i%4)) printf(" "); } printf("\n");

  int w = 5;
  uint32_t x_bs[w];
  u16x32_to_bs32( x_bs , x , w );
  sort32_bs32( x_bs , w );
  uint16_t y[N];
  bs32_to_u16x32( y , x_bs , w );
  for(int i=0;i<n;i++) { printf("%d,",y[i]); if(3==(i%4)) printf(" "); } printf("\n");
#undef N
}

static
void test_32_payload()
{
#define N 32
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );
  uint16_t y[N];
  for(int i=0;i<n;i++) y[i]=i;
  dump_v( y , n );

  int w = 5;
  uint32_t x_bs[w];
  uint32_t y_bs[w];
  u16x32_to_bs32( x_bs , x , w );
  u16x32_to_bs32( y_bs , y , w );

  sort_bs32_1payload( x_bs , w , n , y_bs );

  bs32_to_u16x32( x , x_bs , w );
  bs32_to_u16x32( y , y_bs , w );
  dump_v( x , n );
  dump_v( y , n );

  sort_bs32_1payload( y_bs , w , n , x_bs );

  bs32_to_u16x32( x , x_bs , w );
  bs32_to_u16x32( y , y_bs , w );
  dump_v( x , n );
  dump_v( y , n );

#undef N
}

void test_32_payload2()
{
#define N 32
  int n = N;
  uint16_t x[N];
  for(int i=0;i<n;i++) x[i]=i;
  //for(int i=0;i<n;i++) x[i]=(N-1-i);
  shuffle( x , n );
  dump_v( x , n );
  uint16_t y[N];
  for(int i=0;i<n;i++) y[i]=i;
  dump_v( y , n );
  uint16_t z[N];
  for(int i=0;i<n;i++) z[i]=i;
  dump_v( z , n );


  int w = 5;
  uint32_t x_bs[w];
  uint32_t y_bs[w];
  uint32_t z_bs[w];
  u16x32_to_bs32( x_bs , x , w );
  u16x32_to_bs32( y_bs , y , w );
  u16x32_to_bs32( z_bs , z , w );

  sort_bs32_2payload( x_bs , w , n , y_bs , z_bs );

  bs32_to_u16x32( x , x_bs , w );
  bs32_to_u16x32( y , y_bs , w );
  bs32_to_u16x32( z , z_bs , w );
  dump_v( x , n );
  dump_v( y , n );
  dump_v( z , n );
  printf(" y == z ? %d\n", cmp_eq(y,z,n*2) );

  memcpy( z_bs , x_bs , sizeof(x_bs) );
  sort_bs32_1payload( y_bs , w , n , x_bs );

  bs32_to_u16x32( x , x_bs , w );
  bs32_to_u16x32( y , y_bs , w );
  bs32_to_u16x32( z , z_bs , w );
  dump_v( x , n );
  dump_v( y , n );
  dump_v( z , n );

#undef N
}


static
void dump_sort()
{
#define N 32
  int n = N;
  uint32_t x[N] = {0};
  printf("sort %d element:\n", n );

  printf("method 1:\n");
  n_cp = 0;
  int32_sort( x , n );
  printf("#cp = %d\n", n_cp );

  printf("\n\n");

  printf("method 2:\n");
  n_cp = 0;
  sort_i( n , 0);
  printf("#cp = %d\n", n_cp );
#undef N
}




int main()
{
  //test_256();
  //test_128_payload();
  test_128_payload2();

  return 0;

}
