	.syntax unified
	.thumb
	.text
	.align 2
	.global foo
foo:
	svc #0
	nop
