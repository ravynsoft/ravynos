	.syntax unified
	.cpu cortex-a8
	.text
	.arm
	.align 3
	.global targetfn
	.type targetfn, %function
targetfn:
	bx lr
