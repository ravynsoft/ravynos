	.text
	.type foo, %gnu_indirect_function
	.type	foo, @function
foo:
	ret
	.size	foo, .-foo
	.type start,"function"
	.global start
start:
	.type _start,"function"
	.global _start
_start:
	.type __start,"function"
	.global __start
__start:
	.type __start,"function"
	call	foo@PLT
	movq    foo@GOTPCREL(%rip), %rax
