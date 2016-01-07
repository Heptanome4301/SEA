#include "syscall.h"

#ifndef SCHED_H
#define SCHED_H

//#define SYS_SET_SCHEDULER 6
#define USE_PRIORITIES 1

typedef  int(func_t) (void);

pcb_s* current_process;
pcb_s* last_process;


void sched_init();
unsigned int create_process(func_t* entry,int priority);



void elect();
void elect_priority();
void elect_round_robin();

void start_current_process();

void do_sys_yieldto();

void do_sys_yield();

void do_sys_yield_irq(void);

void do_sys_exit();

void del_terminated_process (pcb_s* previous_process) ;

void delete_timed_out_processes();

pcb_s* get_pcb_process(unsigned int pid);

/* Cette partie n'est pas encore à l'ordre du jour
void sys_set_scheduler();
void do_sys_set_scheduler();
*/


#endif
