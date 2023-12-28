	.set	mips16
	.set	noautoextend
foo:
	beqz	$2, baz
	bnez	$3, baz
	bteqz	baz
	btnez	baz
	b	baz
