	.section .foo,"ax",%progbits
	.globl	_start
	.type	_start, %function
_start:
	.byte 0x10
	.section .bar,"ax",%progbits
	.globl	aligned
	.type	aligned, %function
	.p2align 5
aligned:
	.byte 0x20
