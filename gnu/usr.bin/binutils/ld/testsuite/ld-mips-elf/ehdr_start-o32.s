	.abicalls
	.text
	.weak	__ehdr_start
	.globl	__start
	.ent	__start
	.frame	$29, 0, $31
	.mask	0x00000000, 0
__start:
	.set	noreorder
	.cpload	$25
	.set	reorder
	lw	$2, __ehdr_start
	jr	$31
	.end	__start
