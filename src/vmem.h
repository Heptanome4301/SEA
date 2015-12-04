#ifndef VMEM_H
#define VMEM_H

#define PAGE_SIZE 4096
#define SECON_LVL_TT_COUN 256  //Second level pages are referenced by an 8 bits index
#define SECON_LVL_TT_SIZE (4*SECON_LVL_TT_COUN) //Entry are on 32 bits->4 byte times the number of entry

#define FIRST_LVL_TT_COUN 4096 //first level pages are referenced by a 12 bits index
#define FIRST_LVL_TT_SIZE (4*FIRST_LVL_TT_COUN) //same as second level

#define FIRST_LVL_TABLE_BASE 0X1000000 //start of the user space, end of the kernel heap
#define SECON_LVL_TABLE_BASE (FIRST_LVL_TABLE_BASE + FIRST_LVL_TT_SIZE)

#define IO_DEVICES_RAM_START 0x20000000
#define IO_DEVICES_RAM_END 0x20FFFFFF


void start_mmu_C();
void configure_mmu_C();
unsigned int init_kern_translation_table(void);
void vmem_init();

#endif