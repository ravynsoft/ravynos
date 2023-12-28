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
	bl	foo
