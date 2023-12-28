@ Test to ensure that a Thumb-2 B.W can branch to an ARM function.

	.arch armv7-a
	.global _start
	.syntax unified
	.text
	.thumb_func

_start:
	b.w bar

@ Put this in a separate section to force the assembler to generate a reloc

	.arm
	.section .after, "xa"
	.global bar
	.type bar, %function
bar:
	bx lr

