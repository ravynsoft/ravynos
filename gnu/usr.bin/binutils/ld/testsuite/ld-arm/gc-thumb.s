	.arch	armv7-a
	.syntax	unified
	.text
	.globl	_start
_start:
	mov	pc,lr

	.section .text.foo,"ax",%progbits
	.thumb
	movw	r0,#:lower16:foo-.
	movt	r0,#:upper16:foo-.
