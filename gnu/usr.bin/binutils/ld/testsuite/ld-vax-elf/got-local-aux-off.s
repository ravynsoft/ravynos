	.globl	baz
	.type	baz, @object
baz:
	.byte	0, 1, 2
	.size	baz, . - baz
