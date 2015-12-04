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


void create_process(func_t* entry)
{
	pcb_s* res = (pcb_s*)kAlloc(sizeof(pcb_s));
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



void elect(){
	
  
  if(current_process->next_process->TERMINATED)
	{
		pcb_s* tmp =  current_process->next_process;
		current_process->next_process = tmp->next_process;

		if(current_process == current_process->next_process)
		  kmain_process .next_process = NULL;

		kFree((void*)tmp->sp,((unsigned int)10000));
		kFree((void*)tmp,((unsigned int)sizeof(pcb_s)));
	}
	
  if( current_process->next_process == NULL )
    terminate_kernel();
  else
    current_process = current_process->next_process;
	
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
