#include "kheap.h"
#include "sched.h"
#include "hw.h"


#define STACK_SIZE 10000
#define WORD_SIZE 4

#define SCHEDULER 2


pcb_s kmain_process;

enum sched_type {
	sched_round_robin,
	sched_fcfs,
	sched_priority
	
};






void sched_init(void)
{

	current_process = &kmain_process;
	last_process = &kmain_process;
	
	#if VMEM
	vmem_init();
	#else
	kheap_init();
	#endif
}


void init_sched_priority(pcb_s* res, int priority) {
	
	
	res -> PRIORITY = priority;
	
	res -> next_process = kmain_process . next_process ;
	
	last_process->next_process = res;
	last_process = res;

	if(current_process == &kmain_process)
	 current_process = kmain_process.next_process;
}



void init_sched_round_robin(pcb_s* res)
{
		res -> next_process = kmain_process . next_process ;
		
		last_process->next_process = res;
		last_process = res;

		if(current_process == &kmain_process)
		current_process = kmain_process.next_process;
}


void init_sched_fcfs(pcb_s* res){
	res -> next_process = kmain_process.next_process  ;
	kmain_process.next_process = res;
	
	if(current_process == &kmain_process)
		current_process = kmain_process.next_process;
}



void define_sched(pcb_s* res,int priority)
{
	switch(SCHEDULER){
	
	case sched_round_robin :
		init_sched_round_robin(res);
		break;
		
	case sched_priority :
		init_sched_priority(res,priority);
		break;
	
	case sched_fcfs :
		init_sched_fcfs(res);
		break;
	}
}



void create_process(func_t* entry,int priority)
{
	
	pcb_s* res = (pcb_s*)kAlloc(sizeof(pcb_s));


	res -> lr_usr = (int)entry;
	res -> lr_svc = (int)entry;
	res ->  CPSR_user = 0x10; // mode user
	
	int*sp = (int*)kAlloc(STACK_SIZE); // 10Ko
	res -> sp = (int*)(((int)sp) +( STACK_SIZE/WORD_SIZE )) ;
	res ->TERMINATED = 0;

	define_sched(res,priority);
}


void elect_sched_round_robin()
{
  
	  
  if( current_process->next_process != NULL )
    current_process = current_process->next_process;
	
}

void elect_sched_fcfs()
{
  
  if( kmain_process .next_process != NULL )
    current_process = kmain_process .next_process;
	
	
}

void elect(){
	
	// supprimer le process fini 
	del_terminated_process (current_process);


	switch(SCHEDULER) {
	case sched_round_robin :
		elect_round_robin();
		break;
		
	case sched_priority :
		elect_priority();
		break;
	
	case sched_fcfs :
		elect_sched_fcfs();
		break;
	}
}



void del_terminated_process (pcb_s* previous_process) { // supprime le process qui suit previous_process
	
	if(previous_process->next_process->TERMINATED)
	{
		pcb_s* tmp =  previous_process->next_process;
		previous_process->next_process = tmp->next_process;


		if(previous_process == previous_process->next_process)
		  kmain_process .next_process = NULL;

		kFree((void*)tmp->sp,((unsigned int)STACK_SIZE));
		kFree((void*)tmp,((unsigned int)sizeof(pcb_s)));
	}
	
	if(current_process == &kmain_process)
	 current_process = kmain_process.next_process;

}



void elect_priority() {

	for (pcb_s *proc = current_process->next_process; proc != current_process; proc = proc->next_process){
		if(proc->next_process->TERMINATED){
			del_terminated_process(proc);
		}
	}

	pcb_s *process_prioritaire = current_process->next_process;

	for (pcb_s *proc = current_process->next_process; proc != current_process; proc = proc->next_process){
		if (proc->PRIORITY > process_prioritaire->PRIORITY) 
			process_prioritaire = proc;
	}

	if( process_prioritaire == NULL )
		terminate_kernel();
	else
		current_process = process_prioritaire; // On donne la main au processus qui a la plus grande priorité


}



void elect_round_robin(){
	
	
		if( current_process->next_process == NULL )
			terminate_kernel();
		else
			current_process = current_process->next_process; // On donne la main au processus suivant


	
	
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



void start_current_process(){
  //( (func_t*) (current_process -> lr_usr) )() ;
  __asm("mov r4, %0" : :"r"(current_process -> lr_usr) : "r4");  
  __asm("bx r4");
  sys_exit(0);

}




void do_sys_yield(void)
{
	int i;
	int * p_pile_context = (int*)(pile_context);
	int * p_curr_procss = (int*)current_process;


	// Sauvegarde process current
	for(i=0;i<14;i++)
	{
		*(p_curr_procss+i) = *(p_pile_context+i);
	}

	// passage au mode systeme
	__asm("cps 0b11111");
	__asm("mov %0, lr" : "=r"(current_process->lr_usr) );  
	__asm("mov %0, sp" : "=r"(current_process->sp) );   

	// passage au mode svc
	__asm("cps 0b10011"); 				

	
	__asm("mrs r3,SPSR");
	__asm("mov %0, r3" : "=r"(current_process-> CPSR_user) );   
	

	// echange de context
	elect();
	p_curr_procss = (int*)current_process;
	//current_process = (pcb_s *)(*(p_pile_context +1)) ;    //dest

	for(i=0;i<14;i++)
	{
		*(p_pile_context+i) = *(p_curr_procss+i);
	}

	
	// passage au mode systeme
	__asm("cps 0b11111");					
	__asm("mov lr, %0" : :"r"(current_process->lr_usr)); 
	__asm("mov sp, %0" : :"r"(current_process->sp));

	// passage au mode svc    
	__asm("cps 0b10011"); 				
	
	__asm("mov r3, %0" : :"r"(current_process->CPSR_user) : "r3" );
	__asm("msr SPSR_s,r3"); // normalent SPSR  mais ca marche pas avec ... 
	
}




void do_sys_yieldto(void)
{
	int i;
	int * p_pile_context = (int*)(pile_context);
	int * p_curr_procss = (int*)current_process;


	// Sauvegarde process current
	for(i=0;i<14;i++)
	{
		*(p_curr_procss+i) = *(p_pile_context+i);
	}

	// passage au mode systeme
	__asm("cps 0b11111");					
	__asm("mov %0, lr" : "=r"(current_process->lr_usr) );   
	__asm("mov %0, sp" : "=r"(current_process->sp) );   
	
	
	// passage au mode svc	
	__asm("cps 0b10011"); 					

	
	__asm("mrs r3,SPSR");
	__asm("mov %0, r3" : "=r"(current_process-> CPSR_user) );   
	

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



// pcb_s* current_process;
void do_sys_yield_irq(void)
{
	int i;
	int * p_pile_context = (int*)(pile_context);
	int * p_curr_procss = (int*)current_process;


	// Sauvegarde process current
	for(i=0;i<14;i++)
	{
		*(p_curr_procss+i) = *(p_pile_context+i);
	}

	// passage au mode systeme
	__asm("cps 0b11111");
	__asm("mov %0, lr" : "=r"(current_process->lr_usr) ); 
	__asm("mov %0, sp" : "=r"(current_process->sp) );   

	// passage au mode IRQ
	__asm("cps 0b10010"); 				

	
	__asm("mrs %0, SPSR" : "=r" (current_process-> CPSR_user) );
	

	// echange de context
	elect();
	p_curr_procss = (int*)current_process;
	//current_process = (pcb_s *)(*(p_pile_context +1)) ;    //dest


	for(i=0;i<14;i++)
	{
		*(p_pile_context+i) = *(p_curr_procss+i);
	}

	
	// passage au mode systeme
	__asm("cps 0b11111");					
	__asm("mov lr, %0" : :"r"(current_process->lr_usr)); 
	__asm("mov sp, %0" : :"r"(current_process->sp));

	// passage au mode IRQ    
	__asm("cps 0b10010"); 				
	
 
	__asm("msr SPSR, %0" :: "r" (current_process->CPSR_user) ); 
	
		
}



void sys_exit(int status)
{
	__asm("mov r0, %0" : :"r"(EXIT) : "r0");    // ecriture registre
	__asm("mov r1, %0" : :"r"(status) : "r1");    // ecriture registre
	__asm("SWI #0");
}


void do_sys_exit()
{  
	current_process ->TERMINATED = 1;
	__asm("mov %0, r1" : "=r"(current_process -> EXIT_CODE )); 
}




/* Cette partie n'est pas encore à l'ordre du jour

void do_sys_set_scheduler() 
{

}

void sys_set_scheduler()
{


}
*/
