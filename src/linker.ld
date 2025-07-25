/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
 
/* We want the symbol _start to be our entry point */
ENTRY(_start)
 
/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
	limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}
 
SECTIONS
{
	/* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
	/* and because that is what the Limine spec mandates. */
	/* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
	/* that is the beginning of the region. */
	. = 0xffffffff80000000;
	
	.limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

	. = ALIGN(CONSTANT(MAXPAGESIZE));
 
	.text : {
		*(.text .text.*)
	} :text
 
	/* Move to the next memory page for .rodata */
	. = ALIGN(CONSTANT(MAXPAGESIZE));
 
	.rodata : {
		*(.rodata .rodata.*)
	} :rodata
 
	/* Move to the next memory page for .data */
	. = ALIGN(CONSTANT(MAXPAGESIZE));
 
	.data : {
		*(.data .data.*)
	} :data

	.memory_map : ALIGN(0x1000)
    {
        KEEP(*(.memory_map))
    } :data
 
	/* NOTE: .bss needs to be the last thing mapped to :data, otherwise lots of */
	/* unnecessary zeros will be written to the binary. */
	/* If you need, for example, .init_array and .fini_array, those should be placed */
	/* above this. */
	.bss : {
		*(COMMON)
		*(.bss .bss.*)
	} :data
 
	/* Discard .note.* and .eh_frame since they may cause issues on some hosts. */
	/DISCARD/ : {
		*(.eh_frame)
		*(.note .note.*)
	}
}