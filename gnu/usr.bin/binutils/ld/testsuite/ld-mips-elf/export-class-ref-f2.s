	.abicalls
	.text

	.globl	f1
	.ent	f1
f1:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	jr	$31
	.end	f1

	.globl	f2
	.ent	f2
f2:
	.frame	$sp, 0, $31
	.mask	0x00000000, 0
	.fmask	0x00000000, 0
	jr	$31
	.end	f2
