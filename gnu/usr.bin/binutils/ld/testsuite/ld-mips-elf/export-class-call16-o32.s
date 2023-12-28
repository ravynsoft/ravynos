	.text
	.balign		16
	.xdef		foo
	.ent		foo
foo:
	.frame		$29, 8, $31
	.mask		0x80000000, -4
	.set		noreorder
	.cpload		$25
	.set		reorder
	addiu		$29, -8
	sw		$31, 4($29)
	.cprestore	0
	jal		protected_foo
	jal		hidden_foo
	jal		internal_foo
	lw		$31, 4($29)
	addiu		$29, 8
	jr		$31
	.end		foo
	.balign		4
	.space		8
