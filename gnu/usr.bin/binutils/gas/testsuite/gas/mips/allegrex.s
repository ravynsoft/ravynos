	.text
	.set noreorder

	ext	$4, $5, 6, 8
	ins	$4, $5, 6, 8
	seb	$7
	seb	$8, $10
	seh	$7
	seh	$8, $10
	movz	$2, $3, $4
	movn	$2, $3, $4
	wsbh	$7
	wsbh	$8, $10
	wsbw	$7
	wsbw	$8, $10
	bitrev	$7
	bitrev	$8, $10
	clo	$5, $2
	clz	$3, $4
	madd	$8, $9
	maddu	$8, $9
	msub	$8, $9
	msubu	$8, $9
	min	$7, $8, $9
	max	$7, $8, $9
	rotl	$25, $10, 4
	rotr	$25, $10, 4
	rotl	$25, $10, $4
	rotr	$25, $10, $4
	rotrv	$25, $10, $4
	mtic	$v0, $0
	mtic	$zero, $0
	mfic	$v0, $0
	mfic	$t0, $0
	cache	0x18, 4($a0)
	cache	0x1a, 8($sp)
	cache	0x8, 32760($sp)
	cache	0x4, -32760($sp)
	mtdr	$v0, $0
	mfdr	$v0, $0
	mfdr	$v0, $1
	mtdr	$v1, $1
	dret

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
