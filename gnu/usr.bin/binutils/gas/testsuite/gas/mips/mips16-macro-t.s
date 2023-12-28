	.set	mips16
	.ent	foo
foo:
	div.t	$2,$3,$4
	divu.t	$3,$4,$5
	ddiv.t	$4,$5,$6
	ddivu.t	$5,$6,$7
	rem.t	$6,$7,$16
	remu.t	$6,$7,$17
	drem.t	$2,$3,$4
	dremu.t	$3,$4,$5
	mul.t	$4,$5,$6
	dmul.t	$5,$6,$7
	subu.t	$2,-32767
	subu.t	$3,16
	subu.t	$4,32768
	subu.t	$3,$7,-16383
	subu.t	$4,$16,4
	subu.t	$5,$17,16384
	dsubu.t	$4,-32767
	dsubu.t	$6,6
	dsubu.t	$7,32768
	dsubu.t	$2,$4,-16383
	dsubu.t	$3,$7,8
	dsubu.t	$4,$5,16384
1:	beq.t	$2,$3,1b
1:	bne.t	$4,$5,1b
1:	blt.t	$6,$7,1b
1:	bltu.t	$16,$17,1b
1:	ble.t	$4,$7,1b
1:	bleu.t	$5,$6,1b
1:	bge.t	$4,$16,1b
1:	bgeu.t	$5,$17,1b
1:	bgt.t	$4,$6,1b
1:	bgtu.t	$5,$7,1b
1:	beq.t	$2,1,1b
1:	beq.t	$3,65535,1b
1:	bne.t	$4,1,1b
1:	bne.t	$5,65535,1b
1:	blt.t	$6,-32768,1b
1:	blt.t	$7,32767,1b
1:	bltu.t	$16,-32768,1b
1:	bltu.t	$17,32767,1b
1:	ble.t	$2,-32769,1b
1:	ble.t	$3,32766,1b
1:	bleu.t	$4,-32769,1b
1:	bleu.t	$5,32766,1b
1:	bge.t	$6,-32768,1b
1:	bge.t	$7,32766,1b
1:	bgeu.t	$16,-32768,1b
1:	bgeu.t	$17,32767,1b
1:	bgt.t	$2,-32769,1b
1:	bgt.t	$3,32766,1b
1:	bgtu.t	$4,-32769,1b
1:	bgtu.t	$5,32766,1b
	abs.t	$2
	abs.t	$3,$3
	abs.t	$4,$5
	.end	foo
