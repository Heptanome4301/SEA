#include <stdint.h>
#include "kheap.h"


struct fl {
	struct fl	*next;
	unsigned int	size;
} *freelist = (struct fl *) 0;

uint8_t* kernel_heap_top;
uint8_t* kernel_heap_limit;

unsigned int
aligned_value(unsigned int addr, unsigned int pwr_of_2)
{    
    unsigned int modulo = (1 << pwr_of_2);
    unsigned int max_value_modulo = modulo - 1;
    return (addr + max_value_modulo) & ~max_value_modulo;
}

uint8_t*
kAlloc_aligned(unsigned int size, unsigned int pwr_of_2)
{
	register struct fl *cfl = freelist, **prev;
	unsigned int aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);
	unsigned int size_aligned = aligned_value(size, 2);

	prev = &freelist;
	while (cfl && (aligned_cfl != (unsigned int) cfl || cfl->size != size_aligned))
	{
	    prev = &(cfl->next);
	    cfl = cfl->next;
	    aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);
	}

	if (! cfl)
	{
	    cfl = (struct fl *) kernel_heap_top;
	    aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);

	    if (aligned_cfl == (unsigned int) cfl)
	    {
		kernel_heap_top += size_aligned;
	    }
	    else
	    {
		kFree((uint8_t*) cfl, (aligned_cfl - (unsigned int) cfl));
		kernel_heap_top = (uint8_t*) (aligned_cfl + size_aligned);
	    }

	    /* No space available anymore */
	    if (kernel_heap_top > kernel_heap_limit)
		return FORBIDDEN_ADDRESS;
	}
	else
	{
	    if (aligned_cfl == (unsigned int) cfl)
	    {
		*prev = cfl->next;
	    }
	    else
	    {
		kFree((uint8_t*) cfl, cfl->size - size_aligned);
	    }
	}

	/* Fill with FORBIDDEN_BYTE to debug (more) easily */
	for (int i = 0 ; i < size_aligned ; i++) {
	    *(((uint8_t*) aligned_cfl) + i) = FORBIDDEN_BYTE;
	}
	
	return ((uint8_t *) aligned_cfl);
}

uint8_t*
kAlloc(unsigned int size)
{
	register struct fl *cfl = freelist, **prev;
	unsigned int size_aligned = (size + 3) & ~3;

	prev = &freelist;

	while (cfl && cfl->size != size_aligned)
	{
	    prev = &cfl->next;
	    cfl = cfl->next;
	}

	if (! cfl)
	{
	    cfl = (struct fl *) kernel_heap_top;
	    kernel_heap_top += size_aligned;

	    /* No space available anymore */
	    if (kernel_heap_top > kernel_heap_limit)
		return FORBIDDEN_ADDRESS;
	}
	else
	{
	    *prev = cfl->next;
	}

	/* Fill with FORBIDDEN_BYTE to debug (more) easily */
	for (int i = 0 ; i < size_aligned ; i++) {
	    *((uint8_t*) cfl) = FORBIDDEN_BYTE;
	}

	return ((uint8_t *) cfl);
}

void
kFree(uint8_t* ptr, unsigned int size)
{
	register struct fl* cfl = (struct fl*) ptr;

	cfl->size = (size + 3) & ~3;
	cfl->next = freelist;
	freelist = cfl;
}

void
kheap_init()
{
    kernel_heap_top = (uint8_t*) &__kernel_heap_start__;
    kernel_heap_limit = (uint8_t*) &__kernel_heap_end__;
}
