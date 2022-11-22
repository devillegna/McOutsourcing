
#include "sort_bs32.h"



static inline
uint32_t __cmp_ge_s( const uint32_t * a, int sr_b, int w )  // (a>=b)?
{  // a - b
  //const uint32_t *b = a;
#define b a
  uint32_t borrow = ((b[0]>>sr_b)&~a[0]);
  for(int j=1;j<w;j++) { borrow = (borrow&~(a[j]^(b[j]>>sr_b))) | ((b[j]>>sr_b)&~a[j]); }
  return ~borrow;
#undef b
}

static inline
void __cswap_s( uint32_t * a, int sr_b , uint32_t condition , int w )
{
//  const uint32_t *b = a;
#define b a
  for(int i=0;i<w;i++) {
    uint32_t ab = (a[i]^(b[i]>>sr_b)) & condition;
    a[i] ^= ab;
    b[i] ^= (ab<<sr_b);
  }
#undef b
}



// a: |31--24|23--8b(B)--16|15--8b(A)--8|7---0|
// minmax( A,B )
static void __minmax_last_8bits( uint32_t *bs , int w )
{
  uint32_t mask_1 = 0x0000ff00;
  uint32_t mask_swap1 = __cmp_ge_s( bs  , 8 , w );
  __cswap_s( bs , 8 , mask_swap1&mask_1 , w );
}
static void __minmax_last_4bits( uint32_t *bs, int w)
{
  uint32_t mask_322 = 0x00f0f0f0;
  uint32_t mask_swap322 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap322&mask_322 , w );
}
static void __minmax_last_2bits( uint32_t *bs, int w)
{
  uint32_t mask_323 = 0x0ccccccc;
  uint32_t mask_swap323 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap323&mask_323 , w );
}
static void __minmax_last_1bit( uint32_t *bs, int w)
{
  uint32_t mask_324 = 0x2aaaaaaa;
  uint32_t mask_swap324 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap324&mask_324 , w );
}

static void __minmax_last_8bits_1pl( uint32_t *bs , int w , uint32_t *pl )
{
  uint32_t mask_1 = 0x0000ff00;
  uint32_t mask_swap1 = __cmp_ge_s( bs  , 8 , w );
  __cswap_s( bs , 8 , mask_swap1&mask_1 , w );
  __cswap_s( pl , 8 , mask_swap1&mask_1 , w );
}
static void __minmax_last_4bits_1pl( uint32_t *bs, int w , uint32_t *pl )
{
  uint32_t mask_322 = 0x00f0f0f0;
  uint32_t mask_swap322 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap322&mask_322 , w );
  __cswap_s( pl , 4 , mask_swap322&mask_322 , w );
}
static void __minmax_last_2bits_1pl( uint32_t *bs, int w , uint32_t *pl )
{
  uint32_t mask_323 = 0x0ccccccc;
  uint32_t mask_swap323 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap323&mask_323 , w );
  __cswap_s( pl , 2 , mask_swap323&mask_323 , w );
}
static void __minmax_last_1bit_1pl( uint32_t *bs, int w , uint32_t *pl )
{
  uint32_t mask_324 = 0x2aaaaaaa;
  uint32_t mask_swap324 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap324&mask_324 , w );
  __cswap_s( pl , 1 , mask_swap324&mask_324 , w );
}

static void __minmax_last_8bits_2pl( uint32_t *bs , int w , uint32_t *pl1 , uint32_t *pl2 )
{
  uint32_t mask_1 = 0x0000ff00;
  uint32_t mask_swap1 = __cmp_ge_s( bs  , 8 , w );
  __cswap_s( bs , 8 , mask_swap1&mask_1 , w );
  __cswap_s( pl1 , 8 , mask_swap1&mask_1 , w );
  __cswap_s( pl2 , 8 , mask_swap1&mask_1 , w );
}
static void __minmax_last_4bits_2pl( uint32_t *bs, int w , uint32_t *pl1 , uint32_t *pl2 )
{
  uint32_t mask_322 = 0x00f0f0f0;
  uint32_t mask_swap322 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap322&mask_322 , w );
  __cswap_s( pl1 , 4 , mask_swap322&mask_322 , w );
  __cswap_s( pl2 , 4 , mask_swap322&mask_322 , w );
}
static void __minmax_last_2bits_2pl( uint32_t *bs, int w , uint32_t *pl1 , uint32_t *pl2 )
{
  uint32_t mask_323 = 0x0ccccccc;
  uint32_t mask_swap323 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap323&mask_323 , w );
  __cswap_s( pl1 , 2 , mask_swap323&mask_323 , w );
  __cswap_s( pl2 , 2 , mask_swap323&mask_323 , w );
}
static void __minmax_last_1bit_2pl( uint32_t *bs, int w , uint32_t *pl1 , uint32_t *pl2 )
{
  uint32_t mask_324 = 0x2aaaaaaa;
  uint32_t mask_swap324 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap324&mask_324 , w );
  __cswap_s( pl1 , 1 , mask_swap324&mask_324 , w );
  __cswap_s( pl2 , 1 , mask_swap324&mask_324 , w );
}




static
void sort32_bs32( uint32_t * bs , int w )
{
  // sort2
  uint32_t mask_2 = 0x55555555;
  uint32_t mask_swap2 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap2&mask_2 , w );

  // sort4
    // size 4, swap(0,2)(1,3)
  uint32_t mask_40 = 0x33333333;
  uint32_t mask_swap40 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap40&mask_40 , w );
    // size 4, swap(2,3)
  uint32_t mask_41 = 0x22222222;
  uint32_t mask_swap41 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap41&mask_41 , w );

  // sort8
    // size 8, swap(0,4)(1,5),(2,6),(3,7)
  uint32_t mask_80 = 0x0f0f0f0f;
  uint32_t mask_swap80 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap80&mask_80 , w );
    // size 8, swap(2,4)(3,5)
  uint32_t mask_81 = 0x0c0c0c0c;
  uint32_t mask_swap81 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap81&mask_81 , w );
    // size 8, swap(1,2)(3,4)(5,6)
  uint32_t mask_82 = 0x2a2a2a2a;
  uint32_t mask_swap82 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap82&mask_82 , w );

  // sort16
    // size 16, swap(0,8)(1,9),(2,10),..(7,15)
  uint32_t mask_160 = 0x00ff00ff;
  uint32_t mask_swap160 = __cmp_ge_s( bs , 8 , w );
  __cswap_s( bs , 8 , mask_swap160&mask_160 , w );
    // size 16, swap(4,8)(5,9),(6,10),(7,11)
  uint32_t mask_161 = 0x00f000f0;
  uint32_t mask_swap161 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap161&mask_161 , w );
    // size 16, swap(2,4)(6,8)(10,12)  (3,5)(7,9)(11,13)
  uint32_t mask_162 = 0x0ccc0ccc;
  uint32_t mask_swap162 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap162&mask_162 , w );
    // size 16, swap(1,2)(3,4)(5,6),...(13,14)
  uint32_t mask_163 = 0x2aaa2aaa;
  uint32_t mask_swap163 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap163&mask_163 , w );

  // sort32
  // size 32, 16-bits segments.   swap(0-15,16-31)
  uint32_t mask_320 = 0x0000ffff;
  uint32_t mask_swap320 = __cmp_ge_s( bs , 16 , w );
  __cswap_s( bs , 16 , mask_swap320&mask_320 , w );

  // size 32,  8-bits x 4.  swap(8-11,12-15) (16-19,20-23)
  __minmax_last_8bits( bs , w );
  // size 32, 4-bits x 8. swap(4-7,8-11) (12-15,16-19) (20-23,24-27)
  __minmax_last_4bits( bs , w );
  // size 32, 2-bits x 16
  __minmax_last_2bits( bs , w );
  // size 32, 1-bit x 32, swap(1,2)(3,4),...(29,30)
   __minmax_last_1bit( bs ,  w );
}


static
void sort32_bs32_1pl( uint32_t * bs , int w , uint32_t *pl)
{
  // sort2
  uint32_t mask_2 = 0x55555555;
  uint32_t mask_swap2 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap2&mask_2 , w );
  __cswap_s( pl , 1 , mask_swap2&mask_2 , w );

  // sort4
    // size 4, swap(0,2)(1,3)
  uint32_t mask_40 = 0x33333333;
  uint32_t mask_swap40 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap40&mask_40 , w );
  __cswap_s( pl , 2 , mask_swap40&mask_40 , w );
    // size 4, swap(2,3)
  uint32_t mask_41 = 0x22222222;
  uint32_t mask_swap41 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap41&mask_41 , w );
  __cswap_s( pl , 1 , mask_swap41&mask_41 , w );

  // sort8
    // size 8, swap(0,4)(1,5),(2,6),(3,7)
  uint32_t mask_80 = 0x0f0f0f0f;
  uint32_t mask_swap80 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap80&mask_80 , w );
  __cswap_s( pl , 4 , mask_swap80&mask_80 , w );
    // size 8, swap(2,4)(3,5)
  uint32_t mask_81 = 0x0c0c0c0c;
  uint32_t mask_swap81 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap81&mask_81 , w );
  __cswap_s( pl , 2 , mask_swap81&mask_81 , w );
    // size 8, swap(1,2)(3,4)(5,6)
  uint32_t mask_82 = 0x2a2a2a2a;
  uint32_t mask_swap82 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap82&mask_82 , w );
  __cswap_s( pl , 1 , mask_swap82&mask_82 , w );

  // sort16
    // size 16, swap(0,8)(1,9),(2,10),..(7,15)
  uint32_t mask_160 = 0x00ff00ff;
  uint32_t mask_swap160 = __cmp_ge_s( bs , 8 , w );
  __cswap_s( bs , 8 , mask_swap160&mask_160 , w );
  __cswap_s( pl , 8 , mask_swap160&mask_160 , w );
    // size 16, swap(4,8)(5,9),(6,10),(7,11)
  uint32_t mask_161 = 0x00f000f0;
  uint32_t mask_swap161 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap161&mask_161 , w );
  __cswap_s( pl , 4 , mask_swap161&mask_161 , w );
    // size 16, swap(2,4)(6,8)(10,12)  (3,5)(7,9)(11,13)
  uint32_t mask_162 = 0x0ccc0ccc;
  uint32_t mask_swap162 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap162&mask_162 , w );
  __cswap_s( pl , 2 , mask_swap162&mask_162 , w );
    // size 16, swap(1,2)(3,4)(5,6),...(13,14)
  uint32_t mask_163 = 0x2aaa2aaa;
  uint32_t mask_swap163 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap163&mask_163 , w );
  __cswap_s( pl , 1 , mask_swap163&mask_163 , w );

  // sort32
  // size 32, 16-bits segments.   swap(0-15,16-31)
  uint32_t mask_320 = 0x0000ffff;
  uint32_t mask_swap320 = __cmp_ge_s( bs , 16 , w );
  __cswap_s( bs , 16 , mask_swap320&mask_320 , w );
  __cswap_s( pl , 16 , mask_swap320&mask_320 , w );

  // size 32,  8-bits x 4.  swap(8-11,12-15) (16-19,20-23)
  __minmax_last_8bits_1pl( bs , w , pl );
  // size 32, 4-bits x 8. swap(4-7,8-11) (12-15,16-19) (20-23,24-27)
  __minmax_last_4bits_1pl( bs , w , pl );
  // size 32, 2-bits x 16
  __minmax_last_2bits_1pl( bs , w , pl );
  // size 32, 1-bit x 32, swap(1,2)(3,4),...(29,30)
   __minmax_last_1bit_1pl( bs ,  w , pl );
}



static
void sort32_bs32_2pl( uint32_t * bs , int w , uint32_t *pl1 , uint32_t *pl2 )
{
  // sort2
  uint32_t mask_2 = 0x55555555;
  uint32_t mask_swap2 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap2&mask_2 , w );
  __cswap_s( pl1 , 1 , mask_swap2&mask_2 , w );
  __cswap_s( pl2 , 1 , mask_swap2&mask_2 , w );

  // sort4
    // size 4, swap(0,2)(1,3)
  uint32_t mask_40 = 0x33333333;
  uint32_t mask_swap40 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap40&mask_40 , w );
  __cswap_s( pl1 , 2 , mask_swap40&mask_40 , w );
  __cswap_s( pl2 , 2 , mask_swap40&mask_40 , w );
    // size 4, swap(2,3)
  uint32_t mask_41 = 0x22222222;
  uint32_t mask_swap41 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap41&mask_41 , w );
  __cswap_s( pl1 , 1 , mask_swap41&mask_41 , w );
  __cswap_s( pl2 , 1 , mask_swap41&mask_41 , w );

  // sort8
    // size 8, swap(0,4)(1,5),(2,6),(3,7)
  uint32_t mask_80 = 0x0f0f0f0f;
  uint32_t mask_swap80 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap80&mask_80 , w );
  __cswap_s( pl1 , 4 , mask_swap80&mask_80 , w );
  __cswap_s( pl2 , 4 , mask_swap80&mask_80 , w );
    // size 8, swap(2,4)(3,5)
  uint32_t mask_81 = 0x0c0c0c0c;
  uint32_t mask_swap81 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap81&mask_81 , w );
  __cswap_s( pl1 , 2 , mask_swap81&mask_81 , w );
  __cswap_s( pl2 , 2 , mask_swap81&mask_81 , w );
    // size 8, swap(1,2)(3,4)(5,6)
  uint32_t mask_82 = 0x2a2a2a2a;
  uint32_t mask_swap82 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap82&mask_82 , w );
  __cswap_s( pl1 , 1 , mask_swap82&mask_82 , w );
  __cswap_s( pl2 , 1 , mask_swap82&mask_82 , w );

  // sort16
    // size 16, swap(0,8)(1,9),(2,10),..(7,15)
  uint32_t mask_160 = 0x00ff00ff;
  uint32_t mask_swap160 = __cmp_ge_s( bs , 8 , w );
  __cswap_s( bs , 8 , mask_swap160&mask_160 , w );
  __cswap_s( pl1 , 8 , mask_swap160&mask_160 , w );
  __cswap_s( pl2 , 8 , mask_swap160&mask_160 , w );
    // size 16, swap(4,8)(5,9),(6,10),(7,11)
  uint32_t mask_161 = 0x00f000f0;
  uint32_t mask_swap161 = __cmp_ge_s( bs , 4 , w );
  __cswap_s( bs , 4 , mask_swap161&mask_161 , w );
  __cswap_s( pl1 , 4 , mask_swap161&mask_161 , w );
  __cswap_s( pl2 , 4 , mask_swap161&mask_161 , w );
    // size 16, swap(2,4)(6,8)(10,12)  (3,5)(7,9)(11,13)
  uint32_t mask_162 = 0x0ccc0ccc;
  uint32_t mask_swap162 = __cmp_ge_s( bs , 2 , w );
  __cswap_s( bs , 2 , mask_swap162&mask_162 , w );
  __cswap_s( pl1 , 2 , mask_swap162&mask_162 , w );
  __cswap_s( pl2 , 2 , mask_swap162&mask_162 , w );
    // size 16, swap(1,2)(3,4)(5,6),...(13,14)
  uint32_t mask_163 = 0x2aaa2aaa;
  uint32_t mask_swap163 = __cmp_ge_s( bs , 1 , w );
  __cswap_s( bs , 1 , mask_swap163&mask_163 , w );
  __cswap_s( pl1 , 1 , mask_swap163&mask_163 , w );
  __cswap_s( pl2 , 1 , mask_swap163&mask_163 , w );

  // sort32
  // size 32, 16-bits segments.   swap(0-15,16-31)
  uint32_t mask_320 = 0x0000ffff;
  uint32_t mask_swap320 = __cmp_ge_s( bs , 16 , w );
  __cswap_s( bs , 16 , mask_swap320&mask_320 , w );
  __cswap_s( pl1 , 16 , mask_swap320&mask_320 , w );
  __cswap_s( pl2 , 16 , mask_swap320&mask_320 , w );

  // size 32,  8-bits x 4.  swap(8-11,12-15) (16-19,20-23)
  __minmax_last_8bits_2pl( bs , w , pl1 , pl2 );
  // size 32, 4-bits x 8. swap(4-7,8-11) (12-15,16-19) (20-23,24-27)
  __minmax_last_4bits_2pl( bs , w , pl1 , pl2 );
  // size 32, 2-bits x 16
  __minmax_last_2bits_2pl( bs , w , pl1 , pl2 );
  // size 32, 1-bit x 32, swap(1,2)(3,4),...(29,30)
   __minmax_last_1bit_2pl( bs ,  w , pl1 , pl2 );
}







///////////////////////  XXX  /////////////////////////////////


static inline
uint32_t __cmp_ge_2_s( const uint32_t * a, const uint32_t *b , int sr_b, int w )  // (a>=b)?
{  // a - b
  uint32_t borrow = ((b[0]>>sr_b)&~a[0]);
  for(int j=1;j<w;j++) { borrow = (borrow&~(a[j]^(b[j]>>sr_b))) | ((b[j]>>sr_b)&~a[j]); }
  return ~borrow;
}

static inline
void __cswap_2_s( uint32_t * a, uint32_t *b, int sr_b , uint32_t condition , int w )
{
  for(int i=0;i<w;i++) {
    uint32_t ab = (a[i]^(b[i]>>sr_b)) & condition;
    a[i] ^= ab;
    b[i] ^= (ab<<sr_b);
  }
}

// b: |63---48|47--16b(B)--32|    a: |31--16b(A)--16|15---0|
// minmax( A,B )
static inline
void __minmax_2segs_16bits( uint32_t *a , uint32_t *b, int w )
{
  uint32_t mask_1 = 0x0000ffff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 16 , w );
  __cswap_2_s( b , a , 16 , mask_1&~mask_swap1 , w );
}
// b: |63---40|39--8b(B)--32|    a: |31--8b(A)--24|23---0|
// minmax( A,B )
static inline
void __minmax_2segs_8bits( uint32_t *a , uint32_t *b, int w )
{
  uint32_t mask_1 = 0x000000ff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 24 , w );
  __cswap_2_s( b , a , 24 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_4bits( uint32_t *a , uint32_t *b, int w )
{
  uint32_t mask_1 = 0x0000000f;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 28 , w );
  __cswap_2_s( b , a , 28 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_2bits( uint32_t *a , uint32_t *b, int w )
{
  uint32_t mask_1 = 0x00000003;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 30 , w );
  __cswap_2_s( b , a , 30 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_1bit( uint32_t *a , uint32_t *b, int w )
{
  uint32_t mask_1 = 0x00000001;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 31 , w );
  __cswap_2_s( b , a , 31 , mask_1&~mask_swap1 , w );
}


static inline
void __minmax_2segs_16bits_1pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb )
{
  uint32_t mask_1 = 0x0000ffff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 16 , w );
  __cswap_2_s( b , a , 16 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 16 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_8bits_1pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb )
{
  uint32_t mask_1 = 0x000000ff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 24 , w );
  __cswap_2_s( b , a , 24 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 24 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_4bits_1pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb )
{
  uint32_t mask_1 = 0x0000000f;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 28 , w );
  __cswap_2_s( b , a , 28 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 28 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_2bits_1pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb )
{
  uint32_t mask_1 = 0x00000003;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 30 , w );
  __cswap_2_s( b , a , 30 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 30 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_1bit_1pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb )
{
  uint32_t mask_1 = 0x00000001;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 31 , w );
  __cswap_2_s( b , a , 31 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 31 , mask_1&~mask_swap1 , w );
}


static inline
void __minmax_2segs_16bits_2pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb , uint32_t *pl2a , uint32_t *pl2b )
{
  uint32_t mask_1 = 0x0000ffff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 16 , w );
  __cswap_2_s( b , a , 16 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 16 , mask_1&~mask_swap1 , w );
  __cswap_2_s( pl2b , pl2a , 16 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_8bits_2pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb , uint32_t *pl2a , uint32_t *pl2b )
{
  uint32_t mask_1 = 0x000000ff;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 24 , w );
  __cswap_2_s( b , a , 24 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 24 , mask_1&~mask_swap1 , w );
  __cswap_2_s( pl2b , pl2a , 24 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_4bits_2pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb , uint32_t *pl2a , uint32_t *pl2b )
{
  uint32_t mask_1 = 0x0000000f;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 28 , w );
  __cswap_2_s( b , a , 28 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 28 , mask_1&~mask_swap1 , w );
  __cswap_2_s( pl2b , pl2a , 28 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_2bits_2pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb , uint32_t *pl2a , uint32_t *pl2b )
{
  uint32_t mask_1 = 0x00000003;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 30 , w );
  __cswap_2_s( b , a , 30 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 30 , mask_1&~mask_swap1 , w );
  __cswap_2_s( pl2b , pl2a , 30 , mask_1&~mask_swap1 , w );
}
static inline
void __minmax_2segs_1bit_2pl( uint32_t *a , uint32_t *b, int w , uint32_t *pla , uint32_t *plb , uint32_t *pl2a , uint32_t *pl2b )
{
  uint32_t mask_1 = 0x00000001;
  uint32_t mask_swap1 = __cmp_ge_2_s( b , a , 31 , w );
  __cswap_2_s( b , a , 31 , mask_1&~mask_swap1 , w );
  __cswap_2_s( plb , pla , 31 , mask_1&~mask_swap1 , w );
  __cswap_2_s( pl2b , pl2a , 31 , mask_1&~mask_swap1 , w );
}



///////////////////////  XXX  /////////////////////////////////




static inline
uint32_t __cmp_ge_2( const uint32_t * a, const uint32_t *b , int w )  // (a>=b)?
{  // a - b
  uint32_t borrow = b[0]&~a[0];
  for(int j=1;j<w;j++) { borrow = (borrow&~(a[j]^b[j])) | (b[j]&~a[j]); }
  return ~borrow;
}

static inline
void __cswap_2( uint32_t * a, uint32_t *b, uint32_t condition , int w )
{
  for(int i=0;i<w;i++) {
    uint32_t ab = (a[i]^b[i]) & condition;
    a[i] ^= ab;
    b[i] ^= ab;
  }
}






static
void sort64_bs32( uint32_t * bs , int w )
{
  sort32_bs32( bs , w );
  sort32_bs32( bs+w , w );

  // size 64. swap(0,32)(1,33),..(31,63)
  uint32_t mask_swap0 = __cmp_ge_2( bs , bs+w , w );
  __cswap_2( bs , bs+w , mask_swap0 , w );

  // size 64. partition to 4 pieces. swap mid 2 pieces. (16-bits pieces)
  __minmax_2segs_16bits( bs , bs+w , w );

  // size 64, partition to 8 pieces, (8-bits pieces)
  __minmax_2segs_8bits( bs , bs+w , w );
  __minmax_last_8bits( bs , w );
  __minmax_last_8bits( bs+w , w );

  // size 64, partition to 16 pieces, (4-bits pieces)
  __minmax_2segs_4bits( bs , bs+w , w );
  __minmax_last_4bits( bs , w );
  __minmax_last_4bits( bs+w , w );

    // size 64, partition to 32 pieces, (2-bits pieces)
  __minmax_2segs_2bits( bs , bs+w , w );
  __minmax_last_2bits( bs , w );
  __minmax_last_2bits( bs+w , w );

    // size 64, partition to 64 pieces, (1-bits pieces)
  __minmax_2segs_1bit( bs , bs+w , w );
  __minmax_last_1bit( bs , w );
  __minmax_last_1bit( bs+w , w );

}


static
void sort128_bs32( uint32_t *bs, int w)
{
  sort64_bs32( bs , w );
  sort64_bs32( bs+2*w , w );

  // size 128. swap(0,64)(1,65),..(63,127)
  for(int i=0;i<2;i++) {
    uint32_t mask_swap0 = __cmp_ge_2( bs+i*w , bs+i*w+2*w , w );
    __cswap_2( bs+i*w , bs+i*w+2*w , mask_swap0 , w );
  }

  // size 128. partition to 4 pieces. swap mid 2 pieces. (32-bits pieces)
  {
    uint32_t mask_swap0 = __cmp_ge_2( bs+1*w , bs+2*w , w );
    __cswap_2( bs+1*w , bs+2*w , mask_swap0 , w );
  }

  // size 128. partition to 8 pieces. (16-bits pieces)
  for(int i=0;i<(4-1);i++) {
    __minmax_2segs_16bits( bs+i*w , bs+(i+1)*w , w );
  }

  // size 128, partition to 16 pieces, (8-bits pieces)
  for(int i=0;i<(4-1);i++) {
    __minmax_last_8bits( bs+i*w , w );
    __minmax_2segs_8bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_8bits( bs+3*w , w );


  // size 128, partition to 32 pieces, (4-bits pieces)
  for(int i=0;i<(4-1);i++) {
    __minmax_last_4bits( bs+i*w , w );
    __minmax_2segs_4bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_4bits( bs+3*w , w );

    // size 128, partition to 64 pieces, (2-bits pieces)
  for(int i=0;i<(4-1);i++) {
    __minmax_last_2bits( bs+i*w , w );
    __minmax_2segs_2bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_2bits( bs+3*w , w );

    // size 128, partition to 128 pieces, (1-bits pieces)
  for(int i=0;i<(4-1);i++) {
    __minmax_last_1bit( bs+i*w , w );
    __minmax_2segs_1bit( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_1bit( bs+3*w , w );

}


//  assert( n >= 32 ), assert( 0 == (n&(n-1)) )
static
void _sort_bs32( uint32_t * bs , int w , unsigned n)
{
  if(32==n) { sort32_bs32(bs,w); return; }

  int n_2 = n>>1;
  int n32 = n>>5;
  int n32_2 = n32>>1;

  _sort_bs32( bs , w , n_2 );
  _sort_bs32( bs+n32_2*w , w , n_2 );

  // partition to 2 pieces
  for(int i=0;i<n32_2;i++) {
    uint32_t mask_swap0 = __cmp_ge_2( bs+i*w , bs+i*w+n32_2*w , w );
    __cswap_2( bs+i*w , bs+i*w+n32_2*w , mask_swap0 , w );
  }

  // partition to 4 .. ?? pieces. (??-bits .. 32-bits pieces)
  for(int j = 4; j <= n32 ; j <<= 1 ) {
    int size_1piece32 = n32/j;
    for(int i=1;i<j-1; i+=2 ) {
      for(int k=0;k<size_1piece32;k++) {
        uint32_t mask_swap0 = __cmp_ge_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , w );
        __cswap_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
      }
    }
  }

  // 16-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_2segs_16bits( bs+i*w , bs+(i+1)*w , w );
  }

  // 8-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_8bits( bs+i*w , w );
    __minmax_2segs_8bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_8bits( bs+(n32-1)*w , w );

  // 4-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_4bits( bs+i*w , w );
    __minmax_2segs_4bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_4bits( bs+(n32-1)*w , w );

    // 2-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_2bits( bs+i*w , w );
    __minmax_2segs_2bits( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_2bits( bs+(n32-1)*w , w );

    // 1-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_1bit( bs+i*w , w );
    __minmax_2segs_1bit( bs+i*w , bs+(i+1)*w , w );
  }
  __minmax_last_1bit( bs+(n32-1)*w , w );
}





//  assert( n >= 32 ), assert( 0 == (n&(n-1)) )
static
void _sort_bs32_1pl( uint32_t * bs , int w , unsigned n , uint32_t *pl )
{
  if(32==n) { sort32_bs32_1pl(bs,w,pl); return; }

  int n_2 = n>>1;
  int n32 = n>>5;
  int n32_2 = n32>>1;

  _sort_bs32_1pl( bs , w , n_2 , pl );
  _sort_bs32_1pl( bs+n32_2*w , w , n_2 , pl+n32_2*w );

  // partition to 2 pieces
  for(int i=0;i<n32_2;i++) {
    uint32_t mask_swap0 = __cmp_ge_2( bs+i*w , bs+i*w+n32_2*w , w );
    __cswap_2( bs+i*w , bs+i*w+n32_2*w , mask_swap0 , w );
    __cswap_2( pl+i*w , pl+i*w+n32_2*w , mask_swap0 , w );
  }

  // partition to 4 .. ?? pieces. (??-bits .. 32-bits pieces)
  for(int j = 4; j <= n32 ; j <<= 1 ) {
    int size_1piece32 = n32/j;
    for(int i=1;i<j-1; i+=2 ) {
      for(int k=0;k<size_1piece32;k++) {
        uint32_t mask_swap0 = __cmp_ge_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , w );
        __cswap_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
        __cswap_2( pl+(i*size_1piece32+k)*w , pl+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
      }
    }
  }

  // 16-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_2segs_16bits_1pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w );
  }

  // 8-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_8bits_1pl( bs+i*w , w , pl+i*w );
    __minmax_2segs_8bits_1pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w );
  }
  __minmax_last_8bits_1pl( bs+(n32-1)*w , w , pl+(n32-1)*w );

  // 4-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_4bits_1pl( bs+i*w , w , pl+i*w );
    __minmax_2segs_4bits_1pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w );
  }
  __minmax_last_4bits_1pl( bs+(n32-1)*w , w , pl+(n32-1)*w );

    // 2-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_2bits_1pl( bs+i*w , w , pl+i*w );
    __minmax_2segs_2bits_1pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w );
  }
  __minmax_last_2bits_1pl( bs+(n32-1)*w , w , pl+(n32-1)*w );

    // 1-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_1bit_1pl( bs+i*w , w , pl+i*w );
    __minmax_2segs_1bit_1pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w );
  }
  __minmax_last_1bit_1pl( bs+(n32-1)*w , w , pl+(n32-1)*w );
}





//  assert( n >= 32 ), assert( 0 == (n&(n-1)) )
static
void _sort_bs32_2pl( uint32_t * bs , int w , unsigned n , uint32_t *pl , uint32_t *pl2 )
{
  if(32==n) { sort32_bs32_2pl(bs,w,pl,pl2); return; }

  int n_2 = n>>1;
  int n32 = n>>5;
  int n32_2 = n32>>1;

  _sort_bs32_2pl( bs , w , n_2 , pl , pl2 );
  _sort_bs32_2pl( bs+n32_2*w , w , n_2 , pl+n32_2*w , pl2+n32_2*w );

  // partition to 2 pieces
  for(int i=0;i<n32_2;i++) {
    uint32_t mask_swap0 = __cmp_ge_2( bs+i*w , bs+i*w+n32_2*w , w );
    __cswap_2( bs+i*w , bs+i*w+n32_2*w , mask_swap0 , w );
    __cswap_2( pl+i*w , pl+i*w+n32_2*w , mask_swap0 , w );
    __cswap_2( pl2+i*w , pl2+i*w+n32_2*w , mask_swap0 , w );
  }

  // partition to 4 .. ?? pieces. (??-bits .. 32-bits pieces)
  for(int j = 4; j <= n32 ; j <<= 1 ) {
    int size_1piece32 = n32/j;
    for(int i=1;i<j-1; i+=2 ) {
      for(int k=0;k<size_1piece32;k++) {
        uint32_t mask_swap0 = __cmp_ge_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , w );
        __cswap_2( bs+(i*size_1piece32+k)*w , bs+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
        __cswap_2( pl+(i*size_1piece32+k)*w , pl+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
        __cswap_2( pl2+(i*size_1piece32+k)*w , pl2+((i+1)*size_1piece32+k)*w , mask_swap0 , w );
      }
    }
  }

  // 16-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_2segs_16bits_2pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w , pl2+i*w , pl2+(i+1)*w );
  }

  // 8-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_8bits_2pl( bs+i*w , w , pl+i*w , pl2+i*w );
    __minmax_2segs_8bits_2pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w , pl2+i*w , pl2+(i+1)*w );
  }
  __minmax_last_8bits_2pl( bs+(n32-1)*w , w , pl+(n32-1)*w , pl2+(n32-1)*w );

  // 4-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_4bits_2pl( bs+i*w , w , pl+i*w , pl2+i*w );
    __minmax_2segs_4bits_2pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w , pl2+i*w , pl2+(i+1)*w );
  }
  __minmax_last_4bits_2pl( bs+(n32-1)*w , w , pl+(n32-1)*w , pl2+(n32-1)*w );

    // 2-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_2bits_2pl( bs+i*w , w , pl+i*w , pl2+i*w );
    __minmax_2segs_2bits_2pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w , pl2+i*w , pl2+(i+1)*w );
  }
  __minmax_last_2bits_2pl( bs+(n32-1)*w , w , pl+(n32-1)*w , pl2+(n32-1)*w );

    // 1-bits pieces
  for(int i=0;i<(n32-1);i++) {
    __minmax_last_1bit_2pl( bs+i*w , w , pl+i*w , pl2+i*w );
    __minmax_2segs_1bit_2pl( bs+i*w , bs+(i+1)*w , w , pl+i*w , pl+(i+1)*w , pl2+i*w , pl2+(i+1)*w );
  }
  __minmax_last_1bit_2pl( bs+(n32-1)*w , w , pl+(n32-1)*w , pl2+(n32-1)*w );
}



//////////////////////////////////////////




void sort_bs32( uint32_t * bs , int w , unsigned n)
{
  if( 0 != (n&(n-1)) ) return; // assert
  if( n < 32 ) return;         // assert
  if( n == 32 ) { sort32_bs32(bs,w); return; }
  if( n == 64 ) { sort64_bs32(bs,w); return; }
  if( n == 128 ) { sort128_bs32(bs,w); return; }

  _sort_bs32(bs,w,n);
}



void sort_bs32_1payload( uint32_t * bs , int w , unsigned n, uint32_t *payload)
{
  if( 0 != (n&(n-1)) ) return; // assert
  if( n < 32 ) return;         // assert
  if( n == 32 ) { sort32_bs32_1pl(bs,w,payload); return; }

  _sort_bs32_1pl(bs,w,n,payload);
}


void sort_bs32_2payload( uint32_t * bs , int w , unsigned n, uint32_t *pl1 , uint32_t *pl2 )
{
  if( 0 != (n&(n-1)) ) return; // assert
  if( n < 32 ) return;         // assert
  if( n == 32 ) { sort32_bs32_2pl(bs,w,pl1,pl2); return; }

  _sort_bs32_2pl(bs,w,n,pl1,pl2);
}

