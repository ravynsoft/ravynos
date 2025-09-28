SECTIONS {
	.text : { *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss) *(COMMON) }
}
sym1 = 0x42;
sym2 = 0x43;
defined = DEFINED (sym1) ? sym1 : sym2;
