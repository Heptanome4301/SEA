#include "hw.h"
#include "asm_tools.h"
#include "util.h"
#include "syscall.h"
#include "sched.h"
#include "vmem.h"



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
	current_process-> lr_svc = (int)(func_t*)&kmain;
	//int i;
	//for(i=0;i<NB_PROCESS;i++){
	create_process((func_t*) &user_process1,0);
	create_process((func_t*) &user_process2,2);
	create_process((func_t*) &user_process3,3);
	create_process((func_t*) &user_process4,4);
		//}
 
	timer_init();
	ENABLE_IRQ();

	__asm("cps 0x10");

	start_current_process();
	PANIC();


  /*vmem_init();
  uint32_t petit_nom = vmem_translate(0x48000, NULL);
  uint32_t grand_nom = vmem_translate(0x1500000, NULL);
  petit_nom++;
  grand_nom++;*/

  


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

