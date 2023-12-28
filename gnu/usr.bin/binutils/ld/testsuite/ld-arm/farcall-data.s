	.syntax unified
	.text
	.global _start
	.type _start, %function
_start:
    b far

.section .after
after:
	.word 0x11111111

	.section .far, "ax"
	.type far, %function
far:	bx lr
