	.abicalls
	.text
	.align	2
	.set	nomips16
	.ent	s
	.type	s, @function
s:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,1			# 0x1

	.set	macro
	.set	reorder
	.end	s
	.size	s, .-s
	.align	2
	.globl	t
	.set	nomips16
	.ent	t
	.type	t, @function
t:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.cpload	$25
	.set	nomacro
	lw	$25,%got(s)($28)
	nop
	addiu	$25,$25,%lo(s)
	.reloc	1f,R_MIPS_JALR,s
1:	jr	$25
	nop

	.set	macro
	.set	reorder
	.end	t
	.size	t, .-t
