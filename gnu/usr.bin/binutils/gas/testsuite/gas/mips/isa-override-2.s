	.text
	.globl	foo
	.ent	foo
foo:
	dli	$2, 0x9000000080000000
	.set	push
	.set	mips3
	dli	$2, 0x9000000080000000
	.set	mips0
	dli	$2, 0x9000000080000000
	.set	mips3
	.set	pop
	dli	$2, 0x9000000080000000
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
