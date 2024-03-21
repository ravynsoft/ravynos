#NO_APP
	.text
	.align	2
	.globl	foo
	.type	foo, @function
foo:
	pea x@TLSGD(%a5)
	.size	foo, .-foo
	.globl	x
	.hidden x
	.section	.tdata,"awT",@progbits
	.align	2
	.type	x, @object
	.size	x, 4
x:
	.long	0
	.section	.note.GNU-stack,"",@progbits
