	.text
	.type foo,%gnu_indirect_function
foo:
	ret
	.globl _start
_start:
	ret
	.globl __start
__start:
	.data
	.long foo - .
