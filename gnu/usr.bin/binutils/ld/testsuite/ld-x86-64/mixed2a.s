	.text
	.p2align 4,,15
.globl _start
	.type	_start, @function
_start:
	movl	foo(%rip), %eax
	.size	_start, .-_start
