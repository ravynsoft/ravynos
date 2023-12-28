	.syntax unified
	.arch armv7e-m
	.thumb
	.text
	.align	2
	.global	f2
	.thumb
	.thumb_func
	.type	f2, %function
f2:
	movs	r0, #0
	bx	lr
	.size	f2, .-f2
