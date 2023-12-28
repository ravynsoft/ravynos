	.set	noreorder
	.abicalls
	.global	f1
	.ent	f1
f1:
	.option	pic0
	jal	f3
	.option	pic2
	lui	$2,%hi(f2)
	jr	$31
	addiu	$2,$2,%lo(f2)
	.end	f1

	.global	f2
	.ent	f2
f2:
	lui	$28,%hi(%neg(%gp_rel(f2)))
	addu	$28,$28,$25
	addiu	$28,$28,%lo(%neg(%gp_rel(f2)))
	lw	$25,%call16(extf1)($28)
	lw	$4,%got_disp(extf2)($28)
	lw	$5,%got_disp(extd1)($28)
	jalr	$25
	lw	$6,%got_disp(extd2)($28)
	lw	$25,%call16(extf3)($28)
	jr	$25
	lw	$4,%got_disp(extf4)($28)
	.end	f2

	.global	f3
	.ent	f3
f3:
	jr	$31
	nop
	.end	f3

	.data
	.word	extd1
	.word	extd3
