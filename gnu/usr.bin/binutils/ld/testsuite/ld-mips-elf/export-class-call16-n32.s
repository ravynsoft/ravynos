	.text
	.balign		16
	.xdef		foo
	.ent		foo
foo:
	.frame		$29, 16, $31
	.mask		0x90000000, -8
	addiu		$29, -16
	sd		$31, 8($29)
	.cpsetup	$25, 0, foo
	jal		protected_foo
	jal		hidden_foo
	jal		internal_foo
	.cpreturn
	ld		$31, 8($29)
	addiu		$29, 16
	jr		$31
	.end		foo
	.balign		4
	.space		8
