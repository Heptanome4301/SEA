#ifndef VMEM_H
#define VMEM_H

#include <stdint.h>
#include "syscall.h"

#define PAGE_SIZE 4096
#define SECON_LVL_TT_COUN 256  //Second level pages are referenced by an 8 bits index
#define SECON_LVL_TT_SIZE (4*SECON_LVL_TT_COUN) //Entry are on 32 bits->4 byte times the number of entry

#define FIRST_LVL_TT_COUN 4096 //first level pages are referenced by a 12 bits index
#define FIRST_LVL_TT_SIZE (4*FIRST_LVL_TT_COUN) //same as second level

#define FIRST_LVL_TABLE_BASE 0X1100000 //start of the user space, end of the kernel heap
#define SECON_LVL_TABLE_BASE (FIRST_LVL_TABLE_BASE + FIRST_LVL_TT_SIZE)

#define TOTAL_TT_SIZE (FIRST_LVL_TT_SIZE + SECON_LVL_TT_SIZE * FIRST_LVL_TT_COUN)

#define OCCUPATION_TABLE_SIZE 0x21000

#define IO_DEVICES_RAM_START 0x20000000
#define IO_DEVICES_RAM_END 0x20FFFFFF

#define FRAME_TABLE_SIZE 0x210000

#define FRAME_OCCUPIED 1
#define FRAME_FREE 0

#define FIRST_LVL_TABLE_ALIGN 14
#define SECON_LVL_TABLE_ALIGN 10

static const uint32_t FIRST_LVL_ADDR_MASK = 0xFFFFC000; // last 14 bits to 0
static const uint32_t SECOND_LVL_ADDR_MASK = 0xFFFFFC00; // last 10 bits to 0
static const uint32_t PHY_ADDR_MASK = 0xFFFFF000; // last 12 bits to 0

/*Start the mmu*/
void start_mmu_C();
/*Configure the mmu*/
void configure_mmu_C();
/*Initialize and fill the kernel translation table*/
unsigned int init_kern_translation_table(void);
/*Initialize the memory*/
void vmem_init();
/*Simulate traduction process give physical address from logical address*/
uint32_t vmem_translate(pcb_s* process, uint32_t logical_address);
/*Init and fill the pages occupation table*/
uint8_t* init_occupation_table();
/*Allocate a given number of memory pages for a given process*/
void* vmem_alloc_for_userland(pcb_s* process, int nb_pages);

/*Return the table base of a process*/
uint32_t** get_table_base(pcb_s* process);
void set_second_table_value(uint32_t** table_base, uint32_t logical_address, uint32_t physical_address, uint32_t flags);
uint32_t get_first_level_descriptor (uint32_t** table_base, uint32_t logical_address);
uint32_t* get_second_lvl_descriptor_address(uint32_t first_level_descriptor, uint32_t logical_address);
void free_second_lvl_table(uint32_t** table_base, uint32_t logical_address);

/*free a given number of pages for a process*/
void vmem_free(pcb_s* process, void* logical_address, unsigned int nb_pages);
/*System call for allocate memory to a process*/
void do_sys_mmap();
/*System call for free memory of a process*/
void do_sys_munmap();

#endif