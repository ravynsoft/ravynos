	.syntax unified

	.section .text, "ax"

	.align 5
	.globl _start
	.thumb_func
_start:
	.rept 13
	nop
	.endr
	eor	r0, r1, r2
	b.w	_start

