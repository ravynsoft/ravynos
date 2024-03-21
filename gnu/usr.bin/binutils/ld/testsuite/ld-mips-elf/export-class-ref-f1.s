	.abicalls
	.text

	.globl	f1
	.if	defv == 3
	.protected f1
	.elseif	defv == 2
	.hidden	f1
	.elseif	defv == 1
	.internal f1
	.endif
	.ent	f1
f1:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	jr	$31
	.end	f1
