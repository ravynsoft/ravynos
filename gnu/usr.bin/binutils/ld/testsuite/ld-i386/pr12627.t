OUTPUT_ARCH(i386)
EXTERN(_start)
ENTRY(_start)

SECTIONS
{
	. = 0x1000;

	.bss16 (NOLOAD) : {
		__bss16_start = .;
		*(.bss16)
		__bss16_end = .;
	}
	__bss16_len = __bss16_end - __bss16_start;
	__bss16_dwords = (__bss16_len + 3) >> 2;

	. = 0x8000;

	.text16 : {
		FILL(0x90909090)
		__text16_start = .;
		*(.text16)
		*(.text16.*)
		__text16_end = .;
	}
}
