foo:
	.set	noreorder
1:	eret
	.set	reorder
	b	1b

1:	eret
	.set	noreorder
	b	1b
	.set	reorder
	 srlv	$0, $0, $0

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
        .space	8
