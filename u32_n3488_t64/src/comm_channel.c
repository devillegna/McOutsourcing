

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





#else    // mps2-an386

#include <../../common/mps2/CMSDK_CM4.h>
static inline void uart_putc(int c)
{
  while(CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk);
  CMSDK_UART0->DATA = c & 0xFFu;
}
static inline char uart_revc()
{
  while(!(CMSDK_UART0->STATE & CMSDK_UART_STATE_RXBF_Msk));
       return (char)(CMSDK_UART0->DATA);
}

void get_bytes( uint8_t * v , int l )
{
  uint16_t r;
  for(int i=0;i<l;i++) {
    r = uart_revc();
    v[i] = r&255;
  }
}
void send_bytes( const uint8_t * v , int l )
{
  for(int i=0;i<l;i++) {
    uart_putc(v[i]);
  }
}

#endif  // #if defined(STM32F407VG)

