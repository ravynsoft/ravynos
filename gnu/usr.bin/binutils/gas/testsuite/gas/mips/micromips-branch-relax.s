	.text
	.set	micromips
	.set	noreorder
test:
	b32	test
	addu	$3, $4, $5
	beqz32	$3, test
	addu	$3, $4, $5
	bnez32	$3, test
	addu	$3, $4, $5
	b	test
	addu	$3, $4, $5
	bc	test
	addu	$3, $4, $5
	bal	test
	addu	$3, $4, $5
	.ifndef	insn32
	bals	test
	addu	$3, $4, $5
	.endif
	beqz	$3, test
	addu	$3, $4, $5
	bnez	$3, test
	addu	$3, $4, $5
	.ifndef	insn32
	b16	test2
	addu	$3, $4, $5
	beqz16	$3, test2
	addu	$3, $4, $5
	bnez16	$3, test2
	addu	$3, $4, $5
	.endif
	b	test2
	addu	$3, $4, $5
	bc	test2
	addu	$3, $4, $5
	bal	test2
	addu	$3, $4, $5
	.ifndef	insn32
	bals	test2
	addu	$3, $4, $5
	.endif
	beqz	$3, test2
	addu	$3, $4, $5
	bnez	$3, test2
	addu	$3, $4, $5
	.ifndef	insn32
	b16	test3
	addu	$3, $4, $5
	beqz16	$3, test3
	addu	$3, $4, $5
	bnez16	$3, test3
	addu	$3, $4, $5
	.endif
	b32	test2
	addu	$3, $4, $5
	bc32	test2
	addu	$3, $4, $5
	bal32	test2
	addu	$3, $4, $5
	.ifndef	insn32
	bals32	test2
	addu	$3, $4, $5
	.endif
	beqz32	$3, test2
	addu	$3, $4, $5
	bnez32	$3, test2
	addu	$3, $4, $5
	j	test3
	addu	$3, $4, $5
	jal	test3
	addu	$3, $4, $5
	b	test3
	addu	$3, $4, $5
	bc	test3
	addu	$3, $4, $5
	bal	test3
	addu	$3, $4, $5
	.ifndef	insn32
	bals	test3
	addu	$3, $4, $5
	.endif
	beq	$3, $4, test3
	addu	$3, $4, $5
	bne	$3, $4, test3
	addu	$3, $4, $5
	bltz	$3, test3
	addu	$3, $4, $5
	bgez	$3, test3
	addu	$3, $4, $5
	blez	$20, test3
	addu	$3, $4, $5
	bgtz	$20, test3
	addu	$3, $4, $5
	beqzc	$3, test3
	addu	$3, $4, $5
	bnezc	$3, test3
	addu	$3, $4, $5
	bgezal	$30, test3
	addu	$3, $4, $5
	bltzal	$30, test3
	addu	$3, $4, $5
	.ifndef	insn32
	bgezals	$30, test3
	addu	$3, $4, $5
	bltzals	$30, test3
	addu	$3, $4, $5
	.endif
	bc1f	test3
	addu	$3, $4, $5
	bc1t	test3
	addu	$3, $4, $5
	bc2f	test3
	addu	$3, $4, $5
	bc2t	test3
	addu	$3, $4, $5
	beql	$3, $4, test3
	addu	$3, $4, $5
	beqz	$3, test3
	xor	$3, $4, $5
	bge	$3, $4, test3
	xor	$3, $4, $5
	bgel	$3, $4, test3
	xor	$3, $4, $5
	bgeu	$3, $4, test3
	xor	$3, $4, $5
	bgeul	$3, $4, test3
	xor	$3, $4, $5
	bgezall	$3, test3
	xor	$3, $4, $5
	bgezl	$3, test3
	xor	$3, $4, $5
	bgt	$3, $4, test3
	xor	$3, $4, $5
	bgtl	$3, $4, test3
	xor	$3, $4, $5
	bgtu	$3, $4, test3
	xor	$3, $4, $5
	bgtul	$3, $4, test3
	xor	$3, $4, $5
	bgtzl	$3, test3
	xor	$3, $4, $5
	ble	$3, $4, test3
	xor	$3, $4, $5
	blel	$3, $4, test3
	xor	$3, $4, $5
	bleu	$3, $4, test3
	xor	$3, $4, $5
	bleul	$3, $4, test3
	xor	$3, $4, $5
	blezl	$3, test3
	xor	$3, $4, $5
	blt	$3, $4, test3
	xor	$3, $4, $5
	bltl	$3, $4, test3
	xor	$3, $4, $5
	bltu	$3, $4, test3
	xor	$3, $4, $5
	bltul	$3, $4, test3
	xor	$3, $4, $5
	bltzall	$3, test3
	xor	$3, $4, $5
	bltzl	$3, test3
	xor	$3, $4, $5
	bnel	$3, $4, test3
	xor	$3, $4, $5
	bnez	$3, test3
	xor	$3, $4, $5
	bnezl	$3, test3
	xor	$3, $4, $5

	.skip	511 << 1
test2:
	.insn

	.skip	(32767 - 511) << 1
test3:
	addu	$3, $4, $5

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
