	.syntax unified
	.globl _start
	.globl func_to_branch_to

	.arm
	.text
func_to_branch_to:
	bx lr

	.thumb
	.section .foo, "xa"
	.thumb_func
_start:
	bl func_to_branch_to

