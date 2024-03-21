	.text
	.globl _start
_start:
	.space 16

	.section .rodata,"a"
	.globl foo
foo:
	.dc.a __ehdr_start
