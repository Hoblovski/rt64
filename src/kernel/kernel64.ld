/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

/* OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386") */
OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

mboot_load_addr = 0x00100000;

SECTIONS
{
	/* Kernel code / data starts here, 8000_0000..here belong to devices */
	. = 0xFFFFFFFF80100000;

	PROVIDE(begin = .);

	.text : AT(mboot_load_addr) {
		*(.text .rela.text .stub .text.* .gnu.linkonce.t.*)
	}

	PROVIDE(etext = .);	/* Define the 'etext' symbol to this value */

	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	. = ALIGN(0x1000);
	PROVIDE(ubegin = .);
	.user : {
		*(.user.text)
		*(.user.rodata)
		*(.user.bss)
		*(.user.data)
	}
	. = ALIGN(0x1000);
	PROVIDE(uend = .);

	/* Conventionally, Unix linkers provide pseudo-symbols
	 * etext, edata, and end, at the end of the text, data, and bss.
	 * For the kernel mapping, we need the address at the beginning
	 * of the data section, but that's not one of the conventional
	 * symbols, because the convention started before there was a
	 * read-only rodata section between text and data. */
	PROVIDE(data = .);

	/* The data segment */
	.data : {
		*(.data)
	}

	. = ALIGN(0x1000);

	PROVIDE(edata = .);

	/* why cannot put .user here? */

	.bss : {
		*(.bss)
		*(COMMON)
	}

	PROVIDE(end = .);

	/DISCARD/ : {
		*(.eh_frame .rela.eh_frame .note.GNU-stack)
	}
}

mboot_load_end = mboot_load_addr + (edata - begin);
mboot_bss_end = mboot_load_addr + (end - begin);
mboot_entry_addr = mboot_load_addr + (mboot_entry - begin);
