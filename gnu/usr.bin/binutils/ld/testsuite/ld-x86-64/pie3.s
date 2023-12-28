	.text
	.global _start
	.weak foo
_start:
	leaq	foo(%rip), %rax
