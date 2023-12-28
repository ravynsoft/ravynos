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
        adrp    x0, .LANCHOR0
        add     x0, x0, :lo12:.LANCHOR0
        .data
        .align  3
.LANCHOR0 = . + 0
        .xword  foo
