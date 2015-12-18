#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"
#include "kheap.h"



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


void kmain_7(void){

	sched_init();
	 
	//current_process-> lr_svc = (int)(func_t*)&kmain;
	//int i;
	//for(i=0;i<NB_PROCESS;i++){
	//create_process((func_t*) &user_process1,0);
	create_process((func_t*) &user_process2,1);
	//create_process((func_t*) &user_process3,3);
	//create_process((func_t*) &user_process4,4);
		//}
 
	timer_init();
	ENABLE_IRQ();

	// MODE USER
	__asm("cps 0x10");
	start_current_process();
	
	
	create_process((func_t*) &user_process1,1);
	
	timer_init();
	ENABLE_IRQ();
	
	// MODE USER
	__asm("cps 0x10");
	start_current_process();

}
 


void pause(int mlsced){
	timer_init();
	ENABLE_IRQ();
	
}



//extern int led_allumee ;

void user_process_fork(){
	int i;
	for(i=0;i<3;i++);
	fork();
	for(i=3;;i--);
	
}

void kmain(void){
	sched_init();

	create_process((func_t*) &user_process_fork,8);


	timer_init();
	ENABLE_IRQ();

	// MODE USER
	__asm("cps 0x10");
	start_current_process();




	PANIC();  

}



















