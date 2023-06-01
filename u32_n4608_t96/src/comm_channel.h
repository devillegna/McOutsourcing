
#ifndef _COMM_CHANNEL_H_
#define _COMM_CHANNEL_H_

#include "stdint.h"

void get_bytes( uint8_t * v , int l );

void send_bytes( const uint8_t * v , int l );


//#if !defined(STM32F407VG)
//void hal_send_str(const char * str);
//#endif


/*// Use usart_XXXX_blocking for STM32F4
#if defined(_MAC_OS_)
#define COMM_TTY    "/dev/ttys007"
#else
#define COMM_TTY    "/dev/pts/3"
#endif*/

#endif  //#ifndef _COMM_CHANNEL_H_
