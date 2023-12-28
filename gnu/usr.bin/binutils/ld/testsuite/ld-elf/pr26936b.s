	.text
	.type start,"function"
	.global start
start:
	.type _start,"function"
	.global _start
_start:
	.type __start,"function"
	.global __start
__start:
	.type main,"function"
	.global main
main:
	.type _main,"function"
	.global _main
_main:
	.nop
	.section .text.__x86.get_pc_thunk.bx,"axG",%progbits,__x86.get_pc_thunk.bx,comdat
	.globl  __x86.get_pc_thunk.bx
	.hidden __x86.get_pc_thunk.bx
	.type   __x86.get_pc_thunk.bx, %function
__x86.get_pc_thunk.bx:
	.nop
