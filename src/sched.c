#include "kheap.h"
#include "sched.h"
#include "hw.h"


pcb_s kmain_process;



void sched_init()
{

	current_process = &kmain_process;
	last_process = &kmain_process;
	kheap_init();

}

void create_priority_process(func_t* entry, int priority) {
	

	pcb_s* res = (pcb_s*)kAlloc(sizeof(pcb_s));
	res -> PRIORITY =  priority;
	res -> lr_usr = (int)entry;

	res -> lr_svc = (int)entry;
	
	int*sp = (int*)kAlloc(10000); // 10Ko
	res -> sp = (int*)(((int)sp) +( 10000/4 )) ;

	res ->TERMINATED = 0;

	res -> next_process = kmain_process . next_process ;
	
	last_process->next_process = res;
	last_process = res;

	if(current_process == &kmain_process)
	 current_process = kmain_process.next_process;

}


void create_process(func_t* entry)
{
	create_priority_process(entry, 0);
}

void elect(){
	if (USE_PRIORITIES) elect_priority();
	else  elect_round_robin();
}

static void del_terminated_process (pcb_s* previous_process) { // supprime le process qui suit previous_process

	pcb_s* tmp =  previous_process->next_process;
	previous_process->next_process = tmp->next_process;

	if(previous_process == previous_process->next_process)
	  kmain_process .next_process = NULL;

	kFree((void*)tmp->sp,((unsigned int)10000));
	kFree((void*)tmp,((unsigned int)sizeof(pcb_s)));

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
	
  	if(current_process->next_process->TERMINATED){ // On enlève le processus de la liste chainée lorsqu'il est terminé
	
		del_terminated_process(current_process);
		elect_round_robin();
	} else{
	
		if( current_process->next_process == NULL )
			terminate_kernel();
		else
			current_process = current_process->next_process; // On donne la main au processus suivant
	}
	
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

	
	//__asm("mrs r3, SPSR_irq");
	__asm("mrs r3, SPSR");
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

	// passage au mode IRQ    
	__asm("cps 0b10010"); 				
	
	__asm("mov r3, %0" : :"r"(current_process->CPSR_user) : "r3" );
	//__asm("msr SPSR_irq, r3"); // ca marche pas  .. 
	//__asm("msr SPSR, r3");     // ni ca ..
		
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
