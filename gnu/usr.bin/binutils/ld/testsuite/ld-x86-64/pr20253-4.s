	.text
	.type foo,%gnu_indirect_function
foo:
	ret
	.globl _start
_start:
	movq	__start@GOTPCREL(%rip), %rax
	.globl __start
__start:
	.data
	.dc.a foo
