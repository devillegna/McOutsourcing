#ifndef _FLASHWRITE_STM32F4_H_
#define _FLASHWRITE_STM32F4_H_


#include "stdint.h"

uint32_t * flashpk_address(void);

uint32_t flashpk_open(void);

uint32_t flashpk_program_32bits(uint32_t address, uint32_t input_data );


uint32_t * flashpk_stack_address(void);

uint32_t flashpk_stack_erase();

uint32_t flashpk_stack_program_32bits(uint32_t address, uint32_t input_data );


uint32_t flashpk_close(void);


#endif
