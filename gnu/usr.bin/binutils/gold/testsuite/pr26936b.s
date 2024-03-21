	.text
	.type _start,"function"
	.global _start
_start:
	.nop
	.section .text.__x86.get_pc_thunk.bx,"axG",%progbits,__x86.get_pc_thunk.bx,comdat
	.globl  __x86.get_pc_thunk.bx
	.hidden __x86.get_pc_thunk.bx
	.type   __x86.get_pc_thunk.bx, %function
__x86.get_pc_thunk.bx:
	.nop
