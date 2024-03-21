	.text
.globl _start
	.type	_start, @function
_start:
	divl	x@GOT(%ecx), %eax
	.size	_start, .-_start
.globl x
	.data
	.align 4
	.type	x, @object
	.size	x, 4
x:
	.long	2
