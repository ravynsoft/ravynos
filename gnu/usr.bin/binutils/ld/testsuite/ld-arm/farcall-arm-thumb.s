@ Test to ensure that a ARM to Thumb call exceeding 32Mb generates a stub.

	.global _start
	.global bar
	.syntax unified

@ We will place the section .text at 0x1000.

	.text

_start:
	bl bar

@ We will place the section .foo at 0x2001010.

	.section .foo, "xa"
	.thumb_func
bar:
	bx lr

