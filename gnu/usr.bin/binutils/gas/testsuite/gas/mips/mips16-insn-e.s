	.set		mips16
	.set		noreorder
foo:
	addiu.e		$16, $29, 0	# ADDIUSP
	addu.e		$16, $29, 0

	.align		2
	addiu.e		$16, $pc, 0	# ADDIUPC
	addu.e		$16, $pc, 0
	la.e		$16, . - 1

	b.e		. + 2		# B

	jal.e		0		# JAL(X)
	 nop
	jalx.e		0
	 nop

	beqz.e		$16, . + 2	# BEQZ

	bnez.e		$16, . + 2	# BNEZ

	sll.e		$16, $16, 8	# SHIFT		# SLL
	dsll.e		$16, $16, 8			# DSLL
	srl.e		$16, $16, 8			# SRL
	sra.e		$16, $16, 8			# SRA

	ld.e		$16, 0($16)	# LD

	addiu.e		$16, $16, 0	# RRI-A		# ADDIU
	addu.e		$16, $16, 0
	daddiu.e	$16, $16, 0			# DADDIU
	daddu.e		$16, $16, 0

	addiu.e		$16, 0		# ADDIU8
	addu.e		$16, 0

	slti.e		$16, 0		# SLTI
	slt.e		$16, 0

	sltiu.e		$16, 0		# SLTIU
	sltu.e		$16, 0

	bteqz.e		. + 2		# I8		# BTEQZ
	btnez.e		. + 2				# BTNEZ
	sw.e		$31, 0($29)			# SWRASP
	addiu.e		$29, 0				# ADJSP
	addiu.e		$29, $29, 0
	addu.e		$29, 0
	addu.e		$29, $29, 0
	restore.e	128				# SVRS	     # RESTORE
	save.e		128						# SAVE
	nop.e						# MOV32R
	move.e		$0, $16
	move.e		$16, $0				# MOVR32

	li.e		$16, 0		# LI

	cmpi.e		$16, 0		# CMPI
	cmp.e		$16, 0

	sd.e		$16, 0($16)	# SD

	lb.e		$16, 0($16)	# LB

	lh.e		$16, 0($16)	# LH

	lw.e		$16, 0($29)	# LWSP

	lw.e		$16, 0($16)	# LW

	lbu.e		$16, 0($16)	# LBU

	lhu.e		$16, 0($16)	# LHU

	.align	2
	lw.e		$16, 0($pc)	# LWPC
	lw.e		$16, . - 3

	lwu.e		$16, 0($16)	# LWU

	sb.e		$16, 0($16)	# SB

	sh.e		$16, 0($16)	# SH

	sw.e		$16, 0($29)	# SWSP

	sw.e		$16, 0($16)	# SW

	daddu.e		$16, $16, $16	# RRR		# DADDU
	addu.e		$16, $16, $16			# ADDU
	dsubu.e		$16, $16, $16			# DSUBU
	subu.e		$16, $16, $16			# SUBU

	jr.e		$16		# RR		# J(AL)R(C)	# JR rx
	 nop
	j.e		$16
	 nop
	jr.e		$31						# JR ra
	 nop
	j.e		$31
	 nop
	jalr.e		$16						# JALR
	 nop
	jalr.e		$31, $16
	 nop
	jal.e		$16
	 nop
	jal.e		$31, $16
	 nop
	jrc.e		$16					       # JRC rx
	jrc.e		$31					       # JRC ra
	jalrc.e		$16						# JALRC
	jalrc.e		$31, $16
	sdbbp.e		0				# SDBBP
	slt.e		$16, $16			# SLT
	sltu.e		$16, $16			# SLTU
	sllv.e		$16, $16			# SLLV
	sll.e		$16, $16
	break.e		0				# BREAK
	srlv.e		$16, $16			# SRLV
	srl.e		$16, $16
	srav.e		$16, $16			# SRAV
	sra.e		$16, $16
	dsrl.e		$16, 8				# DSRL
	entry.e						# ENTRY/EXIT
	entry.e		$31
	exit.e		$f0
	exit.e
	cmp.e		$16, $16			# CMP
	neg.e		$16, $16			# NEG
	and.e		$16, $16			# AND
	or.e		$16, $16			# OR
	xor.e		$16, $16			# XOR
	not.e		$16, $16			# NOT
	mfhi.e		$16				# MFHI
	zeb.e		$16				# CNVT		# ZEB
	zeh.e		$16						# ZEH
	zew.e		$16						# ZEW
	seb.e		$16						# SEB
	seh.e		$16						# SEH
	sew.e		$16						# SEW
	mflo.e		$16				# MFLO
	dsra.e		$16, 8				# DSRA
	dsllv.e		$16, $16			# DSLLV
	dsll.e		$16, $16
	dsrlv.e		$16, $16			# DSRLV
	dsrl.e		$16, $16
	dsrav.e		$16, $16			# DSRAV
	dsra.e		$16, $16
	mult.e		$16, $16			# MULT
	multu.e		$16, $16			# MULTU
	div.e		$0, $16, $16			# DIV
	rem.e		$0, $16, $16
	divu.e		$0, $16, $16			# DIVU
	remu.e		$0, $16, $16
	dmult.e		$16, $16			# DMULT
	dmultu.e	$16, $16			# DMULTU
	ddiv.e		$0, $16, $16			# DDIV
	drem.e		$0, $16, $16
	ddivu.e		$0, $16, $16			# DDIVU
	dremu.e		$0, $16, $16

	extend.e	0		# EXTEND

	ld.e		$16, 0($29)	# I64		# LDSP
	sd.e		$16, 0($29)			# SDSP
	sd.e		$31, 0($29)			# SDRASP
	daddiu.e	$29, 0				# DADJSP
	daddiu.e	$29, $29, 0
	daddu.e		$29, 0
	daddu.e		$29, $29, 0
	.align		3
	ld.e		$16, 0($pc)			# LDPC
	ld.e		$16, . - 3
	daddiu.e	$16, 0				# DADDIU5
	daddu.e		$16, 0
	.align		2
	daddiu.e	$16, $pc, 0			# DADDIUPC
	daddu.e		$16, $pc, 0
	dla.e		$16, . - 1
	daddiu.e	$16, $sp, 0			# DADDIUSP
	daddu.e		$16, $sp, 0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align		4, 0
	.space		16
