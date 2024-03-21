@ Test to ensure that ARM calls exceeding 32Mb generate stubs.

	.global _start
	.syntax unified

@ We will place the section .text at 0x1000.

	.text

_start:
	bl bar
	bl bar2


@ We will place the section .mytext at 0x2000.

	.section .mytext, "xa"
	bl bar3
	bl bar4
	bl bar5

@ We will place the section .foo at 0x2003020.

	.section .foo, "xa"

	.global bar
	.thumb_func
bar:
	bx lr

	.arm
	.global bar2
	.type bar2, %function
bar2:
	bx lr

	.global bar3
	.type bar3, %function
bar3:
	bx lr

	.global bar4
	.thumb_func
bar4:
	bx lr

	.global bar5
	.type bar5, %function
bar5:
	bx lr

