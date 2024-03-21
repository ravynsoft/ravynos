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
	auipc	x1, %pcrel_hi (foo_addr)
.ifdef __64_bit__
	ld	x1, %pcrel_lo (.L1) (x1)
.else
	lw	x1, %pcrel_lo (.L1) (x1)
.endif
	ret
	.size	bar, .-bar

	.data
foo_addr:
.ifdef __64_bit__
	.quad	foo
.else
	.long	foo
.endif
