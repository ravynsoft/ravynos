	.text
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
	call	*foo@GOTPCREL(%rip)
	jmp	*foo@GOTPCREL(%rip)
	add	foo@GOTPCREL(%rip), %rax
	movq    foo@GOTPCREL(%rip), %rax
	test	%rax, foo@GOTPCREL(%rip)
	movq    bar@GOTPCREL(%rip), %rax
	.type foo, %gnu_indirect_function
foo:
	ret
	.type bar, %function
bar:
	ret
