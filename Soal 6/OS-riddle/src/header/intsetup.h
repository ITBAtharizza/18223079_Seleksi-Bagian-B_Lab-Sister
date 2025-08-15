#ifndef __INTSETUP__
#define __INTSETUP__
#include "header/cpu/idt.h"
#include <stdint.h>

void call_generic_handler(uint32_t eip, uint16_t cs, uint32_t eflags, uint32_t int_number, uint32_t error_code);

#endif

