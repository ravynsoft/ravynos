	.option arch, +c
	# These are valid instructions.
	li a0,0
	c.li a1,0
	andi a2,a2,0
	c.andi a3,0
	addi x0,x0,0
	# compress to c.mv.
	addi a4,a4,0
	# These are hints.
	c.addi a5,0
	# Don't let these compress to hints.
	slli a0, a0, 0
	srli a1, a1, 0
	srai a2, a2, 0
	# These are hints.
	c.slli64 a3
	c.srli64 a4
	c.srai64 a5
