#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"



#define NB_PROCESS 4

void user_process4()
{
  int n = -1;
  while(1){
    n--;
    // sys_yield();
  }
}

void user_process3()
{
  int n = -1;
  while(1){
    n--;
    // sys_yield();
  }
}

void user_process2()
{
  int n = -1;
  while(1){
    n--;
    // sys_yield();
  }
}


void user_process1()
{
	int v = 0; 
	while(1){
		v++;
		//	sys_yield();
	}
	//sys_exit(0);
}



void kmain(void){

	sched_init();
	
	//int i;
	//for(i=0;i<NB_PROCESS;i++){
	create_priority_process((func_t*) &user_process1,2);
	create_priority_process((func_t*) &user_process2,0);
	create_priority_process((func_t*) &user_process3,12);
	create_priority_process((func_t*) &user_process4,10);
		//}
 
	timer_init();
	ENABLE_IRQ();

	__asm("cps 0x10");

	start_current_process();

}





void kmain_7_1(void){

  timer_init();
  ENABLE_IRQ();
  
 __asm("cps 0x10");

//	sys_yieldto(p1);*/
  int i = 0 ;
  while(1) i++;

  PANIC();  

}

