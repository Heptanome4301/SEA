#include "syscall.h"
#include "util.h"
#include "asm_tools.h"
#include <stdint.h>
#include "hw.h"
#include "sched.h"
#include "kheap.h"
#include "vmem.h"

int led_allumee = 0;

enum SYSCALLS
{
	REBOOT,
	NOP,
	SET_TIME,
	GET_TIME,
	YIELDTO,
	YIELD,
	FORK,
	EXIT,
	MMAP,
	MUNMAP,
};

void __attribute__((naked)) 
swi_handler(void)
{
	//sauvegarde du context
	__asm("stmfd sp!, {r0-r12,lr}");
	__asm("mov %0, sp" : "=r"(pile_context) );  // lecture registre

	int num_intp;
	__asm("mov %0, r0" : "=r"(num_intp ) : : "r0");  // lecture registre


	switch(num_intp)
	{
		case REBOOT :
			do_sys_reboot();			
			break;

		case NOP :
			do_sys_nop();
			break;

		case SET_TIME :
			do_sys_settime();
			break;
		
		case GET_TIME :
			do_sys_gettime();
			break;

		case YIELDTO :
			do_sys_yieldto();
			break;
			
		case YIELD : 
			do_sys_yield();
			break;

		case FORK : 
			do_sys_fork();
			break;
			
		case EXIT:
			do_sys_exit();
			break;
			
		case MMAP : 
			do_sys_mmap();
			break;

		case MUNMAP :
			do_sys_munmap();
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

/*Reboot the system*/
void 
sys_reboot()
{
	__asm("mov r0, %0" : :"r"(REBOOT) : "r0");    // ecriture registre
	__asm("SWI #0");
	//__asm("bl swi_handler");
}

void
do_sys_reboot()
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

/* Does nothing, the return*/
void 
sys_nop()
{
	__asm("mov r0, %0" : :"r"(NOP) : "r0");    // ecriture registre
	__asm("SWI #0");
}

void 
do_sys_nop()
{
	//__asm("msr SPSR_svc CPSR");	
	//__asm("b LR_svc");
}

/*Set the timer to the given date*/
void 
sys_settime(uint64_t date_ms)
{
	__asm("mov r0, %0" : :"r"(SET_TIME) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(date_ms) : "r1");    // ecriture registre
	__asm("mov r2, %0" : :"r"(date_ms >> 32) : "r2");    // ecriture registre
	__asm("SWI #0");
}

void 
do_sys_settime()
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

/*Get the system time*/
uint64_t 
sys_gettime()
{
	__asm("mov r0, %0" : :"r"(GET_TIME) : "r0");    // ecriture registre
	__asm("SWI #0");

	uint64_t date_ms;
	uint64_t fort;
	uint64_t faible; 

	__asm("mov %0, r0" : "=r"(faible) );  // lecture registre
	__asm("mov %0, r1" : "=r"(fort) );  // lecture registre

	date_ms = (faible & 0x00000000ffffffff) | (fort << 32) ;

	return date_ms;
}

void 
do_sys_gettime()
{
	uint64_t date_ms;

	date_ms = get_date_ms();

	*(int *)pile_context = (int)(date_ms & 0x00000000ffffffff) ; // faible
	*(int *)(pile_context+sizeof(int)) = (int)((date_ms & 0xffffffff00000000) >> 32 ) ; // fort
}

/*Exit a processus*/
void 
sys_exit(int status)
{
	__asm("mov r0, %0" : :"r"(EXIT) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(status) : "r1");    // ecriture registre
	__asm("SWI #0");
}

/*Step down for another processus*/
void 
sys_yieldto(pcb_s* dest)
{
	//int tmp;
	__asm("mov r0, %0" : :"r"(YIELDTO) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(dest) : "r1");    // ecriture registre
	//__asm("mov %0, lr" : "=r"(tmp) );           // lecture registre
	//__asm("mov r2, %0" : :"r"(tmp) : "r2");    // ecriture registre
	__asm("SWI #0");
}

int 
fork(){
		__asm("mov %0, lr" : "=r"(current_process->lr_svc) );  // lecture registre
		//__asm("mov %0, lr" : "=r"(fort) );  // lecture registre
		return  sys_fork();		
}

int 
sys_fork(){
	__asm("mov r0, %0" : :"r"(FORK) : "r0");    // ecriture registre	
	__asm("SWI #0");
	
	int tmp;
	__asm("mov %0, sp" : "=r"(tmp) );           // lecture registre  
	//__asm("mov sp,#4");
	return tmp;	
}

void 
do_sys_fork(){
	unsigned int pid = create_process((int(*)(void))current_process->lr_usr,current_process->DUE_TIME);
	pcb_s* new_process = get_pcb_process(pid);
	new_process -> lr_svc = current_process -> lr_svc;
	
	(new_process-> sp)--;
	*(new_process-> sp) = 0;
	(current_process-> sp)--;
	*(current_process-> sp) = pid;
}

void* 
sys_mmap(unsigned int size)
{
	__asm("mov r1, %0": : "r"(size));
	__asm("mov r0, %0": : "r"(MMAP));
	__asm("SWI #0");
	
	void* allocated_mem;
	__asm("mov %0, r0" : "=r"(allocated_mem));

	return allocated_mem;
}

void 
sys_munmap(void* logical_address, unsigned int size)
{

	__asm("mov r2, %0" : : "r"((uint32_t)(logical_address)));
	__asm("mov r1, %0": : "r"(size));
	__asm("mov r0, %0": : "r"(MUNMAP));
	__asm("SWI #0");
}

void __attribute__((naked))
irq_handler2(void)
{

	int tmp ;
	
	__asm("mov %0, lr" : "=r"(tmp) );           // lecture registre  
	  tmp -= 4; 
	
	if(led_allumee) led_off();
	else led_on();
	led_allumee = !led_allumee ;
	
	rearm();
	
	__asm("mov lr, %0" : :"r"(tmp) : "lr");    // ecriture registre 
	__asm("mov pc,lr");

}

void __attribute__((naked)) 
irq_handler(void)
{
  //sauvegarde du context
  __asm("stmfd sp!, {r0-r12,lr}");
  __asm("mov %0, sp" : "=r"(pile_context) );      
  
  *(((int*)pile_context)+13) -= 4;
 
  do_sys_yield_irq();
  rearm();

  //restauration du context
  __asm("ldmfd sp!, {r0-r12,pc}^");
}

void 
rearm(void)
{
  /* 10 ms seems good */
  set_next_tick_default();
  
  /* Enable timer irq */
  ENABLE_TIMER_IRQ();

  DISABLE_IRQ();
  ENABLE_IRQ();
}

void 
sem_init(sem_s* sem, unsigned int val)
{
	sem->watcher.next = NULL;
	sem->counter = val;
}

void 
sem_up(sem_s* sem)
{	
	sem->counter++;
	
	//supprimer le current process de waiting_process_sem
	waiting_process_sem* tmp = &sem->watcher;
	while(tmp->next !=NULL){
		if(tmp->next->current == current_process)
		{
			//suppression de current_process
			waiting_process_sem*  tmp2 =  tmp->next;
			tmp->next = tmp->next->next;
			kFree((void*)tmp2,((unsigned int)sizeof(waiting_process_sem)));
			break;
		}
		tmp = tmp->next;
	}
	
	//débloquer une eventuel process blocké dans waiting_process_sem
	tmp = &sem->watcher;
	while(tmp->next !=NULL){
		if(tmp->next->current->blocked )
		{
			tmp->next->current->blocked = 0;
			break;
		}
		tmp = tmp->next;
	}	
}

void 
sem_down(sem_s* sem)
{
	waiting_process_sem* new = (waiting_process_sem*)kAlloc(sizeof(waiting_process_sem)); 
	new->current = current_process;
	new->next = NULL;
	
	waiting_process_sem* tmp = &sem->watcher;
	while(tmp->next !=NULL){ // trouver la queue de waiting_process_sem
		tmp = tmp->next;
	}
	tmp->next = new;
	
	sem->counter--;
	if(sem->counter<=0){
		current_process -> blocked = 1;		
	}
}