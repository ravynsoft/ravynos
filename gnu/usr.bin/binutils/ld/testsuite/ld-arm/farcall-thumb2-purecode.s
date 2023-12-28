@ Test to ensure that a purecode Thumb2 call exceeding 4Mb generates a stub.

	.global _start
	.syntax unified
	.arch armv7-m
	.thumb
	.thumb_func

@ We will place the section .text at 0x1000.

	.text
bar:
	bx lr

@ We will place the section .foo at 0x02001014.

	.section .foo, "0x20000006"
_start:
	bl bar
