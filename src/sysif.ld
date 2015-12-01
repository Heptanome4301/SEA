MEMORY {
	RAM : ORIGIN = 0x8000, LENGTH = 0x20000000-0x8000
    PERIPHERALS : ORIGIN = 0x20000000, LENGTH = 0x20FFFFFF - 0x20000000
}

KERNEL_STACK_SIZE = 512;
SYS_STACK_SIZE = 512;
IRQ_STACK_SIZE = 512;

USER_STACKS_SIZE = 0x100000;

SECTIONS {

	.text : {
	      build/init.o(.text*)
	} > RAM

	.text : {
	      *(.text*)
	} > RAM

     __bss_start = .;
    .bss : {
    	 *(.bss*)
    } > RAM
    __bss_end__ = .;

    __rodata_start__ = .;
    .rodata : {
	 *(.rodata*)
    } > RAM
    __rodata_end__ = .;

    __data_start__ = .;
    .data : {
    	 *(.data)
    } > RAM
    __data_end__ = .;

    .stack : {

       __stack_start__ = .; 
	   . += KERNEL_STACK_SIZE ;
	   . = ALIGN(4);
	   __svc_stack_end__ = .;
	   
	   . += SYS_STACK_SIZE ;
	   . = ALIGN(4);
	   __sys_stack_end__ = .;

       . += IRQ_STACK_SIZE ;
	   . = ALIGN(4);
	   __irq_stack_end__ = .;

   	   __stacks_end__ = .;
	   
    } > RAM

    . = ALIGN(4096);
    __kernel_heap_start__ = .;
	. = 0xFFFFFF;
    __kernel_heap_end__ = .;
    . = ALIGN(4096);
    __after_kernel_heap__ = .;

    . = 0x20000000;
    .devices : {
        __devices_start__ = .;
        . += 0xFFFFFF;
        __devices_end__ = .;
    } > PERIPHERALS
}
