SECTIONS {
	.text : { *(.text) }
	.data : { *(.data .rw) }
	.bss : { *(.bss) *(COMMON) }
}
defined1 = defined;
