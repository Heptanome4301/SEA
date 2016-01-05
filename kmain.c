#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"


void kmain(void)
{
	// supervisor mode
	__asm("cps 0x13");
	vmem_init();
  uint32_t trans0 = vmem_translate(0x48000, NULL);
	uint32_t v1 = (uint32_t) vmem_alloc_for_userland(NULL);
  uint32_t v2 = (uint32_t) vmem_alloc_for_userland(NULL);
  uint32_t trans1 = vmem_translate(v1, NULL);
  uint32_t trans2 = vmem_translate(v2, NULL);


	v1++;
  v2++;
	trans0++;
  trans1++;
  trans2++;
}

