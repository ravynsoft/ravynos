	.arch armv7
	.syntax unified
	.thumb
foo:
	bl	1f
1:	it	cc
	bcc.w	.+0xe0c
	bx	lr
