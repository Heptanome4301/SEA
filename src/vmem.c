#include "vmem.h"
#include <stdint.h>

uint32_t memory_flags = 0b000001110010; //see question 9.2
uint32_t device_flags = 0b000000110110; //see question 9.3
uint32_t fl_flags = 0b0000000001; //see fig 9.5

void 
vmem_init()
{
	init_kern_translation_table();
	configure_mmu_C();
	start_mmu_C();
}

void
start_mmu_C()
{
register unsigned int control;
__asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
__asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) */
__asm("mcr p15, 0, r0, c8, c7, 0"); //Invalidate TLB entries
/* Enable ARMv6 MMU features (disable sub-page AP) */
control = (1<<23) | (1 << 15) | (1 << 4) | 1;
/* Invalidate the translation lookaside buffer (TLB) */
__asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));
/* Write control register */
__asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
}
void
configure_mmu_C()
{
register unsigned int pt_addr = FIRST_LVL_TABLE_BASE;
//total++; //why?
/* Translation table 0 */
__asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));
/* Translation table 1 */
__asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
/* Use translation table 0 for everything */
__asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));
/* Set Domain 0 ACL to "Manager", not enforcing memory permissions
* Every mapped section/page is in domain 0
*/
__asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));
}

unsigned int
init_kern_translation_table(void)
{
	int fl_index; //first level index
	uint32_t * fl_page_entry = (uint32_t*)FIRST_LVL_TABLE_BASE; //first level

	for(fl_index = 0 ; fl_index < FIRST_LVL_TT_COUN ; fl_index++)
	{
		int sl_index; //second level index
		uint32_t* sl_page_entry= (uint32_t*)(SECON_LVL_TABLE_BASE + fl_index* SECON_LVL_TT_SIZE);

		for(sl_index = 0 ; sl_index < SECON_LVL_TT_COUN ; sl_index++)
		{
			uint32_t page_addr = ((fl_index<<8) + sl_index) <<12; //build the 20 first bits of virtual address

			if(page_addr < FIRST_LVL_TABLE_BASE && page_addr > 0)
			{
				*sl_page_entry = page_addr | memory_flags;
			} 
			else if(page_addr > IO_DEVICES_RAM_START && page_addr < IO_DEVICES_RAM_END)
			{
				*sl_page_entry = page_addr | device_flags;
			} 
			else
			{
				*sl_page_entry = 0 ; // translation fault
			}

			sl_page_entry++;
		}
		uint32_t addr = SECON_LVL_TABLE_BASE + fl_index* SECON_LVL_TT_SIZE; //see fig 9.4
		*fl_page_entry = (uint32_t)((addr) | fl_flags);

		fl_page_entry++;
	}

 	return 0;
}