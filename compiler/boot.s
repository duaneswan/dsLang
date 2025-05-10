/* boot.s - Multiboot-compatible boot code for dsOS
 *
 * This assembly file provides the entry point and boot setup code for dsOS.
 * It includes a Multiboot-compliant header that allows dsOS to be loaded by
 * GRUB or other Multiboot-compliant bootloaders. The code executes initial
 * setup and then jumps to the kernel_main function written in dsLang.
 */

/* Multiboot header constants */
.set ALIGN,     1<<0                   /* Align loaded modules on page boundaries */
.set MEMINFO,   1<<1                   /* Provide memory map */
.set FLAGS,     ALIGN | MEMINFO        /* This is the Multiboot 'flag' field */
.set MAGIC,     0x1BADB002             /* 'magic number' lets bootloader find the header */
.set CHECKSUM,  -(MAGIC + FLAGS)       /* Checksum of above to prove we are multiboot */

/* Multiboot header section */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Reserve a small stack for the initial boot */
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 /* 16 KiB */
stack_top:

/* .text section */
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the stack */
    mov $stack_top, %esp
    
    /* Reset EFLAGS */
    pushl $0
    popf

    /* Push the pointer to the Multiboot information structure */
    pushl %ebx
    
    /* Push the Multiboot magic value */
    pushl %eax
    
    /* Call the global constructors */
    call _init
    
    /* Transfer control to the kernel */
    call kernel_main
    
    /* If kernel_main returns, enter an infinite loop */
.hang:
    cli
    hlt
    jmp .hang
.size _start, . - _start

/* This is for 64-bit mode when we get there */
.section .text
.global _start64
.type _start64, @function
_start64:
    /* Set up the stack (using 64-bit register) */
    mov $stack_top, %rsp
    
    /* Reset RFLAGS */
    pushq $0
    popf
    
    /* Call the global constructors (if any) */
    call _init
    
    /* x86-64 calling convention passes args in registers:
     * RDI, RSI, RDX, RCX, R8, R9 for first 6 args
     */
    
    /* Call kernel_main */
    call kernel_main
    
    /* If kernel_main returns, enter an infinite loop */
    cli
    hlt
    jmp .
.size _start64, . - _start64

/* Standard entry point for C/C++ runtime */
.global _init
.type _init, @function
_init:
    /* For now, just return since we don't have global constructors */
    ret
.size _init, . - _init

/* Declare kernel_main symbol as external */
.extern kernel_main
