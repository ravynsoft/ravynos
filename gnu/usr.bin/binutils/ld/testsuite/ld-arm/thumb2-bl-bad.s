@ Test to ensure that a Thumb-2 BL with an oversize offset  makes the linker generate a stub.

	.arch armv7
	.global _start
	.syntax unified

@ We will place the section .text at 0x1000.

	.text
	.thumb_func

_start:
	bl bar

@ We will place the section .foo at 0x100100c.

	.section .foo, "xa"
	.thumb_func

bar:
	bx lr

