	.section .text._start,"ax",%progbits
	.globl	_start
	.type	_start, %function
_start:
	.byte 0
	.size	_start, .-_start

	.section .text.bar,"ax",%progbits
	.globl	foo
	.type	foo, %function
foo:
	.dc.a bar
	.size	foo, .-foo
