#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"
#include "config.h"


//##############
//TEST sched [round-robin]
//#############

void user_process4()
{
  int n = 4;
  while(1){
    n*=4;
  }
}

void user_process3()
{
  int n = 3;
  while(1){
    n*=3;
  }
}

void user_process2()
{
  int n = 2;
  while(1){
    n*=2;
  }
}


void user_process1()
{
  int v = 1; 
  while(1){
    v*=1;
  }
}


void kmain(void){
	
		sched_init(sched_round_robin);
	
		int i = 0;
	
		create_process((func_t*) &user_process1,i);
		create_process((func_t*) &user_process2,i);
		create_process((func_t*) &user_process3,i);
		create_process((func_t*) &user_process4,i);
	  
	  
	timer_init();
	ENABLE_IRQ();

  // MODE USER
	__asm("cps 0x10");
	start_current_process();
	
}
