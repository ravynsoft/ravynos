	.syntax unified
	.thumb
	.globl two
two:	
	bl far_fn
	.rept 0x200000
	.long 0
	.endr
