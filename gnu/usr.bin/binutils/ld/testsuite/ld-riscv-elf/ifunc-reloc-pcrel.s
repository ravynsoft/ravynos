	.text

	.type	foo_resolver, @function
foo_resolver:
	ret
	.size	foo_resolver, .-foo_resolver

	.globl	foo
	.type	foo, %gnu_indirect_function
	.set	foo, foo_resolver

	.globl	bar
	.type	bar, @function
bar:
.L1:
	auipc	x1, %pcrel_hi (foo)
	addi	x1, x1, %pcrel_lo (.L1)
.L2:
	auipc	x2, %pcrel_hi (foo)
.ifdef __64_bit__
	ld	x2, %pcrel_lo (.L2) (x2)
.else
	lw	x2, %pcrel_lo (.L2) (x2)
.endif
	ret
	.size	bar, .-bar
