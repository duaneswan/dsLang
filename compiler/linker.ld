/* linker.ld - Linker script for dsOS kernel
 * 
 * This script defines the memory layout for the dsOS kernel binary.
 * It places the multiboot header first, followed by the code and data sections,
 * ensuring proper alignment and organization of the kernel in memory.
 */

/* Entry point of the kernel */
ENTRY(_start)

/* Define the program headers we want in our ELF file */
PHDRS
{
    headers PT_PHDR PHDRS;
    text PT_LOAD FILEHDR PHDRS;
    data PT_LOAD;
}

/* Define the memory sections */
SECTIONS
{
    /* Start at 1 MiB - a conventional place for kernels to be loaded by the bootloader */
    . = 1M;

    /* First put the multiboot header, as it must be within the first 8 KiB */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    } :text

    /* Read-only data section */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    } :text

    /* Read-write data section */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    } :data

    /* BSS section - this contains uninitialized data */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    } :data

    /* Heap section */
    .heap BLOCK(4K) : ALIGN(4K)
    {
        *(.heap)
    } :data

    /* Discard any unused sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
