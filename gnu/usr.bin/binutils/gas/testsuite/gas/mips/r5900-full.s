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

	# Test R5900 specific instructions:
	adda.s $f0, $f31
	adda.s $f31, $f0

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

	# Test ei/di, but the R5900 has a bug. ei/di should not be used.
	di
	ei

	# Like div but result is written to lo1 and hi1 registers (pipeline 1).
	div1 $0, $1, $31
	div1 $0, $31, $1
	divu1 $0, $1, $31
	divu1 $0, $31, $1

	# 128 bit store instruction.
	sq $0, 0($0)
	sq $1, 0x7fff($1)
	sq $8, -0x8000($8)
	sq $31, -1($31)
	.set at
	sq $0, 0x8000($2)
	sq $8, -0x8001($31)
	sq $31, 0xF1234567($4)
	.set noat

	# 128 bit load instruction.
	lq $0, 0($0)
	lq $1, 0x7fff($1)
	lq $8, -0x8000($8)
	lq $31, -1($31)
	lq $3, 0x8000($2)
	lq $8, -0x8001($31)
	lq $31, 0xF1234567($4)

	# Prefetch cache
	pref 0, 0($0)
	pref 1, 0x7fff($1)
	pref 8, -0x8000($8)
	pref 31, -1($31)

	# Floating point multiply-ADD
	madd.s $f0, $f31, $f0
	madd.s $f31, $f0, $f31

	# Like maddu, but pipeline 1
	maddu1 $0, $31
	maddu1 $31, $0
	maddu1 $0, $0, $31
	maddu1 $31, $31, $0

	# Like madd, but pipeline 1
	madd1 $0, $31
	madd1 $31, $0
	madd1 $0, $0, $31
	madd1 $31, $31, $0

	# Floating point multiply-ADD
	madda.s $f0, $f31
	madda.s $f31, $f0

	# Floating point maximum
	max.s $f0, $f31, $f0
	max.s $f31, $f0, $f31

	# Floating point minimum
	min.s $f0, $f31, $f0
	min.s $f31, $f0, $f31

	# Preformance counter registers
	mfpc $31, 0
	mfpc $0, 1
	mfps $0, 0
	mfps $31, 0
	mtpc $31, 0
	mtpc $0, 1
	mtps $0, 0
	mtps $31, 0

	# Brekpoint register
	mfbpc $0
	mfbpc $31
	mtbpc $0
	mtbpc $31
	mfdab $0
	mfdab $31
	mtdab $0
	mtdab $31
	mfdabm $0
	mfdabm $31
	mtdabm $0
	mtdabm $31
	mfdvb $0
	mfdvb $31
	mtdvb $0
	mtdvb $31
	mfdvbm $0
	mfdvbm $31
	mtdvbm $0
	mtdvbm $31
	mfiab $0
	mfiab $31
	mtiab $0
	mtiab $31
	mfiabm $0
	mfiabm $31
	mtiabm $0
	mtiabm $31

	# Pipeline1
	mfhi1 $0
	mfhi1 $31
	mthi1 $0
	mthi1 $31
	mflo1 $0
	mflo1 $31
	mtlo1 $0
	mtlo1 $31

	# Shift amount register
	mfsa $0
	mfsa $31
	mtsa $0
	mtsa $31
	mtsab $0, -1
	mtsab $8, 0x8000
	mtsab $8, 0x7FFF
	mtsab $31, 0
	mtsah $0, -1
	mtsah $8, 0x8000
	mtsah $8, 0x7FFF
	mtsah $31, 0

	movn $0, $0, $31
	movn $31, $31, $0
	movz $0, $0, $31
	movz $31, $31, $0

	# Floating multiply and subtract
	msub.s $f0, $f31, $f0
	msub.s $f31, $f0, $f31

	# Floating multiply and subtract from accumulator
	msuba.s $f0, $f31
	msuba.s $f31, $f0

	# Floating point multiply to accumulator
	mula.s $f0, $f31
	mula.s $f31, $f0

	# Like mult but pipeline 1
	mult1 $0, $0, $31
	mult1 $31, $31, $0
	mult1 $0, $31
	mult1 $31, $0

	# Like multu but pipeline 1
	multu1 $0, $0, $31
	multu1 $31, $31, $0
	multu1 $0, $31
	multu1 $31, $0

	# Quadword funnel shift right variable
	qfsrv $0, $0, $31
	qfsrv $31, $31, $0

	# Floating point reciprocal squre root
	rsqrt.s $f0, $f31, $f0
	rsqrt.s $f31, $f0, $f31

	# Floating point subtract to accumulator
	suba.s $f0, $f31
	suba.s $f31, $f0

	# Parallel instructions operating on 128 bit registers:
	pabsh $0, $31
	pabsh $31, $0
	pabsw $0, $31
	pabsw $31, $0
	paddb $0, $0, $31
	paddb $31, $31, $0
	paddh $0, $0, $31
	paddh $31, $31, $0
	paddsb $0, $0, $31
	paddsb $31, $31, $0
	paddsh $0, $0, $31
	paddsh $31, $31, $0
	paddsw $0, $0, $31
	paddsw $31, $31, $0
	paddub $0, $0, $31
	paddub $31, $31, $0
	padduh $0, $0, $31
	padduh $31, $31, $0
	padduw $0, $0, $31
	padduw $31, $31, $0
	paddw $0, $0, $31
	paddw $31, $31, $0
	padsbh $0, $0, $31
	padsbh $31, $31, $0
	pand $0, $0, $31
	pand $31, $31, $0
	pceqb $0, $0, $31
	pceqb $31, $31, $0
	pceqh $0, $0, $31
	pceqh $31, $31, $0
	pceqw $0, $0, $31
	pcgtb $31, $31, $0
	pceqw $0, $0, $31
	pceqw $31, $31, $0
	pcgtb $0, $0, $31
	pcgtb $31, $31, $0
	pcgth $0, $0, $31
	pcgth $31, $31, $0
	pcgtw $0, $0, $31
	pcgtw $31, $31, $0
	pcpyh $0, $31
	pcpyh $31, $0
	pcpyld $0, $0, $31
	pcpyld $31, $31, $0
	pcpyud $0, $0, $31
	pcpyud $31, $31, $0
	pdivbw $0, $31
	pdivbw $31, $0
	pdivuw $0, $31
	pdivuw $31, $0
	pdivw $0, $31
	pdivw $31, $0
	pexch $0, $31
	pexch $31, $0
	pexcw $0, $31
	pexcw $31, $0
	pexeh $0, $31
	pexeh $31, $0
	pexew $0, $31
	pexew $31, $0
	pext5 $0, $31
	pext5 $31, $0
	pextlb $0, $0, $31
	pextlb $31, $31, $0
	pextlh $0, $0, $31
	pextlh $31, $31, $0
	pextlw $0, $0, $31
	pextlw $31, $31, $0
	pextub $0, $0, $31
	pextub $31, $31, $0
	pextuh $0, $0, $31
	pextuh $31, $31, $0
	pextuw $0, $0, $31
	pextuw $31, $31, $0
	phmadh $0, $0, $31
	phmadh $31, $31, $0
	phmsbh $0, $0, $31
	phmsbh $31, $31, $0
	pinteh $0, $0, $31
	pinteh $31, $31, $0
	pinth $0, $0, $31
	pinth $31, $31, $0
	plzcw $0, $31
	plzcw $31, $0
	pmaddh $0, $0, $31
	pmaddh $31, $31, $0
	pmadduw $0, $0, $31
	pmadduw $31, $31, $0
	pmaddw $0, $0, $31
	pmaddw $31, $31, $0
	pmaxh $0, $0, $31
	pmaxh $31, $31, $0
	pmaxw $0, $0, $31
	pmaxw $31, $31, $0
	pmfhi $0
	pmfhi $31
	pmfhl.lh $0
	pmfhl.lh $31
	pmfhl.lw $0
	pmfhl.lw $31
	pmfhl.sh $0
	pmfhl.sh $31
	pmfhl.slw $0
	pmfhl.slw $31
	pmfhl.uw $0
	pmfhl.uw $31
	pmflo $0
	pmflo $31
	pminh $0, $0, $31
	pminh $31, $31, $0
	pminw $0, $0, $31
	pminw $31, $31, $0
	pmsubh $0, $0, $31
	pmsubh $31, $31, $0
	pmsubw $0, $0, $31
	pmsubw $31, $31, $0
	pmthi $0
	pmthi $31
	pmthl.lw $0
	pmthl.lw $31
	pmtlo $0
	pmtlo $31
	pmulth $0, $0, $31
	pmulth $31, $31, $0
	pmultuw $0, $0, $31
	pmultuw $31, $31, $0
	pmultw $0, $0, $31
	pmultw $31, $31, $0
	pmultw $0, $0, $31
	pmultw $31, $31, $0
	pnor $0, $0, $31
	pnor $31, $31, $0
	por $0, $0, $31
	por $31, $31, $0
	ppac5 $0, $31
	ppac5 $31, $0
	ppacb $0, $0, $31
	ppacb $31, $31, $0
	ppach $0, $0, $31
	ppach $31, $31, $0
	ppacw $0, $0, $31
	ppacw $31, $31, $0
	prevh $0, $31
	prevh $31, $0
	prot3w $0, $31
	prot3w $31, $0
	psllh $31, $0, 0
	psllh $0, $31, 31
	psllvw $0, $31, $0
	psllvw $31, $0, $31
	psllw $31, $0, 0
	psllw $0, $31, 31
	psrah $31, $0, 0
	psrah $0, $31, 31
	psravw $0, $31, $0
	psravw $31, $0, $31
	psraw $31, $0, 0
	psraw $0, $31, 31
	psrlh $31, $0, 0
	psrlh $0, $31, 31
	psrlvw $0, $31, $0
	psrlvw $31, $0, $31
	psrlw $31, $0, 0
	psrlw $0, $31, 31
	psubb $0, $0, $31
	psubb $31, $31, $0
	psubh $0, $0, $31
	psubh $31, $31, $0
	psubsb $0, $0, $31
	psubsb $31, $31, $0
	psubsh $0, $0, $31
	psubsh $31, $31, $0
	psubsw $0, $0, $31
	psubsw $31, $31, $0
	psubub $0, $0, $31
	psubub $31, $31, $0
	psubuh $0, $0, $31
	psubuh $31, $31, $0
	psubuw $0, $0, $31
	psubuw $31, $31, $0
	psubw $0, $0, $31
	psubw $31, $31, $0
	pxor $0, $0, $31
	pxor $31, $31, $0

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

	# Enable sc/ll instructions by changing ISA level:
	.set push
	.set mips2
	ll $5, 0($6)
	sc $5, 0($6)
	.set pop

	# Enable scd/lld instructions by changing ISA level:
	.set push
	.set mips3
	lld $5, 0($6)
	scd $5, 0($6)
	.set pop

	.space	8
	.end stuff
