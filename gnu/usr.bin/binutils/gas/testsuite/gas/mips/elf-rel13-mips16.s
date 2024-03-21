	.set	mips16
	.ent	foo
foo:
	move	$2,$28
	lw	$4,%got(l1)($2)
	lw	$4,%got(l2)($2)
	lw	$4,%got(l3)($2)
	lw	$4,%got(l3)($2)
	lw	$4,%got(l1+0x400)($2)
	addiu	$4,%lo(l1)
	addiu	$4,%lo(l1+0x400)
	addiu	$4,%lo(l3)
	addiu	$4,%lo(l2)
	.align	5
	.end	foo

	.data
l1:	.word	1

	.lcomm	l2, 4

	.rdata
	.word	1
l3:	.word	2
