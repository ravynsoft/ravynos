@ Test to ensure that a ARM to ARM call exceeding 32Mb generates an error
@ if the destination is of type STT_SECTION (eg non-global symbol)

	.global _start
	.syntax unified

@ We will place the section .text at 0x1000.

	.text

_start:
	bl bar

@ We will place the section .foo at 0x2001020.

	.section .foo, "xa"

bar:
	bx lr

