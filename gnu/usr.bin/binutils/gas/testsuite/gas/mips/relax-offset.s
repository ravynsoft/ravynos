	.abicalls

	.align	4, 0
	.globl	foo
	.ent	foo
foo:
	.set	noreorder
	.cpload	$25
	.set	reorder
	beq	$4, $0, bar + 4
	jr	$31

	.space	131072

	.align	4, 0
bar:
	syscall
	jr	$31
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
