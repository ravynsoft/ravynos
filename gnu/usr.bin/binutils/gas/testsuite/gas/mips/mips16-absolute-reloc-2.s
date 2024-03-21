	.text

	.space	0x1000

	.globl	foo
	.ent	foo
	.set	mips16
foo:
	addiu	$2, bar
	addiu	$2, $3, bar
	addiu	$2, $pc, bar
	addiu	$sp, bar
	addiu	$2, $sp, bar
	cmpi	$2, bar
	lb	$2, bar($3)
	lbu	$2, bar($3)
	lh	$2, bar($3)
	lhu	$2, bar($3)
	li	$2, bar
	lw	$2, bar($3)
	lw	$2, bar($pc)
	lw	$2, bar($sp)
	sb	$2, bar($3)
	sh	$2, bar($3)
	sll	$2, $3, bar
	slti	$2, bar
	sltiu	$2, bar
	sra	$2, $3, bar
	srl	$2, $3, bar
	sw	$2, bar($3)
	sw	$2, bar($sp)
	sw	$ra, bar($sp)
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
