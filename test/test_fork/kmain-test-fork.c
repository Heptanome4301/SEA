#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"
#include "config.h"



//################################
// TEST Fork
//################################



void user_process_fork(){
  int i;
  for(i=0;i<3;i++);
  if(fork()){
	int k = 0; k = k +1 -1;
	while(1);
  }else {
	int s = 0; s = s +1 -1;
	while(1);
  }
 
  
}

void kmain(void){
  sched_init(sched_round_robin);

  create_process((func_t*) &user_process_fork,8);


  timer_init();
  ENABLE_IRQ();

  // MODE USER
  __asm("cps 0x10");
  start_current_process();

  PANIC();  
}



