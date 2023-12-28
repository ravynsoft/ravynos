	.set		mips16
	.set		noreorder
foo:
	addiu.t		$16, $29, 0	# ADDIUSP
	addu.t		$16, $29, 0

	.align		2
	addiu.t		$16, $pc, 0	# ADDIUPC
	addu.t		$16, $pc, 0
	la.t		$16, . - 1

	b.t		. + 2		# B

	jal.t		0		# JAL(X)
	 nop
	jalx.t		0
	 nop

	beqz.t		$16, . + 2	# BEQZ

	bnez.t		$16, . + 2	# BNEZ

	sll.t		$16, $16, 8	# SHIFT		# SLL
	dsll.t		$16, $16, 8			# DSLL
	srl.t		$16, $16, 8			# SRL
	sra.t		$16, $16, 8			# SRA

	ld.t		$16, 0($16)	# LD

	addiu.t		$16, $16, 0	# RRI-A		# ADDIU
	addu.t		$16, $16, 0
	daddiu.t	$16, $16, 0			# DADDIU
	daddu.t		$16, $16, 0

	addiu.t		$16, 0		# ADDIU8
	addu.t		$16, 0

	slti.t		$16, 0		# SLTI
	slt.t		$16, 0

	sltiu.t		$16, 0		# SLTIU
	sltu.t		$16, 0

	bteqz.t		. + 2		# I8		# BTEQZ
	btnez.t		. + 2				# BTNEZ
	sw.t		$31, 0($29)			# SWRASP
	addiu.t		$29, 0				# ADJSP
	addiu.t		$29, $29, 0
	addu.t		$29, 0
	addu.t		$29, $29, 0
	restore.t	128				# SVRS	     # RESTORE
	save.t		128						# SAVE
	nop.t						# MOV32R
	move.t		$0, $16
	move.t		$16, $0				# MOVR32

	li.t		$16, 0		# LI

	cmpi.t		$16, 0		# CMPI
	cmp.t		$16, 0

	sd.t		$16, 0($16)	# SD

	lb.t		$16, 0($16)	# LB

	lh.t		$16, 0($16)	# LH

	lw.t		$16, 0($29)	# LWSP

	lw.t		$16, 0($16)	# LW

	lbu.t		$16, 0($16)	# LBU

	lhu.t		$16, 0($16)	# LHU

	.align	2
	lw.t		$16, 0($pc)	# LWPC
	lw.t		$16, . - 3

	lwu.t		$16, 0($16)	# LWU

	sb.t		$16, 0($16)	# SB

	sh.t		$16, 0($16)	# SH

	sw.t		$16, 0($29)	# SWSP

	sw.t		$16, 0($16)	# SW

	daddu.t		$16, $16, $16	# RRR		# DADDU
	addu.t		$16, $16, $16			# ADDU
	dsubu.t		$16, $16, $16			# DSUBU
	subu.t		$16, $16, $16			# SUBU

	jr.t		$16		# RR		# J(AL)R(C)	# JR rx
	 nop
	j.t		$16
	 nop
	jr.t		$31						# JR ra
	 nop
	j.t		$31
	 nop
	jalr.t		$16						# JALR
	 nop
	jalr.t		$31, $16
	 nop
	jal.t		$16
	 nop
	jal.t		$31, $16
	 nop
	jrc.t		$16					       # JRC rx
	jrc.t		$31					       # JRC ra
	jalrc.t		$16						# JALRC
	jalrc.t		$31, $16
	sdbbp.t		0				# SDBBP
	slt.t		$16, $16			# SLT
	sltu.t		$16, $16			# SLTU
	sllv.t		$16, $16			# SLLV
	sll.t		$16, $16
	break.t		0				# BREAK
	srlv.t		$16, $16			# SRLV
	srl.t		$16, $16
	srav.t		$16, $16			# SRAV
	sra.t		$16, $16
	dsrl.t		$16, 8				# DSRL
	entry.t						# ENTRY/EXIT
	entry.t		$31
	exit.t		$f0
	exit.t
	cmp.t		$16, $16			# CMP
	neg.t		$16, $16			# NEG
	and.t		$16, $16			# AND
	or.t		$16, $16			# OR
	xor.t		$16, $16			# XOR
	not.t		$16, $16			# NOT
	mfhi.t		$16				# MFHI
	zeb.t		$16				# CNVT		# ZEB
	zeh.t		$16						# ZEH
	zew.t		$16						# ZEW
	seb.t		$16						# SEB
	seh.t		$16						# SEH
	sew.t		$16						# SEW
	mflo.t		$16				# MFLO
	dsra.t		$16, 8				# DSRA
	dsllv.t		$16, $16			# DSLLV
	dsll.t		$16, $16
	dsrlv.t		$16, $16			# DSRLV
	dsrl.t		$16, $16
	dsrav.t		$16, $16			# DSRAV
	dsra.t		$16, $16
	mult.t		$16, $16			# MULT
	multu.t		$16, $16			# MULTU
	div.t		$0, $16, $16			# DIV
	rem.t		$0, $16, $16
	divu.t		$0, $16, $16			# DIVU
	remu.t		$0, $16, $16
	dmult.t		$16, $16			# DMULT
	dmultu.t	$16, $16			# DMULTU
	ddiv.t		$0, $16, $16			# DDIV
	drem.t		$0, $16, $16
	ddivu.t		$0, $16, $16			# DDIVU
	dremu.t		$0, $16, $16

	extend.t	0		# EXTEND

	ld.t		$16, 0($29)	# I64		# LDSP
	sd.t		$16, 0($29)			# SDSP
	sd.t		$31, 0($29)			# SDRASP
	daddiu.t	$29, 0				# DADJSP
	daddiu.t	$29, $29, 0
	daddu.t		$29, 0
	daddu.t		$29, $29, 0
	.align		3
	ld.t		$16, 0($pc)			# LDPC
	ld.t		$16, . - 3
	daddiu.t	$16, 0				# DADDIU5
	daddu.t		$16, 0
	.align		2
	daddiu.t	$16, $pc, 0			# DADDIUPC
	daddu.t		$16, $pc, 0
	dla.t		$16, . - 1
	daddiu.t	$16, $sp, 0			# DADDIUSP
	daddu.t		$16, $sp, 0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align		4, 0
	.space		16
