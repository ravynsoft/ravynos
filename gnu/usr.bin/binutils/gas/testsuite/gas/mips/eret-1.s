foo:
	li	$t4, 0
	eret
	b	2f
1:	li	$t2, 3
	eret
2:	li	$a0, 0
	deret
	beq	$t3, $t2, 1b
	deret
	eret
	eret
	b	2b

	.set noreorder
	li	$t4, 4
	deret
	li	$t4, 3
	eret
	b	1f
	 li	$t4, 3
	eret
	nop
	b	1f
	 li	$t4, 3
1:	li	$t4, 3
	eret

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
        .space	8
