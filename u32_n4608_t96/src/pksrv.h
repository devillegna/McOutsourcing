#ifndef _PK_SERVICE_H_
#define _PK_SERVICE_H_

#include "stdint.h"


#define NUM_ROW_PK          (1248)
#define NUM_ROW_PREPK     (NUM_ROW_PK+32)

#define SIZE_CHUNK_PREPK  (NUM_ROW_PREPK*4)
#define SIZE_CHUNK_PK     (NUM_ROW_PK*4)

// 144
#define NUM_CHUNK_EXTPK      (4608/32)
// 39
#define NUM_CHUNK_I       (NUM_ROW_PK/32)
// 105
#define NUM_CHUNK_PK      (NUM_CHUNK_EXTPK-NUM_CHUNK_I)

#define PKSRV_PARAM       2

// parameter = 2 for n4608 t96
uint32_t pksrv_init( uint32_t parameter );  // return a token of the client index

uint32_t pksrv_retrive_pk( const uint8_t * pkhash );  // return a token of the client index

unsigned pksrv_store_prepk( uint32_t token , const uint8_t* prepk_chunk , unsigned idx_prepk );

unsigned pksrv_compute_pk( uint32_t token ); // compute invmat for generating pk actually

unsigned pksrv_get_pk(uint32_t token, uint8_t * pk_chunk , unsigned idx_pk );



#define _PKSRV_OUTSOURCING_



#endif
