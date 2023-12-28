	.syntax unified
	.text
	.global _start
	.type _start, %function
_start:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend
