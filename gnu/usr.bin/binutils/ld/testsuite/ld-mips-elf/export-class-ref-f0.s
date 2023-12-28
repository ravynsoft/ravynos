	.abicalls
	.text

	.if	refv == 3
	.protected f1
	.elseif	refv == 2
	.hidden	f1
	.elseif	refv == 1
	.internal f1
	.endif

	.globl	f0
	.ent	f0
f0:
	.frame	$sp, 32, $31
	.mask	0x80000000, -4
	.fmask	0x00000000, 0
	.set	noreorder
	.cpload	$25
	.set	reorder
	addiu	$sp, $sp, -32
	sw	$31, 28($sp)
	.cprestore 16

	lw	$25, %call16(f1)($28)
	jalr	$25
	lw	$28, 16($sp)

	lw	$25, %call16(f2)($28)
	jalr	$25
	lw	$28, 16($sp)

	move	$2, $0
	lw	$31, 28($sp)
	addiu	$sp, $sp, 32
	jr	$31
	.end	f0
