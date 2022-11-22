

#include "comm_channel.h"


#if defined(STM32F407VG)

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/rng.h>
#define SERIAL_GPIO GPIOA
#define SERIAL_USART USART2


void get_bytes( uint8_t * v , int l )
{
  uint16_t r;
  for(int i=0;i<l;i++) {
    r = usart_recv_blocking(SERIAL_USART);
    v[i] = r&255;
  }
}

void send_bytes( const uint8_t * v , int l )
{
  for(int i=0;i<l;i++) {
    usart_send_blocking(SERIAL_USART, v[i] );
  }
}





#else    // defined(STM32F407VG)




#if 0
void hal_send_str(const char * str)
{
  const char *cur = str;
  while(*cur) {
    send_bytes( cur , 1 );
    cur++;
  }
  send_bytes( "\n" , 1 );
}
#endif


#include <fcntl.h>
#include "unistd.h"
#include "stdio.h"
#include "stdlib.h"


static int fd = -1;

void get_bytes( uint8_t * v , int l )
{
//  uint8_t *bb = v;

  if( -1 == fd ) fd = open( COMM_TTY , O_RDWR | O_NOCTTY | O_SYNC);
  if( -1 == fd ) { printf("can not open serial port"); exit(-1); }

  while(l) {
    int sz = read( fd , v , l );
    if( 0 > sz ) break;
    if( 0 < sz ) {
      l -= sz;
      v += sz;
    }
  }
  //close( fd );
//  if( l < 3 ) {
//    printf("get bytes: [%02x,%02x].\n", bb[0], bb[1]);
//  }

}

void send_bytes( const uint8_t * v , int l )
{
//  if( l < 3 ) {
//    printf("send bytes: [%02x,%02x] \n", v[0], v[1]);
//  }

  if( -1 == fd ) fd = open( COMM_TTY , O_RDWR | O_NOCTTY | O_SYNC);
  if( -1 == fd ) { printf("can not open serial port"); exit(-1); }

  while(l) {
    int sz = write( fd , v , l );
    if( 0 > sz ) break;
    if( 0 < sz ) {
      l -= sz;
      v += sz;
    }
  }
  //close( fd );
}



#endif  // #if defined(STM32F407VG)

