#include <stdint.h>

#ifndef SYSCALL_H
#define SYSCALL_H

int pile_context;

typedef struct {int r0;int r1;int r2;int r3;int r4;int r5;int r6;int r7;int r8;int r9;int r10;int r11;int r12;int lr_svc;int lr_usr;int*sp; int CPSR_user;} pcb_s;

void sys_reboot();
void swi_handler(void);
void do_sys_reboot();

void sys_nop();
void do_sys_nop();

void sys_settime(uint64_t date_ms);
void do_sys_settime();

uint64_t sys_gettime();
void do_sys_gettime();

void sys_yieldto(pcb_s* dest);
void do_sys_yieldto();


#endif
