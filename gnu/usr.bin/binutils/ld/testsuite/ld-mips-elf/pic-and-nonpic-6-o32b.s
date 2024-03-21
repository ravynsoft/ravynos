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
	.cpload	$25
	lw	$25,%call16(extf1)($28)
	lw	$4,%got(extf2)($28)
	lw	$5,%got(extd1)($28)
	jalr	$25
	lw	$6,%got(extd2)($28)
	lw	$25,%call16(extf3)($28)
	jr	$25
	lw	$4,%got(extf4)($28)
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
