	.data
	.globl foo
foo:
	.quad -1
	.text
	.globl	_start
	.type	_start, @function
_start:
	pushq	foo@GOTPCREL(%rip)
