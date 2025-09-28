	.text
	.align	4
	.globl	doit
	.set	mips16
	.ent	doit
	.type	doit, @function
doit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	sll	$2,$4,1
	sll	$4,$4,3
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$2,$2,$4
	.end	doit
	.size	doit, .-doit
