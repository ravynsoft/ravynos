	.global _start
	.syntax unified

@ We will place the section .text at 0x1000.

	.text
	.thumb_func

_start:
	bfl 2, bar

@ We will place the section .foo at 0x1001000.

	.section .foo, "xa"
	.thumb_func

bar:
	bx lr

