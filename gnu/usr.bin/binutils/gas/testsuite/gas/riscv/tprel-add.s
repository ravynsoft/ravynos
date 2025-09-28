	# Don't allow tprel_add in amoadd.
	amoadd.w x8,x9,%tprel_add(i)(x10)
	# Do require tprel_add in 4-operand add.
	add	a5,a5,tp,0
	.globl	i
	.section	.tbss,"awT",@nobits
	.align	2
	.type	i, @object
	.size	i, 4
i:
	.zero	4
