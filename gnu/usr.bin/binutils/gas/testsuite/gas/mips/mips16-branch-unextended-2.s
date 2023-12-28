	.set	mips16
	.set	autoextend
foo:
	beqz.t	$2, baz
	bnez.t	$3, baz
	bteqz.t	baz
	btnez.t	baz
	b.t	baz
