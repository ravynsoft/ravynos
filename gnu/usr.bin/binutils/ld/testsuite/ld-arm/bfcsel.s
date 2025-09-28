	.global _start
	.syntax unified

@ We will place the section .text at 0x1000.

	.text
	.thumb_func

_start:
	bfcsel .LB1, bar, .LB2, eq
	mov r3, r4
	mov r1, r2
.LB1:
	beq .LB2
.LB2:
	mov r3, r2

@ We will place the section .foo at 0x1001000.

	.section .foo, "xa"
	.thumb_func

bar:
	bx lr

