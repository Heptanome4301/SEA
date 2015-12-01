#ifndef PHYALLOC_H
#define PHYALLOC_H

#include <stdint.h>

extern uint8_t* kernel_heap_top;
extern uint8_t* kernel_heap_limit;

extern uint32_t __kernel_heap_start__;
extern uint32_t __kernel_heap_end__;
extern uint32_t __after_kernel_heap__;
extern uint32_t __devices_start__;
extern uint32_t __devices_end__;
extern uint32_t __stacks_end__;
extern uint32_t __bss_end__;

unsigned int aligned_value(unsigned int addr, unsigned int pwr_of_2);
uint8_t* kAlloc_aligned(unsigned int size, unsigned int pwr_of_2);
uint8_t* kAlloc(unsigned int size);
void kFree(uint8_t* ptr, unsigned int size);
void kheap_init();

#define FORBIDDEN_BYTE ((uint8_t) 0x00)
#define FORBIDDEN_ADDRESS (void*) 0xFFFFFFF0

#endif
