	.globl	_start
_start:
	bl	elsewhere
	lis 9,elsewhere@ha
        la 0,elsewhere@l(9)
	bl	undefined
	b	.

	.section .far,"ax",@progbits
elsewhere:
	bl	_start
	b	.

	.section .pad
	.space 0x4000000
