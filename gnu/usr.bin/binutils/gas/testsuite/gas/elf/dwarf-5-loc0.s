	.file "foo.c"
	.text
	.balign 16
	.globl _start
_start:
	.file 0 "foo.c"
	.quad 0
	.loc 0 1 view 0
	.balign 16
	.loc 0 2 view 0
	.size _start, .-_start
