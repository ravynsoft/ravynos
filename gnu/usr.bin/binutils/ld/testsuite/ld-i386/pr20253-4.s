	.text
	.type foo,%gnu_indirect_function
foo:
	ret
	.globl _start
_start:
	movl	__start@GOT(%eax), %eax
	.globl __start
__start:
	.data
	.dc.a foo
