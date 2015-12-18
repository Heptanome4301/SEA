#include "syscall.h"
#include "util.h"
#include "asm_tools.h"
#include <stdint.h>
#include "hw.h"
#include "sched.h"


#define REBOOT_INT 1
#define NOP_INT 2
#define SYS_TIME 3
#define SYS_TIME_GT 4
#define YIELDTO 5
#define YIELD 6
#define SYS_FORK 8




void sys_reboot()
{
	__asm("mov r0, %0" : :"r"(REBOOT_INT) : "r0");    // ecriture registre
	__asm("SWI #0");
	//__asm("bl swi_handler");


}


void __attribute__((naked)) swi_handler(void)
{
	//sauvegarde du context
	__asm("stmfd sp!, {r0-r12,lr}");
	__asm("mov %0, sp" : "=r"(pile_context) );  // lecture registre

	int num_intp;
	__asm("mov %0, r0" : "=r"(num_intp ) : : "r0");  // lecture registre


	switch(num_intp)
	{
		case REBOOT_INT :
			do_sys_reboot();			
			break;

		case NOP_INT :
			do_sys_nop();
			break;

		case SYS_TIME :
			
			do_sys_settime();
			break;
		
		case SYS_TIME_GT :
			do_sys_gettime();
			break;

		case YIELDTO :
			do_sys_yieldto();
			break;
			
		case YIELD : 
			do_sys_yield();
			break;
			
		case EXIT:
			do_sys_exit();
			break;
			
		case SYS_FORK : 
			do_sys_fork();
			break;
			
		/*
		case SYS_SET_SCHEDULER :
			do_sys_set_scheduler();
			break;

*/
		default :
			PANIC();
		break;
	}

	//restaurer context
	__asm("ldmfd sp!, {r0-r12,pc}^");

}


void do_sys_reboot()
{
	__asm("mov pc, %0" : :"r"(0) ); 

	/*const int PM_RSTC = 0x2010001c;
	const int PM_WDOG = 0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

	Set32(PM_WDOG, PM_PASSWORD | 1);
	Set32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	while(1);*/

	
}


void sys_nop()
{

	__asm("mov r0, %0" : :"r"(NOP_INT) : "r0");    // ecriture registre
	__asm("SWI #0");

	

}

void do_sys_nop()
{

	//__asm("msr SPSR_svc CPSR");	
	//__asm("b LR_svc");
	
}


void sys_settime(uint64_t date_ms)
{
	
	__asm("mov r0, %0" : :"r"(SYS_TIME) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(date_ms) : "r1");    // ecriture registre
	__asm("mov r2, %0" : :"r"(date_ms >> 32) : "r2");    // ecriture registre
	__asm("SWI #0");

}


void do_sys_settime()
{
	uint64_t date_ms;
	uint64_t a;
	uint64_t b;

	a = *(int *)(pile_context+sizeof(int));
	b = *(int *)(pile_context+2*sizeof(int));
	date_ms = (a & 0x00000000ffffffff) | (b << 32) ;
	//date_ms = date_ms+1;

	set_date_ms(date_ms);

}


uint64_t sys_gettime()
{
	
	__asm("mov r0, %0" : :"r"(SYS_TIME_GT) : "r0");    // ecriture registre
	__asm("SWI #0");

	uint64_t date_ms;
	uint64_t fort;
	uint64_t faible; 

	__asm("mov %0, r0" : "=r"(faible) );  // lecture registre
	__asm("mov %0, r1" : "=r"(fort) );  // lecture registre


	date_ms = (faible & 0x00000000ffffffff) | (fort << 32) ;

	return date_ms;

}


void do_sys_gettime()
{
	uint64_t date_ms;

	date_ms = get_date_ms();

	*(int *)pile_context = (int)(date_ms & 0x00000000ffffffff) ; // faible
	*(int *)(pile_context+sizeof(int)) = (int)((date_ms & 0xffffffff00000000) >> 32 ) ; // fort

}

int led_allumee = 0;

void __attribute__((naked))irq_handler2(void)
{

	int tmp ;
	
	__asm("mov %0, lr" : "=r"(tmp) );           // lecture registre  
	  tmp -= 4; 
 
	
	if(led_allumee) led_off();
	else led_on();
	led_allumee = !led_allumee ;
	
	rearmer();
	
	__asm("mov lr, %0" : :"r"(tmp) : "lr");    // ecriture registre 
	__asm("mov pc,lr");

}


void __attribute__((naked)) irq_handler(void)
{

  //sauvegarde du context
  __asm("stmfd sp!, {r0-r12,lr}");
  __asm("mov %0, sp" : "=r"(pile_context) );      
  
  *(((int*)pile_context)+13) -= 4;
 
  do_sys_yield_irq();
  rearmer();
  

  //restauration du context
  __asm("ldmfd sp!, {r0-r12,pc}^");

}


void rearmer(void)
{
  /* 10 ms seems good */
  set_next_tick_default();
  
  /* Enable timer irq */
  ENABLE_TIMER_IRQ();

  DISABLE_IRQ();
  ENABLE_IRQ();
 
}

int fork(){
		return  sys_fork();
		
}


int sys_fork(){
	__asm("mov r0, %0" : :"r"(SYS_FORK) : "r0");    // ecriture registre	
	__asm("SWI #0");
	
	int tmp;
	__asm("mov %0, sp" : "=r"(tmp) );           // lecture registre  
	//__asm("mov sp,#4");
	return tmp;
	
}

void do_sys_fork(){
	unsigned int pid = create_process((int(*)(void))current_process->lr_usr,current_process->DUE_TIME);
	pcb_s* new_process = get_pcb_process(pid);
	new_process -> lr_svc = current_process -> lr_svc;
	
	(new_process-> sp)--;
	*(new_process-> sp) = 0;
	(current_process-> sp)--;
	*(current_process-> sp) = pid;
}








