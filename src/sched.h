#include "syscall.h"

#ifndef SCHED_H
#define SCHED_H

#define YIELDTO 5
#define SYS_SET_SCHEDULER 6

typedef  int(func_t) (void);
pcb_s* current_process;


void sched_init();
pcb_s* create_process(func_t* entry);

void sys_yieldto(pcb_s* dest);
void do_sys_yieldto();

void do_sys_set_scheduler();


#endif
