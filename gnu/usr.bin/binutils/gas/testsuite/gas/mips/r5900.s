	.text

stuff:
	.ent stuff
	.set push
	.set noreorder
	.set noat

	add $0, $0, $31
	add $1, $10, $3
	add $31, $31, $0

	addi $31, $0, 0
	addi $1, $10, 3
	addi $0, $31, -1

	addiu $31, $0, 0
	addiu $1, $10, 3
	addiu $31, $0, 0xFFFF

	and $0, $0, $31
	and $1, $10, $3
	and $31, $31, $0

	andi $31, $0, 0
	andi $1, $10, 3
	andi $0, $31, 0xFFFF

	nop

	# The c.lt.s instruction of R5900 has the same opcode as c.olt.s of MIPS I.
	c.lt.s $f0, $f31
	c.lt.s $f31, $f0

	# The c.le.s instruction of R5900 has the same opcode as c.ole.s of MIPS I.
	c.le.s $f0, $f31
	c.le.s $f31, $f0

	c.eq.s $f0, $f31
	c.eq.s $f31, $f0

	c.f.s $f0, $f31
	c.f.s $f31, $f0

	# The cvt.w.s instruction of the R5900 does the same as trunc.w.s in MIPS I.
	# The cvt.w.s instruction of MIPS I doesn't exist in the R5900 CPU.
	# For compatibility the instruction trunc.w.s uses the opcode of cvt.w.s.
	# cvt.w.s should not be used on R5900.
	trunc.w.s $f0, $f31
	trunc.w.s $f31, $f0

	# 128 bit store instruction.
	sq $0, 0($0)
	sq $1, 0x7fff($1)
	sq $8, -0x8000($8)
	sq $31, -1($31)

	# 128 bit load instruction.
	lq $0, 0($0)
	lq $1, 0x7fff($1)
	lq $8, -0x8000($8)
	lq $31, -1($31)

	# Prefetch cache
	pref 0, 0($0)
	pref 1, 0x7fff($1)
	pref 8, -0x8000($8)
	pref 31, -1($31)

	# Preformance counter registers
	mfpc $31, 0
	mfpc $0, 1
	mfps $0, 0
	mfps $31, 0
	mtpc $31, 0
	mtpc $0, 1
	mtps $0, 0
	mtps $31, 0

	# Pipeline1
	mfhi1 $0
	mfhi1 $31
	mthi1 $0
	mthi1 $31
	mflo1 $0
	mflo1 $31
	mtlo1 $0
	mtlo1 $31

	movn $0, $0, $31
	movn $31, $31, $0
	movz $0, $0, $31
	movz $31, $31, $0

	# Parallel instructions operating on 128 bit registers:
	pcpyld $0, $0, $31
	pcpyld $31, $31, $0
	pextlh $0, $0, $31
	pextlh $31, $31, $0
	pextlw $0, $0, $31
	pextlw $31, $31, $0

	# G1 instructions
	mult $0, $0, $31
	mult $31, $31, $0
	multu $0, $0, $31
	multu $31, $31, $0
	mul $0, $0, $31
	mul $31, $31, $0
	madd $0, $0, $31
	madd $31, $31, $0
	madd $0, $31
	madd $31, $0
	maddu $0, $0, $31
	maddu $31, $31, $0
	maddu $0, $31
	maddu $31, $0
	sync

	.set pop
	.set push
	.set reorder
	# Test the short loop fix with 3 loop instructions.
	li $3, 300
short_loop3:
	addi $3, -1
	addi $4, -1
	# A NOP will be inserted in the branch delay slot.
	bne $3, $0, short_loop3

	# Test the short loop fix with 6 loop instructions.
	li $3, 300
short_loop6:
	addi $3, -1
	addi $4, -1
	addi $5, -1
	addi $6, -1
	addi $7, -1
	# A NOP will be inserted in the branch delay slot.
	bne $3, $0, short_loop6

	# Test the short loop fix with 7 loop instructions.
	li $3, 300
short_loop7:
	addi $3, -1
	addi $4, -1
	addi $5, -1
	addi $6, -1
	addi $7, -1
	addi $8, -1
	# The short loop fix does not apply for loops with
	# more than 6 instructions.
	bne $3, $0, short_loop7

	li $4, 3
	.set pop

	.space	8
	.end stuff
