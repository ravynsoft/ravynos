@ Test to ensure that a Thumb to ARM call exceeding 4Mb generates a stub.
@ Check that we can generate two types of stub in the same section.

	.global _start
	.syntax unified

@ We will place the section .text at 0x1c01010.

	.text
	.thumb_func
_start:
	.global bar
	bl bar
@ This call is close enough to generate a "short branch" stub
@ or no stub if blx is available.
	.space 0x0300000
	bl bar

@ We will place the section .foo at 0x2001014.

	.section .foo, "xa"

	.arm
	.type bar, %function
bar:
	bx lr

