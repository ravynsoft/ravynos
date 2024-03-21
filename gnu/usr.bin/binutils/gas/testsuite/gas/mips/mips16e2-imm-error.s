	.set	mips16
foo:
	lw	$2, -32769($gp)
	lw	$2, 32768($gp)
	lh	$2, -32769($gp)
	lh	$2, 32768($gp)
	lhu	$2, -32769($gp)
	lhu	$2, 32768($gp)
	lb	$2, -32769($gp)
	lb	$2, 32768($gp)
	lbu	$2, -32769($gp)
	lbu	$2, 32768($gp)
	sw	$2, -32769($gp)
	sw	$2, 32768($gp)
	sh	$2, -32769($gp)
	sh	$2, 32768($gp)
	sb	$2, -32769($gp)
	sb	$2, 32768($gp)

	ll	$2, -257($gp)
	ll	$2, 256($gp)
	lwl	$2, -257($gp)
	lwl	$2, 256($gp)
	lwr	$2, -257($gp)
	lwr	$2, 256($gp)
	sc	$2, -257($gp)
	sc	$2, 256($gp)
	swl	$2, -257($gp)
	swl	$2, 256($gp)
	swr	$2, -257($gp)
	swr	$2, 256($gp)

	cache	0, -257($2)
	cache	0, 256($2)
	cache	-1, 0($2)
	cache	32, 0($2)
	pref	0, -257($2)
	pref	0, 256($2)
	pref	-1, 0($2)
	pref	32, 0($2)

	addiu	$2, $gp, -32769
	addiu	$2, $gp, 32768
	addu	$2, $gp, -32769
	addu	$2, $gp, 32768

	lui	$2, -1
	lui	$2, 65536
	andi	$2, -1
	andi	$2, 65536
	ori	$2, -1
	ori	$2, 65536
	xori	$2, -1
	xori	$2, 65536

	ext	$2, $3, 0, 0
	ext	$2, $3, 0, 33
	ext	$2, $3, -1, 1
	ext	$2, $3, 32, 1
	ins	$2, $3, 0, 0
	ins	$2, $3, 0, 33
	ins	$2, $3, -1, 1
	ins	$2, $3, 32, 1
	ins	$2, $0, 0, 0
	ins	$2, $0, 0, 33
	ins	$2, $0, -1, 1
	ins	$2, $0, 32, 1

	sync	-1
	sync	32

	mfc0	$2, $3, -1
	mfc0	$2, $3, 32
	mtc0	$2, $3, -1
	mtc0	$2, $3, 32

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	16
	.align	4, 0
