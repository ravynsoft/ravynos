	.text
	.globl	foo
	.ent	foo
foo:
	ld	$2, 0($4)
	or	$2, 0x89ab0000
	l.d	$f2, 0($4)
	li.d	$f2, 1.0000005128531484
	.set	push
	.set	mips3
	ld	$2, 0($4)
	or	$2, 0x89ab0000
	dli	$2, 0x9000000080000000
	l.d	$f2, 0($4)
	li.d	$f2, 1.0000005128531484
	.set	mips0
	ld	$2, 0($4)
	or	$2, 0x89ab0000
	l.d	$f2, 0($4)
	li.d	$f2, 1.0000005128531484
	.set	mips3
	.set	pop
	ld	$2, 0($4)
	or	$2, 0x89ab0000
	l.d	$f2, 0($4)
	li.d	$f2, 1.0000005128531484
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
