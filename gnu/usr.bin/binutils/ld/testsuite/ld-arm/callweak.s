	.syntax unified
	.arch armv6
	.weak bar
	.section .far, "ax", %progbits
	.global	_start
	.type	_start, %function
_start:
	bl bar
	bleq bar
	.thumb
	.type foo, %function
	.thumb_func
foo:
	bl bar
	movs r0, #0
	bl bar
	bx lr
