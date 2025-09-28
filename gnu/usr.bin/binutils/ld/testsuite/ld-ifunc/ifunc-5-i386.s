	.text
	.type foo, %gnu_indirect_function
.globl foo
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
	call	.L6
.L6:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L6], %ebx
	call	foo@PLT
	leal	foo@GOT(%ebx), %eax
