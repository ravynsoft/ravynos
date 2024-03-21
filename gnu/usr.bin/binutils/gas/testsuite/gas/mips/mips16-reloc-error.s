	.text

	.set	bar, 8

	.ent	foo
	.set	mips16
foo:
	li	$2, %hi(bar)
	sll	$2, $2, 16

	addiu	$3, $2, %lo(bar)
	daddiu	$3, $2, %lo(bar)

	slti	$3, %lo(bar)
	sltiu	$3, %lo(bar)

	sll	$3, $2, %lo(bar)
	sra	$3, $2, %lo(bar)
	srl	$3, $2, %lo(bar)
	dsll	$3, $2, %lo(bar)
	dsra	$3, %lo(bar)
	dsrl	$3, %lo(bar)

	break	%lo(bar)
	sdbbp	%lo(bar)

	b	%lo(bar)
	beqz	$3, %lo(bar)
	bnez	$3, %lo(bar)
	bteqz	%lo(bar)
	btnez	%lo(bar)

	jal	%lo(bar)
	jalx	%lo(bar)

	save	$31, $16, %lo(bar)
	restore	$31, $16, %lo(bar)

	asmacro	0, 0, 0, 0, 0, %lo(bar)
	asmacro	0, 0, 0, 0, %lo(bar), 0
	asmacro	0, 0, 0, %lo(bar), 0, 0
	asmacro	0, 0, %lo(bar), 0, 0, 0
	asmacro	0, %lo(bar), 0, 0, 0, 0
	asmacro	%lo(bar), 0, 0, 0, 0, 0

	nop
	.set	nomips16
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
