	.syntax unified
	.thumb
foo:
	cmp r0, r1
	beq foo
