
#include "pksrv.h"
#include "string.h"


#if defined(_PKSRV_OUTSOURCING_)


#include "comm_channel.h"


static void set_cmd( unsigned char *cmd , unsigned char b0 , unsigned char b1 )
{
  cmd[0] = b0;
  cmd[1] = b1;
}

static inline void send_cmd( const unsigned char *cmd ) { send_bytes(cmd,2); }



#if defined(STM32F407VG)
#include "hal.h"

#define _PROFILE_PKSRV_
static uint64_t _pksrv_time_used = 0;
void _pksrv_reset_used_time() { _pksrv_time_used = 0; }
uint64_t _pksrv_get_used_time() { return _pksrv_time_used; }

#else
//static inline void hal_send_str(const char * str ) {}
//static inline uint64_t hal_get_time() { return 0; }
#endif






// return a token
uint32_t pksrv_init( uint32_t parameter )
{
  if(PKSRV_PARAM != parameter ) { return 0; }  // 0 means fail !!!

#if defined(_PROFILE_PKSRV_)
uint64_t _pksrv_time_temp = hal_get_time();
#endif

  hal_send_str(":: send init() and get token");
  unsigned char cmd[2];
  unsigned char rsp[2];
  uint32_t token = 0;

  while( 0 == token ) {
    set_cmd( cmd , 0xf0 , parameter );
    send_cmd( cmd );
    get_bytes( rsp , 2 );
    token = rsp[0]&3;
  }

#if defined(_PROFILE_PKSRV_)
_pksrv_time_used += hal_get_time()-_pksrv_time_temp;
#endif

  return token;
}


// return a token
uint32_t pksrv_retrive_pk( const uint8_t * pkhash )
{
#if defined(_PROFILE_PKSRV_)
uint64_t _pksrv_time_temp = hal_get_time();
#endif

  hal_send_str(":: retrive a token from pkhash");
  unsigned char cmd[2];
  unsigned char rsp[2];

  set_cmd( cmd , 0xf0 , 255 );
  send_cmd( cmd );
  send_bytes( pkhash , 32 );

  // get ack
  get_bytes( rsp , 2 );
  uint32_t token = rsp[0]&3;
  if( 255 == rsp[1] ) token = -1;

#if defined(_PROFILE_PKSRV_)
_pksrv_time_used += hal_get_time()-_pksrv_time_temp;
#endif

  return token;
}



unsigned pksrv_store_prepk( uint32_t token , const uint8_t* prepk_chunk , unsigned idx_prepk )
{
  if( NUM_CHUNK_EXTPK <= idx_prepk ) return -1;

#if defined(_PROFILE_PKSRV_)
uint64_t _pksrv_time_temp = hal_get_time();
#endif

  hal_send_str(":: store a pk block");
  unsigned char cmd[2];
  unsigned char rsp[2];

  set_cmd( cmd , 0xf0 + 8 + (token&3) , idx_prepk&255 );
  send_cmd( cmd );
  // get ack
  get_bytes( rsp , 2 );
  // XXX: if fail ??

  // send data
  send_bytes( prepk_chunk , SIZE_CHUNK_PREPK );

#if defined(_PROFILE_PKSRV_)
_pksrv_time_used += hal_get_time()-_pksrv_time_temp;
#endif

  return 0;
}




unsigned pksrv_compute_pk( uint32_t token )  // compute invmat for generating pk actually
{
#if defined(_PROFILE_PKSRV_)
uint64_t _pksrv_time_temp = hal_get_time();
#endif

  hal_send_str(":: compute inv mat");
  unsigned char cmd[2];
  unsigned char rsp[2];

  set_cmd( cmd , 0xf0 + 4 + (token&3) , 0 );
  send_cmd( cmd );
  get_bytes( rsp , 2 );

#if defined(_PROFILE_PKSRV_)
_pksrv_time_used += hal_get_time()-_pksrv_time_temp;
#endif

  if( 0==rsp[1] ) return -1;
  return 0;
}


//////////////////////////////////////////////////



unsigned pksrv_get_pk(uint32_t token, uint8_t * pk_chunk , unsigned idx_pk )
{
  if( NUM_CHUNK_PK <= idx_pk ) return -1;

#if defined(_PROFILE_PKSRV_)
uint64_t _pksrv_time_temp = hal_get_time();
#endif

  hal_send_str(":: request a pk block");
  unsigned char cmd[2];
  unsigned char rsp[2];

  set_cmd( cmd , 0xf0 + 12 + (token&3) , idx_pk );
  send_cmd( cmd );
  // get ack
  get_bytes( rsp , 2 );
  // XXX: if fail ??

  // receive data
  get_bytes( pk_chunk , SIZE_CHUNK_PK );

#if defined(_PROFILE_PKSRV_)
_pksrv_time_used += hal_get_time()-_pksrv_time_temp;
#endif

  return 0;
}



#endif



