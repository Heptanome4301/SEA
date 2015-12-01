;@ Configurer une variable
;@ void Set32(unsigned int add, int val)
.globl Set32
Set32:
	str r1, [r0]
	mov pc, lr
;@
;@ Obtenir la valeur d'un registre
;@ int Get32(unsigned int add)
.globl Get32
Get32:
	ldr r0, [r0]
	mov pc, lr


;@ Attent n cycles (environ)
;@ Prototype C : void delay(unsigned int c)
.globl delay
delay:
	lsl r0, r0, #2
	add r0, r0, #1
	delay_loop:
		subs r0, r0, #1
		bne delay_loop
	mov pc, lr

.globl memory_barrier
memory_barrier:
   mcr   p15, 0, ip, c7, c5, 0      @ invalidate I cache
   mcr   p15, 0, ip, c7, c5, 6      @ invalidate BTB
   mcr   p15, 0, ip, c7, c10, 4     @ drain write buffer
   mcr   p15, 0, ip, c7, c5, 4      @ prefetch flush
   svc   0xF00000
   mov   pc, lr

.globl invalidate_cache
invalidate_cache:
   mcr	 p15, 0, r0, c1, c0, 0
   mcr   p15, 0, r0, c7, c7, 0
   mov   pc, lr

