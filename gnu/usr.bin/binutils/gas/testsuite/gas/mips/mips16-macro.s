	.set	mips16
	.ent	foo
foo:
	div	$2,$3,$4
	divu	$3,$4,$5
	ddiv	$4,$5,$6
	ddivu	$5,$6,$7
	rem	$6,$7,$16
	remu	$6,$7,$17
	drem	$2,$3,$4
	dremu	$3,$4,$5
	mul	$4,$5,$6
	dmul	$5,$6,$7
	subu	$2,-32767
	subu	$3,16
	subu	$4,32768
	subu	$3,$7,-16383
	subu	$4,$16,4
	subu	$5,$17,16384
	dsubu	$4,-32767
	dsubu	$6,6
	dsubu	$7,32768
	dsubu	$2,$4,-16383
	dsubu	$3,$7,8
	dsubu	$4,$5,16384
1:	beq	$2,$3,1b
1:	bne	$4,$5,1b
1:	blt	$6,$7,1b
1:	bltu	$16,$17,1b
1:	ble	$4,$7,1b
1:	bleu	$5,$6,1b
1:	bge	$4,$16,1b
1:	bgeu	$5,$17,1b
1:	bgt	$4,$6,1b
1:	bgtu	$5,$7,1b
1:	beq	$2,1,1b
1:	beq	$3,65535,1b
1:	bne	$4,1,1b
1:	bne	$5,65535,1b
1:	blt	$6,-32768,1b
1:	blt	$7,32767,1b
1:	bltu	$16,-32768,1b
1:	bltu	$17,32767,1b
1:	ble	$2,-32769,1b
1:	ble	$3,32766,1b
1:	bleu	$4,-32769,1b
1:	bleu	$5,32766,1b
1:	bge	$6,-32768,1b
1:	bge	$7,32766,1b
1:	bgeu	$16,-32768,1b
1:	bgeu	$17,32767,1b
1:	bgt	$2,-32769,1b
1:	bgt	$3,32766,1b
1:	bgtu	$4,-32769,1b
1:	bgtu	$5,32766,1b
	abs	$2
	abs	$3,$3
	abs	$4,$5
	.end	foo
