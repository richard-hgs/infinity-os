ENTRY(_start);
OUTPUT_FORMAT(elf32-i386);
OUTPUT_ARCH(i386:i386);
__BASE_ADDR = 0x00010000;

SECTIONS
{
	.text __BASE_ADDR : AT(__BASE_ADDR)				/* Code section */
	{
		code = .;
		*(.text)
		*(.text.*)
	}
	.rodata : AT(__BASE_ADDR + (rodata - code))		/* Read Only data section */
	{
		rodata = .;
		*(.rodata)
		*(.rodata.*)
	}
	.data : AT(__BASE_ADDR + (data - code))			/* Data section */
	{
		data = .;
		*(.data)
		*(.data.*)
	}
	.bss : AT(__BASE_ADDR + (bss - code))			/* Uninitialized data section */
	{
		bss = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		__MAX_ADDR = .;
	}
	/DISCARD/ :
	{
		*(.comment);
		*(.igot.*);
		*(.eh_frame);
	}
}
