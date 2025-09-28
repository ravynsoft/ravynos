	.syntax unified
	.cpu cortex-a8
	.text
	.thumb
	.thumb_func
	.align 3
	.global targetfn
	.type targetfn, %function
targetfn:
	bx lr
