	.text

	.type	foo_resolver, @function
foo_resolver:
	ret
	.size	foo_resolver, .-foo_resolver

	.globl	foo1
	.type	foo1, %gnu_indirect_function
	.set	foo1, foo_resolver

	.globl	foo2
	.type	foo2, %gnu_indirect_function
	.set	foo2, foo_resolver

	.globl	bar
	.type	bar, @function
bar:
.L1:
	auipc	x1, %got_pcrel_hi (foo1)
.ifdef __64_bit__
	ld	x1, %pcrel_lo (.L1) (x1)
.else
	lw	x1, %pcrel_lo (.L1) (x1)
.endif

	call	foo1
	call	foo1@plt

.L2:
	auipc	x2, %got_pcrel_hi (foo2)
.ifdef __64_bit__
	ld	x2, %pcrel_lo (.L2) (x2)
.else
	lw	x2, %pcrel_lo (.L2) (x2)
.endif
	ret
	.size	bar, .-bar
