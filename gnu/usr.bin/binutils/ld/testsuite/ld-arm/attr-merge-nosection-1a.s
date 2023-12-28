	.cpu cortex-m0
	.fpu softvfp
	.syntax unified
	.thumb
	.text
	.global foo
	.thumb_func
	.type foo, %function
foo:
	bx lr
