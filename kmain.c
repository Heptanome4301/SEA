#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"
#include "config.h"


void kmain(void)
{
  /*
  Put your code here
  and don't commit it
  please
  ...
  except if you add includes
  ...
  bisous
  */
  vmem_init();
  uint32_t * address1 = sys_mmap(12);
  uint32_t * address2 = sys_mmap(5);
  sys_munmap(address1, 12);
  uint32_t * address3 = sys_mmap (5);

  //zob++;
  address1 ++;
  address2 ++;
  address3 ++;
}
