	.equ	$fprel, 2
	.set	mips16

	.ent	foo
foo:
	move	$2,$gp

	# Test various forms of relocation syntax.

	li	$4,(%hi gvar)
	sll	$4,16
	addiu	$4,(%lo (gvar))
	lw	$4,%lo gvar($5)

	# Check that registers aren't confused with $ identifiers.

	lw	$4,($fprel)($17)

	# Check various forms of paired relocations.

	lw	$4,%got(lvar)($2)
	sb	$5,%lo(lvar)($4)

	lw	$4,%got(lvar)($2)
	addiu	$4,%lo(lvar)

	# Check individual relocations.

	lw	$3,%call16(gfunc)($2)
	addiu	$4,%call16(gfunc)

	lw	$4,%gprel(gvar)($2)
	sw	$4,%gprel(gvar)($2)
	addiu	$4,%gprel(gvar)

	# Check the alternative form.

	lw	$4,%gp_rel(gvar)($2)
	sw	$4,%gp_rel(gvar)($2)
	addiu	$4,%gp_rel(gvar)

	.align	6
	.end	foo

	.data
lvar:	.word	1,2
