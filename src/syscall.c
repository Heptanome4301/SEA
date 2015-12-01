#include "syscall.h"
#include "util.h"
#include "asm_tools.h"
#include <stdint.h>
#include "hw.h"
#include "sched.h"

#define WORD_SIZE 4

#define REBOOT_INT 1
#define NOP_INT 2
#define SYS_TIME 3
#define SYS_TIME_GT 4




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