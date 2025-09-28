	.text

	.ent	foo
	.set	mips16
foo:
	ld	$4, $3($2)
	ld	$4, $3($pc)
	ld	$4, $3($sp)
	lw	$4, $3($2)
	lw	$4, $3($pc)
	lw	$4, $3($sp)
	lwu	$4, $3($2)
	lh	$4, $3($2)
	lhu	$4, $3($2)
	lb	$4, $3($2)
	lbu	$4, $3($2)

	sd	$4, $3($2)
	sd	$4, $3($sp)
	sd	$ra, $3($sp)
	sw	$4, $3($2)
	sw	$4, $3($sp)
	sw	$ra, $3($sp)
	sh	$4, $3($2)
	sb	$4, $3($2)

	addiu	$3, $2
	addiu	$4, $3, $2
	addiu	$3, $pc, $2
	addiu	$sp, $2
	addiu	$3, $sp, $2

	daddiu	$3, $2
	daddiu	$4, $3, $2
	daddiu	$3, $pc, $2
	daddiu	$sp, $2
	daddiu	$3, $sp, $2

	slti	$3, $2
	sltiu	$3, $2

	cmpi	$3, $2
	cmp	$3, $2
	li	$3, $2

	sll	$3, $2, $2
	sra	$3, $2, $2
	srl	$3, $2, $2
	dsll	$3, $2, $2
	dsra	$3, $2
	dsrl	$3, $2

	break	$2
	sdbbp	$2

	b	$2
	beqz	$3, $2
	bnez	$3, $2
	bteqz	$2
	btnez	$2

	jal	$2
	jalx	$2

	save	$31, $16, $2
	restore	$31, $16, $2

	asmacro	0, 0, 0, 0, 0, $2
	asmacro	0, 0, 0, 0, $2, 0
	asmacro	0, 0, 0, $2, 0, 0
	asmacro	0, 0, $2, 0, 0, 0
	asmacro	0, $2, 0, 0, 0, 0
	asmacro	$2, 0, 0, 0, 0, 0

	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
