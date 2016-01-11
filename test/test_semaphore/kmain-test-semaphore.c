#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"
#include "config.h"


//################################
// TEST Semaphore
//################################

sem_s* sem;

void users_process1()
{
	
  sem_init(sem, 1);
  sem_down(sem);

  int i = 1000000000;
  while(i>0){i--;};
  
  sem_up(sem);

  //while(1); //##
  
  //sys_exit(0);
  sys_nop();
}


void users_process2()
{
  sem_down(sem);
  //sys_exit(0);
  while(1);
  sem_up(sem);
}




void kmain(void){ 
  
	sched_init(sched_round_robin);

  create_process((func_t*) &users_process2,0);
  create_process((func_t*) &users_process1,0);

  timer_init();
  ENABLE_IRQ();

  // MODE USER
  __asm("cps 0x10");

  //sys_exit(0);

  start_current_process();



  PANIC();  

}



