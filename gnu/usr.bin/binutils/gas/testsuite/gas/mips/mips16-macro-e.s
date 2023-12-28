	.set	mips16
	.ent	foo
foo:
	div.e	$2,$3,$4
	divu.e	$3,$4,$5
	ddiv.e	$4,$5,$6
	ddivu.e	$5,$6,$7
	rem.e	$6,$7,$16
	remu.e	$6,$7,$17
	drem.e	$2,$3,$4
	dremu.e	$3,$4,$5
	mul.e	$4,$5,$6
	dmul.e	$5,$6,$7
	subu.e	$2,-32767
	subu.e	$3,16
	subu.e	$4,32768
	subu.e	$3,$7,-16383
	subu.e	$4,$16,4
	subu.e	$5,$17,16384
	dsubu.e	$4,-32767
	dsubu.e	$6,6
	dsubu.e	$7,32768
	dsubu.e	$2,$4,-16383
	dsubu.e	$3,$7,8
	dsubu.e	$4,$5,16384
1:	beq.e	$2,$3,1b
1:	bne.e	$4,$5,1b
1:	blt.e	$6,$7,1b
1:	bltu.e	$16,$17,1b
1:	ble.e	$4,$7,1b
1:	bleu.e	$5,$6,1b
1:	bge.e	$4,$16,1b
1:	bgeu.e	$5,$17,1b
1:	bgt.e	$4,$6,1b
1:	bgtu.e	$5,$7,1b
1:	beq.e	$2,1,1b
1:	beq.e	$3,65535,1b
1:	bne.e	$4,1,1b
1:	bne.e	$5,65535,1b
1:	blt.e	$6,-32768,1b
1:	blt.e	$7,32767,1b
1:	bltu.e	$16,-32768,1b
1:	bltu.e	$17,32767,1b
1:	ble.e	$2,-32769,1b
1:	ble.e	$3,32766,1b
1:	bleu.e	$4,-32769,1b
1:	bleu.e	$5,32766,1b
1:	bge.e	$6,-32768,1b
1:	bge.e	$7,32766,1b
1:	bgeu.e	$16,-32768,1b
1:	bgeu.e	$17,32767,1b
1:	bgt.e	$2,-32769,1b
1:	bgt.e	$3,32766,1b
1:	bgtu.e	$4,-32769,1b
1:	bgtu.e	$5,32766,1b
	abs.e	$2
	abs.e	$3,$3
	abs.e	$4,$5
	.end	foo
