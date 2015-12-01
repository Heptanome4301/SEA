#ifndef ASM_TOOLS_H
#define ASM_TOOLS_H

#include <stdint.h>

/*
 * Exceptions management
 */
#define ENABLE_IRQ()     __asm volatile("cpsie i");
#define DISABLE_IRQ()     __asm volatile("cpsid i");

#define ENABLE_AB()    __asm volatile("cpsie a");
#define DISABLE_AB()    __asm volatile("cpsid a");				

#define LOAD_KERNEL_PAGE_TABLE()					\
    __asm volatile("push {r0}");					\
    __asm volatile("mov	r0, %0" : : "r"(kmain_process.page_table));	\
    __asm volatile("mcr 	p15, 0, r0, c2, c0, 0");		\
    __asm volatile("pop {r0}");


#define LOAD_CURRENT_PAGE_TABLE()					\
    __asm volatile("mov	  r0, %0" : : "r"(current_process->page_table)); \
    __asm volatile("mcr   p15, 0, r0, c2, c0, 0");

#define INVALIDATE_TLB()				\
    __asm volatile("mcr   p15, 0, sp, c8, c7, 0");

/*
 * Functions
 */
void Set32(uint32_t add, int val);
int Get32(uint32_t add);
void delay(unsigned int c);
void memory_barrier();
void invalidate_cache();
void invalidate_tlb_asm();

#endif
