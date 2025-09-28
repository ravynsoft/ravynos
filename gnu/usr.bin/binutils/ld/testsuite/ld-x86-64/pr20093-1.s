	.section	.lbss,"aw",@nobits
foo1:
	.space 1073741824
	.space 1073741824
	.space 1073741824
	.text
	.globl	_start
	.type	_start, @function
_start:
	movq	foo1@GOTPCREL(%rip), %rax
	.size	_start, .-_start
