	.set	mips16
	.ent	foo
foo:
	move	$2,$28
	lw	$4,%got(l1)($2)
	addiu	$4,%lo(l1)

	lw	$4,%got(l1 + 16)($2)
	addiu	$4,%lo(l1 + 16)

	lw	$4,%got(l1 + 0x7fec)($2)
	addiu	$4,%lo(l1 + 0x7fec)

	lw	$4,%got(l1 + 0x7ff0)($2)
	addiu	$4,%lo(l1 + 0x7ff0)

	lw	$4,%got(l1 + 0xffec)($2)
	addiu	$4,%lo(l1 + 0xffec)

	lw	$4,%got(l1 + 0xfff0)($2)
	addiu	$4,%lo(l1 + 0xfff0)

	lw	$4,%got(l1 + 0x18000)($2)
	addiu	$4,%lo(l1 + 0x18000)

	lw	$4,%got(l2)($2)
	addiu	$4,%lo(l2)

	lw	$4,%got(l2 + 0xfff)($2)
	addiu	$4,%lo(l2 + 0xfff)

	lw	$4,%got(l2 + 0x1000)($2)
	addiu	$4,%lo(l2 + 0x1000)

	lw	$4,%got(l2 + 0x12345)($2)
	addiu	$4,%lo(l2 + 0x12345)

	lw	$4,%gprel(l3)($2)
	lw	$4,%gprel(l3 + 4)($2)
	lw	$4,%gprel(l4)($2)
	lw	$4,%gprel(l4 + 4)($2)
	lw	$4,%gprel(l5)($2)
	lw	$4,%gprel(l5 + 8)($2)
	lw	$4,%gprel(l5 + 12)($2)

	.align	6
	.end	foo

	.data
	.word	1,2,3,4
l1:	.word	4,5
	.space	0x1f000 - 24
l2:	.word	7,8

	.sdata
l3:	.word	1
l4:	.word	2
	.word	3
l5:	.word	4
