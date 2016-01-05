#include <stdint.h>

#ifndef SYSCALL_H
#define SYSCALL_H

int pile_context;


typedef struct pcb_st{	
	int r0;int r1;int r2;int r3;int r4;int r5;int r6;int r7;int r8;
	int r9;int r10;int r11;int r12;
	int lr_svc;int lr_usr;
	int*sp; 
	uint32_t** page_table;
	int CPSR_user;
	struct pcb_st* next_process;
	int TERMINATED ;
    int EXIT_CODE;
	unsigned char PRIORITY;
	int DUE_TIME;
	unsigned int PID;
	int blocked ;
} pcb_s;


typedef struct st_waiting_process_sem {
	pcb_s* current;
	struct st_waiting_process_sem* next;
}waiting_process_sem;

typedef struct {
	waiting_process_sem watcher;
	int counter;
}sem_s;


void sem_init(sem_s* sem, unsigned int val);
void sem_up(sem_s* sem);
void sem_down(sem_s* sem);


void sys_reboot();

void do_sys_reboot();

void sys_nop();
void do_sys_nop();

void sys_settime(uint64_t date_ms);
void do_sys_settime();

uint64_t sys_gettime();
void do_sys_gettime();


void swi_handler(void);
void irq_handler(void);


void rearmer(void);

int fork();
int sys_fork();
void do_sys_fork();

#endif
