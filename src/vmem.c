#include "vmem.h"
#include "kheap.h"
#include "config.h"
#include "hw.h"
#include "syscall.h"
#include "sched.h"

static const uint32_t kernel_heap_end = (uint32_t) &__kernel_heap_end__ ;

uint32_t memory_flags = 0b000001110010;
					  //0b010001110011; //see question 9.2
uint32_t device_flags = 0b000000111110;
				   	  //0b010000110111; //see question 9.3
uint32_t fl_flags = 0b0000000001; //see fig 9.5

uint8_t* occupation_table;

void 
vmem_init()
{
	kheap_init();
	unsigned int table_base = init_kern_translation_table();
	occupation_table = init_occupation_table();
	configure_mmu_C(table_base);
	//__asm("cps 0b10111");//Activate data abort and interruptions : bit 7-8 of cpsr
	//TODO Later
	//start_mmu_C();
}

void
start_mmu_C()
{
	register unsigned int control;
	__asm volatile("cpsie a"); //enable abort
	__asm volatile("cpsie i"); //enable interruptions


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
configure_mmu_C(unsigned int table_base)
{
	register unsigned int pt_addr = table_base;
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

	uint32_t table_base = (uint32_t) kAlloc_aligned(FIRST_LVL_TT_SIZE, 14); 
	//14  = 12 bits for first level index + 2bits 0
 
	int fl_index; //ST_table_base; //first level
	
	//for each pages of the first level
	for(fl_index = 0 ; fl_index < FIRST_LVL_TT_COUN ; fl_index++)
	{
		//build the first level address
		uint32_t fl_address = table_base | (fl_index << 2);

		//Allocate space for the second level table
		uint32_t fl_entry = (uint32_t) kAlloc_aligned(SECON_LVL_TT_SIZE, 10);
		// 10 = 8 bits for adress, 2bits 01

		//Build the first level translation table
		*(uint32_t*)fl_address = fl_entry | fl_flags;

		int sl_index; //second level index

		for(sl_index = 0 ; sl_index < SECON_LVL_TT_COUN ; sl_index++)
		{
			//Address in the second level table
			uint32_t sl_address = fl_entry | (sl_index << 2);

			uint32_t page_addr = ((fl_index<<8) + sl_index) <<12; //build the 20 first bits of logical address

			if(page_addr <= kernel_heap_end && page_addr > 0)
			{
				*(uint32_t*) sl_address = page_addr | memory_flags;
			} 
			else if(page_addr > IO_DEVICES_RAM_START && page_addr < IO_DEVICES_RAM_END)
			{
				*(uint32_t*) sl_address = page_addr | device_flags;
			} 
			else
			{
				*(uint32_t*) sl_address = 0 ; // translation fault
			}
		}
	}

 	return table_base;
}

uint32_t
vmem_translate(uint32_t logical_address, pcb_s* process)
{
	uint32_t pa; /* The result */

	/* 1st and 2nd table addresses */
	uint32_t table_base;
	uint32_t second_level_table;

	/* Indexes */
	uint32_t first_level_index;
	uint32_t second_level_index;
	uint32_t page_index;

	/* Descriptors */
	uint32_t first_level_descriptor;
	uint32_t* first_level_descriptor_address;
	uint32_t second_level_descriptor;
	uint32_t* second_level_descriptor_address;
	if (process == NULL)
	{
		__asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
	}
	else
	{
		table_base = (uint32_t) process->page_table;
	}
	
	table_base = table_base & 0xFFFFC000;

	/* Indexes*/
	first_level_index = (logical_address >> 20);
	second_level_index = ((logical_address << 12) >> 24);
	page_index = (logical_address & 0x00000FFF);

	/* First level descriptor */
	first_level_descriptor_address = (uint32_t*) (table_base | (first_level_index << 2));
	first_level_descriptor = *(first_level_descriptor_address); //TODO replace with get_first...

	/* Translation fault*/
	if (! (first_level_descriptor & 0x3)) 
	{
		return (uint32_t) FORBIDDEN_ADDRESS;
	}

	/* Second level descriptor */
	second_level_table = first_level_descriptor & 0xFFFFFC00;
	second_level_descriptor_address = (uint32_t*) (second_level_table | (second_level_index << 2));
	second_level_descriptor = *((uint32_t*) second_level_descriptor_address);

	/* Translation fault*/
	if (! (second_level_descriptor & 0x3)) 
	{
		return (uint32_t) FORBIDDEN_ADDRESS;
	}

	/* Physical address */
	pa = (second_level_descriptor & 0xFFFFF000) | page_index;
	return pa;
}

uint8_t*
init_occupation_table()
{
	//TODO
	uint8_t* occupation_table_start_address = kAlloc(OCCUPATION_TABLE_SIZE);
	unsigned int i;
	unsigned int table_kernel_heap_end = divide(kernel_heap_end, PAGE_SIZE);
	unsigned int table_device_start = divide(IO_DEVICES_RAM_START, PAGE_SIZE);
	unsigned int table_device_end = divide(IO_DEVICES_RAM_END, PAGE_SIZE);
	for(i = 0; i <= table_kernel_heap_end; i++)
	{
		occupation_table_start_address[i] = FRAME_OCCUPIED;
	}
	for(i = table_kernel_heap_end + 1 ; i < table_device_start ; i++)
	{
		occupation_table_start_address[i] = FRAME_FREE;
	}
	for (i = table_device_start; i <= table_device_end ; i++)
	{
		occupation_table_start_address[i] = FRAME_OCCUPIED;
	}
	return occupation_table_start_address;
}

void*
vmem_alloc_for_userland(pcb_s* process, int nb_pages)
{
	uint32_t** table_base = get_table_base(process);

	uint32_t logical_address;
	uint32_t physical_address;
	uint32_t first_page;
	int successive_empty_pages = 0;

	for (logical_address = kernel_heap_end + 1; logical_address < IO_DEVICES_RAM_START -1; logical_address += PAGE_SIZE)
	{
		physical_address = vmem_translate(logical_address, process);

		if(physical_address == (uint32_t) FORBIDDEN_ADDRESS)
		{
			successive_empty_pages ++;
		} 
		else
		{
			successive_empty_pages = 0;
		}
		if(successive_empty_pages == nb_pages)
		{
			first_page = logical_address - (nb_pages-1)*PAGE_SIZE;
			break;
		}	
	}

	int frame_number;
	int i = 0;
	uint32_t current_logical_address;
	uint32_t free_frame_address = NULL;
	for (frame_number = 0 ; frame_number < nb_pages ; frame_number ++)
	{ 
		for(i = 0; i < FRAME_TABLE_SIZE; i++)
		{
			if(occupation_table[i] == FRAME_FREE)
			{
				occupation_table[i] = FRAME_OCCUPIED;
				free_frame_address = i*4096;
				break;
			}
		}
		current_logical_address = first_page + frame_number * PAGE_SIZE;
		set_second_table_value(table_base, current_logical_address, free_frame_address);
	}


	return (uint8_t*) first_page;

}

void 
vmem_free(pcb_s* process, uint8_t* logical_address, unsigned int nb_pages)
{
	int page_counter;
	uint32_t** table_base = get_table_base(process);
	uint32_t frame;
	for (page_counter = 0 ; page_counter < nb_pages ; page_counter++)
	{
		uint32_t current_page = (uint32_t) logical_address + (page_counter * PAGE_SIZE);
		frame = vmem_translate(current_page, process);
		occupation_table[divide(frame, PAGE_SIZE)] = FRAME_FREE;
		free_second_lvl_table(table_base, current_page); //translation fault after that
	}
}

void 
do_sys_mmap()
{
	uint32_t nb_pages = *((uint32_t*) pile_context + 1); //number of pages to allocate
	uint32_t logical_address = (uint32_t) vmem_alloc_for_userland(current_process, nb_pages);
	*((uint32_t*)pile_context) = logical_address;
}

void 
do_sys_munmap()
{
	uint32_t nb_pages = *((uint32_t*) pile_context + 1); //number of pages to free
	uint32_t logical_address = *((uint32_t*) pile_context + 2);
	vmem_free(current_process, (uint8_t*)(logical_address), nb_pages);
}

void 
set_second_table_value(uint32_t** table_base, uint32_t logical_address, uint32_t physical_address)
{
	//this function do the translation to complete the translation table 
	uint32_t first_level_descriptor;
	uint32_t* second_level_descriptor_address;

	first_level_descriptor = get_first_level_descriptor(table_base, logical_address);

	second_level_descriptor_address = get_second_lvl_descriptor_address(first_level_descriptor, logical_address);

	*second_level_descriptor_address = (physical_address & PHY_ADDR_MASK) | memory_flags;
}

void
free_second_lvl_table(uint32_t** table_base, uint32_t logical_address)
{
	uint32_t first_level_descriptor;
	uint32_t* second_level_descriptor_address;

	first_level_descriptor = get_first_level_descriptor(table_base, logical_address);

	second_level_descriptor_address = get_second_lvl_descriptor_address(first_level_descriptor, logical_address);

	*second_level_descriptor_address = NULL;
}

uint32_t
get_first_level_descriptor (uint32_t** table_base, uint32_t logical_address)
{
	uint32_t first_level_index = (logical_address >> 20);

	/* First level descriptor */
	uint32_t* first_level_descriptor_address = (uint32_t*) ((uint32_t) table_base | (first_level_index << 2));
	uint32_t first_level_descriptor = *(first_level_descriptor_address);

	return first_level_descriptor;
}

uint32_t*
get_second_lvl_descriptor_address(uint32_t first_level_descriptor, uint32_t logical_address)
{
	uint32_t second_level_index = (logical_address << 12) >> 24;
	uint32_t second_level_table = first_level_descriptor & 0xFFFFFC00;
	uint32_t* second_level_descriptor_address = (uint32_t*) ((uint32_t)second_level_table | (second_level_index << 2));
	return second_level_descriptor_address;
}

uint32_t** get_table_base(pcb_s* process)
{
	uint32_t** table_base;
	if (process == NULL)
	{
		__asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
	}
	else
	{
		table_base = process->page_table;
	}
	
	return table_base;
}
