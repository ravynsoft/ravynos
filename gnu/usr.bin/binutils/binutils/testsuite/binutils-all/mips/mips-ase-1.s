	.module	mips64r2
	.module	fp=64
	.set	mdmx
	.set	mips3d
foo:
	add.qh	$v0, $v1, $v2
	add.ps	$f3, $f4, $f5
	addr.ps	$f6, $f7, $f8

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
