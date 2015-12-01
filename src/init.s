;; @ Build the interrupt vector table here at address 0x8000
;; @ (because we are using Linux boot mechanism)
.globl _start
_start:
    ldr pc,reset_vector
    ldr pc,undefined_vector
    ldr pc,swi_vector
    ldr pc,prefetch_vector
    ldr pc,data_vector
    ldr pc,unused_vector
    ldr pc,irq_vector
    ldr pc,fiq_vector
reset_vector:      .word reset_asm_handler
undefined_vector:  .word undefined_asm_handler
swi_vector:        .word swi_handler
prefetch_vector:   .word prefetch_asm_handler
data_vector:       .word data_asm_handler
unused_vector:     .word unused_asm_handler
irq_vector:        .word irq_asm_handler
fiq_vector:        .word fiq_asm_handler

	
reset_asm_handler:

;;@ Copy IVT at address 0 as it should be
    mov r0,#0x8000
    mov r1,#0x0000
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
	
    ;@ FIQ 
    ;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD1
    msr cpsr_c,r0
    ldr sp, =__irq_stack_end__

    ;@ IRQ 
    ;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD2
    msr cpsr_c,r0
    ldr sp, =__irq_stack_end__

   ;@ (PSR_ABORT_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD7
    msr cpsr_c,r0
    ldr sp, =__irq_stack_end__

    ;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD3
    msr cpsr_c,r0
    ldr sp, =__svc_stack_end__

    ;@ (PSR_SYSTEM_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xDF
    msr cpsr_c,r0
    ldr sp, =__sys_stack_end__

    bl kmain

after_kmain:
    b after_kmain

;;@ Trampolines to Interrupt Service Routines
        
undefined_asm_handler:
	b undefined_asm_handler

swi_asm_handler:
	b swi_asm_handler
	
prefetch_asm_handler:
	b irq_asm_handler

irq_asm_handler:
	b irq_asm_handler

unused_asm_handler:
	b unused_asm_handler

fiq_asm_handler:
	b fiq_asm_handler

data_asm_handler:
	b data_asm_handler

