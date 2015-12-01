#include "kheap.h"
#include "sched.h"


pcb_s kmain_process;



void sched_init()
{

	current_process = &kmain_process;
	kheap_init();

}


pcb_s* create_process(func_t* entry)
{
	pcb_s* res = (pcb_s*)kAlloc(sizeof(pcb_s));
	res -> lr_usr = (int)entry;
	
	int*sp = (int*)kAlloc(10000); // 10Ko
	res -> sp = (int*)(((int)sp) +( 10000/4 )) ;

	return res;

}

void sys_yieldto(pcb_s* dest)
{

	//int tmp;

	__asm("mov r0, %0" : :"r"(YIELDTO) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(dest) : "r1");    // ecriture registre
	//__asm("mov %0, lr" : "=r"(tmp) );           // lecture registre
	//__asm("mov r2, %0" : :"r"(tmp) : "r2");    // ecriture registre
	__asm("SWI #0");

}



void do_sys_yieldto()
{
	int i;
	int * p_pile_context = (int*)(pile_context);
	int * p_curr_procss = (int*)current_process;


	// Sauvegarde process current
	for(i=0;i<14;i++)
	{
		*(p_curr_procss+i) = *(p_pile_context+i);
	}

	__asm("cps 0b11111");					// passage au mode systeme
	__asm("mov %0, lr" : "=r"(current_process->lr_usr) );   // lecture registre
	__asm("mov %0, sp" : "=r"(current_process->sp) );   	// lecture registre
	__asm("cps 0b10011"); 					// passage au mode svc

	
	__asm("mrs r3,SPSR");
	__asm("mov %0, r3" : "=r"(current_process-> CPSR_user) );   // lecture registre
	

	// echange de context
	current_process = (pcb_s *)(*(p_pile_context +1)) ;    //dest

	for(i=0;i<14;i++)
	{
		*(p_pile_context+i) = *(p_curr_procss+i);
	}

	__asm("cps 0b11111");					// passage au mode systeme
	__asm("mov lr, %0" : :"r"(current_process->lr_usr));    // ecriture registre
	__asm("mov sp, %0" : :"r"(current_process->sp));    	// ecriture registre	
	__asm("cps 0b10011"); 					// passage au mode svc
	
	__asm("mov r3, %0" : :"r"(current_process->CPSR_user) : "r3" );    	// ecriture registre
	__asm("msr SPSR_s,r3"); // normalent SPSR  mais ca marche pas avec ... 
	
	
	
}

/* Cette partie n'est pas encore à l'ordre du jour

void do_sys_set_scheduler() 
{

}

void sys_set_scheduler()
{


}
*/
