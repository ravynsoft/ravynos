	
	.section .ctors, "aw"
	.space 0x10, 1
	
	.section .dtors, "aw"
	.space 0x10, 2

	.section .jcr, "aw"
	.space 0x8, 3
	
	.section .data.rel.ro
	.p2align 4
	.space 0x20, 3

	.section .dynamic
	.space 0xb0, 2

	.section .got
	.space 0x2a8, 4
	
	.bss
	.zero 0x840


