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


static void set_cmd( unsigned char *cmd , unsigned char b0 , unsigned char b1 , unsigned char b2 , unsigned char b3 )
{
  cmd[0] = b0;
  cmd[1] = b1;
  cmd[2] = b2;
  cmd[3] = b3;
}

static void send_cmd( const unsigned char *cmd )
{
  usart_send_blocking(SERIAL_USART, cmd[0]);
  usart_send_blocking(SERIAL_USART, cmd[1]);
  usart_send_blocking(SERIAL_USART, cmd[2]);
  usart_send_blocking(SERIAL_USART, cmd[3]);
}


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




static int test_something()
{
  unsigned char data0[8];
  unsigned char data1[8];
  unsigned char cmd[4];

  hal_send_str("[m4] send init command and read usart 4 times: ");

  set_cmd( cmd , 0xbb , 0 , 0 , '\n' );
  send_cmd( cmd );

  for(int i=0;i<4;i++) {
    unsigned r = read_usart();
    send_unsigned("[m4] read :" , r );
  }

  // wait for response and token
  // if timeout
  return -1;

  // step 1: call compute()


  // step 2: store 8 bytes to the host

  // step 3: read 8 bytes from the host

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



  int i=1;
  //while( test_simple_response( (i++)&1 ) ) { ; }
  while( test_response_1( (i++)&1 ) ) { ; }


  hal_send_str(":: === experiments end === ");

  return 0;
}
