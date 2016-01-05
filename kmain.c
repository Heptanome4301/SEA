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
 /* int n = -1;
  while(1){
    n--;
    // sys_yield();
  }*/
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

void kmain_fork(void){
	sched_init();

	create_process((func_t*) &user_process_fork,8);


	timer_init();
	ENABLE_IRQ();

	// MODE USER
	__asm("cps 0x10");
	start_current_process();




	PANIC();  

}

sem_s* sem;

void users_process1()
{
	sem_init(sem, 1);
	sem_down(sem);

	int i = 1000000000;
	while(i>0){i--;};
	
	sem_up(sem);
}


void users_process2()
{
  sem_down(sem);
  while(1);
  sem_up(sem);
}




void kmain_sem(void){ // attention faut mettre sched_roud_robin
	sched_init();

	create_process((func_t*) &users_process1,0);
	create_process((func_t*) &users_process2,0);

	timer_init();
	ENABLE_IRQ();

	// MODE USER
	__asm("cps 0x10");
	start_current_process();




	PANIC();  

}

















