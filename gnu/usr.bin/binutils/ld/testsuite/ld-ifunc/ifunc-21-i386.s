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
	call	*foo@GOT(%ebx)
	jmp	*foo@GOT(%ebx)
	add	foo@GOT(%ebx), %eax
	mov	foo@GOT(%ebx), %eax
	test	%eax, foo@GOT(%ebx)
	mov	bar@GOT(%ebx), %eax
	.type	foo, %gnu_indirect_function
foo:
	ret
	.type	bar, %function
bar:
	ret
