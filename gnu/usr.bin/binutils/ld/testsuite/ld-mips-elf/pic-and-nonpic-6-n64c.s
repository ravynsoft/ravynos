	.abicalls
	.option	pic0
	.set	noreorder
	.global	__start
	.ent	__start
__start:
	jal	f1
	nop
	lui	$2,%hi(f2)
	addiu	$2,$2,%lo(f2)
	jal	extf3
	nop
	jal	extf4
	nop
	jal	extf5
	nop
	lui	$2,%hi(extd2)
	addiu	$2,$2,%lo(extd2)
	lui	$2,%hi(extd3)
	addiu	$2,$2,%lo(extd3)
	.end	__start

	.data
	.word	extd2
	.word	extd4
