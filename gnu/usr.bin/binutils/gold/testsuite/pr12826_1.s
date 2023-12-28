	.syntax unified
	.arch armv7e-m
	.thumb
	.text
	.align	2
	.global	f1
	.thumb
	.thumb_func
	.type	f1, %function
f1:
	movs	r0, #0
	bx	lr
	.size	f1, .-f1
