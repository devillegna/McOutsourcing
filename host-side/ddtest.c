#include "api.h"
#include "randombytes.h"
#include "hal.h"
#include "sendfn.h"

#include <string.h>

#define NTESTS 10

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








#if defined(STM32F407VG) 
 
#include <libopencm3/stm32/rcc.h>    
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/rng.h> 
#define SERIAL_GPIO GPIOA
#define SERIAL_USART USART2


static uint16_t read_usart()
{
  return usart_recv_blocking(SERIAL_USART);
  //return usart_recv(SERIAL_USART);
}

#endif



static void try_read_all()
{
  uint16_t r;
  while ((USART_SR(SERIAL_USART) & USART_SR_RXNE)) {
    r = usart_recv(SERIAL_USART);
  }
}

static void get_bytes( uint8_t * v , int l )
{
  uint16_t r;
  for(int i=0;i<l;i++) {
    r = usart_recv_blocking(SERIAL_USART);
    v[i] = r&255;
  }
}

static void send_bytes( const uint8_t * v , int l )
{
  for(int i=0;i<l;i++) {
    usart_send_blocking(SERIAL_USART, v[i] );
  }
}



static void set_cmd( unsigned char *cmd , unsigned char b0 , unsigned char b1 )
{
  cmd[0] = b0;
  cmd[1] = b1;
}

static inline void send_cmd( const unsigned char *cmd ) { send_bytes(cmd,2); }


static char * msg0 = ":: ?=token\n";
static int msg0_size = 11;

static char * msg1 = ":: ?-th block [m4]->host\n";
static int msg1_size = 25;


static char * msg2 = ":: 0<-inv()\n";
static int msg2_size = 12;

static int test_mc348864keysrv()
{
  unsigned char data0[4*800];
  unsigned char cmd[2];
  unsigned char rsp[2];

  hal_send_str(":: testing mc348864keysrv");

  // stop 0: init and get token
  hal_send_str(":: send init() and get token");
  set_cmd( cmd , 0xf0 , 0 );
  send_cmd( cmd );
  get_bytes( rsp , 2 );
  uint8_t token = rsp[0]&3;
  memcpy( data0 , msg0 , msg0_size );
  data0[3] = '0'+token;

  send_bytes( data0 , msg0_size );

  //return -1;

  // step 1: send 24 blocks of (4x800) random bytes
  for(int i=0;i<24;i++) {
    memcpy( data0 , msg1 , msg1_size );
    data0[3] = '0'+i;
    send_bytes( data0 , msg1_size );

    set_cmd( cmd , 0xf0 + 8 + token , i );
    send_cmd( cmd );
    // get ack
    get_bytes( rsp , 2 );

    // fill out data
    randombytes( data0 , 4*(768+32) );
    // send data
    send_bytes( data0 , 4*(768+32) );
  }

  //return -1;

  // step 2: call compute()

  hal_send_str(":: compute inv mat");
  set_cmd( cmd , 0xf0 + 4 + token , 0 );
  send_cmd( cmd );
  get_bytes( rsp , 2 );
  memcpy( data0 , msg2 , msg2_size );
  data0[3] += rsp[1];
  send_bytes( data0 , msg2_size );

  if( 0==rsp[1] ) return -1;

  // step 3: store a pk block
  hal_send_str(":: store a pk block");
  set_cmd( cmd , 0xf0 + 8 + token , 24 );
  send_cmd( cmd );
  // get ack
  get_bytes( rsp , 2 );

  // fill out data
  randombytes( data0 , 4*(768+32) );
  // send data
  send_bytes( data0 , 4*(768+32) );

  // step 4: get a pk block

  hal_send_str(":: get a pk block");
  set_cmd( cmd , 0xf0 + 12 + token , 0 );
  send_cmd( cmd );
  // get ack
  get_bytes( rsp , 2 );

  // receive data
  get_bytes( data0 , 4*(768) );
  hal_send_str(":: 4x768 bytes recieved");

  return 0;
}






static int test_response_1(unsigned skip)
{

  uint32_t tmp = 0;

  get_bytes( &tmp , 2 );
  // response
  if( skip ) {
    //try_read_all();
    hal_send_str(":: SKIP response ");
    return -1;
  }

  uint8_t cmd0 = tmp&255;
  uint8_t cmd1 = (tmp>>8)&255;
  if( cmd0 >= (0xf0+8+4) ) {
    hal_send_str(":: receiving data");
    send_bytes( &tmp , 2 );

    uint32_t tmp1 = 0;
    get_bytes( &tmp1 , 2 );
    uint8_t data[8];
    get_bytes( data , 8 );
  } else if ( cmd0 >= (0xf0+8) ) {
    hal_send_str(":: sending more data");
    send_bytes( &tmp , 2 );

    uint32_t tmp1 = 0;
    get_bytes( &tmp1 , 2 );
    uint8_t data[8] = {0,1,2,3,4,5,6,7};
    send_bytes( data , 8 );
  } else {
    hal_send_str(":: repeat responses");
    send_bytes( &tmp , 2 );
  }

  return tmp^0xffff;

}

static int test_simple_response(unsigned skip)
{
  uint32_t tmp = 0;

  get_bytes( &tmp , 2 );
  // response
  if( skip ) {
    //try_read_all();
    hal_send_str(":: SKIP response ");
    return -1;
  }

  hal_send_str(":: repeat responses");
  send_bytes( &tmp , 2 );

  return tmp^0xffff;
}


int main(void)
{
  hal_setup(CLOCK_FAST);

  unsigned reading = 0;

  // marker for automated testing
  hal_send_str(":: === experiments start === ");


  test_mc348864keysrv();


  hal_send_str(":: === experiments end === ");

  return 0;
}
