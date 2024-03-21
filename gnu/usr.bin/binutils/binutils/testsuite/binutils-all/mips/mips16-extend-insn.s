	.set	mips16
	.set	noreorder
foo:
	extend	0x123		# ADDIUSP
	addiu	$16, $29, 0
	extend	0x123
	addiu	$16, $29, 128
	extend	0x123
	addiu	$16, $29, 256
	extend	0x123
	addiu	$16, $29, 384
	extend	0x123
	addiu	$16, $29, 512
	extend	0x123
	addiu	$16, $29, 640
	extend	0x123
	addiu	$16, $29, 768
	extend	0x123
	addiu	$16, $29, 896

	extend	0x123		# ADDIUPC
	addiu	$16, $pc, 0
	extend	0x123
	addiu	$16, $pc, 128
	extend	0x123
	addiu	$16, $pc, 256
	extend	0x123
	addiu	$16, $pc, 384
	extend	0x123
	addiu	$16, $pc, 512
	extend	0x123
	addiu	$16, $pc, 640
	extend	0x123
	addiu	$16, $pc, 768
	extend	0x123
	addiu	$16, $pc, 896

	extend	0x123		# B
	b	. + 2
	extend	0x123
	b	. + 66
	extend	0x123
	b	. + 130
	extend	0x123
	b	. + 194
	extend	0x123
	b	. + 258
	extend	0x123
	b	. + 322
	extend	0x123
	b	. + 386
	extend	0x123
	b	. + 450

	extend	0x123		# BEQZ
	beqz	$16, . + 2
	extend	0x123
	beqz	$16, . + 66
	extend	0x123
	beqz	$16, . + 130
	extend	0x123
	beqz	$16, . + 194
	extend	0x123
	beqz	$16, . - 254
	extend	0x123
	beqz	$16, . - 190
	extend	0x123
	beqz	$16, . - 126
	extend	0x123
	beqz	$16, . - 62

	extend	0x123		# BNEZ
	bnez	$16, . + 2
	extend	0x123
	bnez	$16, . + 66
	extend	0x123
	bnez	$16, . + 130
	extend	0x123
	bnez	$16, . + 194
	extend	0x123
	bnez	$16, . - 254
	extend	0x123
	bnez	$16, . - 190
	extend	0x123
	bnez	$16, . - 126
	extend	0x123
	bnez	$16, . - 62

	extend	0x123		# SHIFT		# SLL
	sll	$16, $16, 8
	extend	0x123
	sll	$16, $16, 1
	extend	0x123
	sll	$16, $16, 2
	extend	0x123
	sll	$16, $16, 3
	extend	0x123
	sll	$16, $16, 4
	extend	0x123
	sll	$16, $16, 5
	extend	0x123
	sll	$16, $16, 6
	extend	0x123
	sll	$16, $16, 7

	extend	0x123				# DSLL
	dsll	$16, $16, 8
	extend	0x123
	dsll	$16, $16, 1
	extend	0x123
	dsll	$16, $16, 2
	extend	0x123
	dsll	$16, $16, 3
	extend	0x123
	dsll	$16, $16, 4
	extend	0x123
	dsll	$16, $16, 5
	extend	0x123
	dsll	$16, $16, 6
	extend	0x123
	dsll	$16, $16, 7

	extend	0x123				# SRL
	srl	$16, $16, 8
	extend	0x123
	srl	$16, $16, 1
	extend	0x123
	srl	$16, $16, 2
	extend	0x123
	srl	$16, $16, 3
	extend	0x123
	srl	$16, $16, 4
	extend	0x123
	srl	$16, $16, 5
	extend	0x123
	srl	$16, $16, 6
	extend	0x123
	srl	$16, $16, 7

	extend	0x123				# SRA
	sra	$16, $16, 8
	extend	0x123
	sra	$16, $16, 1
	extend	0x123
	sra	$16, $16, 2
	extend	0x123
	sra	$16, $16, 3
	extend	0x123
	sra	$16, $16, 4
	extend	0x123
	sra	$16, $16, 5
	extend	0x123
	sra	$16, $16, 6
	extend	0x123
	sra	$16, $16, 7

	extend	0x123		# LD
	ld	$16, 0($16)

	extend	0x123		# RRI-A		# ADDIU
	addiu	$16, $16, 0
	extend	0x123				# DADDIU
	daddiu	$16, $16, 0

	extend	0x123		# ADDIU8
	addiu	$16, 0
	extend	0x123
	addiu	$16, 32
	extend	0x123
	addiu	$16, 64
	extend	0x123
	addiu	$16, 96
	extend	0x123
	addiu	$16, -128
	extend	0x123
	addiu	$16, -96
	extend	0x123
	addiu	$16, -64
	extend	0x123
	addiu	$16, -32

	extend	0x123		# SLTI
	slti	$16, 0
	extend	0x123
	slti	$16, 32
	extend	0x123
	slti	$16, 64
	extend	0x123
	slti	$16, 96
	extend	0x123
	slti	$16, 128
	extend	0x123
	slti	$16, 160
	extend	0x123
	slti	$16, 192
	extend	0x123
	slti	$16, 224

	extend	0x123		# SLTIU
	sltiu	$16, 0
	extend	0x123
	sltiu	$16, 32
	extend	0x123
	sltiu	$16, 64
	extend	0x123
	sltiu	$16, 96
	extend	0x123
	sltiu	$16, 128
	extend	0x123
	sltiu	$16, 160
	extend	0x123
	sltiu	$16, 192
	extend	0x123
	sltiu	$16, 224

	extend	0x123		# I8		# BTEQZ
	bteqz	. + 2
	extend	0x123
	bteqz	. + 66
	extend	0x123
	bteqz	. + 130
	extend	0x123
	bteqz	. + 194
	extend	0x123
	bteqz	. - 254
	extend	0x123
	bteqz	. - 190
	extend	0x123
	bteqz	. - 126
	extend	0x123
	bteqz	. - 62

	extend	0x123				# BTNEZ
	btnez	. + 2
	extend	0x123
	btnez	. + 66
	extend	0x123
	btnez	. + 130
	extend	0x123
	btnez	. + 194
	extend	0x123
	btnez	. - 254
	extend	0x123
	btnez	. - 190
	extend	0x123
	btnez	. - 126
	extend	0x123
	btnez	. - 62

	extend	0x123				# SWRASP
	sw	$31, 0($29)
	extend	0x123
	sw	$31, 128($29)
	extend	0x123
	sw	$31, 256($29)
	extend	0x123
	sw	$31, 512($29)
	extend	0x123
	sw	$31, 640($29)
	extend	0x123
	sw	$31, 768($29)
	extend	0x123
	sw	$31, 896($29)
	extend	0x123
	sw	$31, 0($29)

	extend	0x123				# ADJSP
	addiu	$29, 0
	extend	0x123
	addiu	$29, 256
	extend	0x123
	addiu	$29, 512
	extend	0x123
	addiu	$29, 768
	extend	0x123
	addiu	$29, -1024
	extend	0x123
	addiu	$29, -768
	extend	0x123
	addiu	$29, -512
	extend	0x123
	addiu	$29, -256

	extend	0x123				# SVRS		# RESTORE
	restore	128
	extend	0x123						# SAVE
	save	128

	extend	0x123				# MOV32R
	move	$0, $16
	extend	0x123
	move	$0, $17

	extend	0x123				# MOVR32
	move	$16, $0

	extend	0x123		# LI
	li	$16, 0
	extend	0x123
	li	$16, 32
	extend	0x123
	li	$16, 64
	extend	0x123
	li	$16, 96
	extend	0x123
	li	$16, 128
	extend	0x123
	li	$16, 160
	extend	0x123
	li	$16, 192
	extend	0x123
	li	$16, 224

	extend	0x123		# CMPI
	cmpi	$16, 0
	extend	0x123
	cmpi	$16, 32
	extend	0x123
	cmpi	$16, 64
	extend	0x123
	cmpi	$16, 96
	extend	0x123
	cmpi	$16, 128
	extend	0x123
	cmpi	$16, 160
	extend	0x123
	cmpi	$16, 192
	extend	0x123
	cmpi	$16, 224

	extend	0x123		# SD
	sd	$16, 0($16)

	extend	0x123		# LB
	lb	$16, 0($16)

	extend	0x123		# LH
	lh	$16, 0($16)

	extend	0x123		# LWSP
	lw	$16, 0($29)
	extend	0x123
	lw	$16, 128($29)
	extend	0x123
	lw	$16, 256($29)
	extend	0x123
	lw	$16, 384($29)
	extend	0x123
	lw	$16, 512($29)
	extend	0x123
	lw	$16, 640($29)
	extend	0x123
	lw	$16, 768($29)
	extend	0x123
	lw	$16, 896($29)

	extend	0x123		# LW
	lw	$16, 0($16)

	extend	0x123		# LBU
	lbu	$16, 0($16)

	extend	0x123		# LHU
	lhu	$16, 0($16)

	extend	0x123		# LWPC
	lw	$16, 0($pc)
	extend	0x123
	lw	$16, 128($pc)
	extend	0x123
	lw	$16, 256($pc)
	extend	0x123
	lw	$16, 384($pc)
	extend	0x123
	lw	$16, 512($pc)
	extend	0x123
	lw	$16, 640($pc)
	extend	0x123
	lw	$16, 768($pc)
	extend	0x123
	lw	$16, 896($pc)

	extend	0x123		# LWU
	lwu	$16, 0($16)

	extend	0x123		# SB
	sb	$16, 0($16)

	extend	0x123		# SH
	sh	$16, 0($16)

	extend	0x123		# SWSP
	sw	$16, 0($29)
	extend	0x123
	sw	$16, 128($29)
	extend	0x123
	sw	$16, 256($29)
	extend	0x123
	sw	$16, 384($29)
	extend	0x123
	sw	$16, 512($29)
	extend	0x123
	sw	$16, 640($29)
	extend	0x123
	sw	$16, 768($29)
	extend	0x123
	sw	$16, 896($29)

	extend	0x123		# SW
	sw	$16, 0($16)

	extend	0x123		# RRR		# DADDU
	daddu	$16, $16, $16

	extend	0x123				# ADDU
	addu	$16, $16, $16

	extend	0x123				# DSUBU
	dsubu	$16, $16, $16

	extend	0x123				# SUBU
	subu	$16, $16, $16

	extend	0x123		# RR		# J(AL)R(C)	# JR rx
	jr	$16
	extend	0x123						# JR ra
	jr	$31
	extend	0x123						# JALR
	jalr	$16
	extend	0x123						# JRC rx
	jrc	$16
	extend	0x123						# JRC ra
	jrc	$31
	extend	0x123						# JALRC
	jalrc	$16

	extend	0x123				# SDBBP
	sdbbp	0

	extend	0x123				# SLT
	slt	$16, $16

	extend	0x123				# SLTU
	sltu	$16, $16

	extend	0x123				# SLLV
	sllv	$16, $16

	extend	0x123				# BREAK
	break	0

	extend	0x123				# SRLV
	srlv	$16, $16

	extend	0x123				# SRAV
	srav	$16, $16

	extend	0x123				# DSRL
	dsrl	$16, 8
	extend	0x123
	dsrl	$16, 1
	extend	0x123
	dsrl	$16, 2
	extend	0x123
	dsrl	$16, 3
	extend	0x123
	dsrl	$16, 4
	extend	0x123
	dsrl	$16, 5
	extend	0x123
	dsrl	$16, 6
	extend	0x123
	dsrl	$16, 7

	extend	0x123				# ENTRY/EXIT
	entry
	extend	0x123
	entry	$31
	extend	0x123
	exit	$f0
	extend	0x123
	exit	$f0-$f1
	extend	0x123
	exit

	extend	0x123				# CMP
	cmp	$16, $16

	extend	0x123				# NEG
	neg	$16, $16

	extend	0x123				# AND
	and	$16, $16

	extend	0x123				# OR
	or	$16, $16

	extend	0x123				# XOR
	xor	$16, $16

	extend	0x123				# NOT
	not	$16, $16

	extend	0x123				# MFHI
	mfhi	$16

	extend	0x123				# CNVT		# ZEB
	zeb	$16
	extend	0x123						# ZEH
	zeh	$16
	extend	0x123						# ZEW
	zew	$16
	extend	0x123						# SEB
	seb	$16
	extend	0x123						# SEH
	seh	$16
	extend	0x123						# SEW
	sew	$16

	extend	0x123				# MFLO
	mflo	$16

	extend	0x123				# DSRA
	dsra	$16, 8
	extend	0x123
	dsra	$16, 1
	extend	0x123
	dsra	$16, 2
	extend	0x123
	dsra	$16, 3
	extend	0x123
	dsra	$16, 4
	extend	0x123
	dsra	$16, 5
	extend	0x123
	dsra	$16, 6
	extend	0x123
	dsra	$16, 7

	extend	0x123				# DSLLV
	dsllv	$16, $16

	extend	0x123				# DSRLV
	dsrlv	$16, $16

	extend	0x123				# DSRAV
	dsrav	$16, $16

	extend	0x123				# MULT
	mult	$16, $16

	extend	0x123				# MULTU
	multu	$16, $16

	extend	0x123				# DIV
	div	$0, $16, $16

	extend	0x123				# DIVU
	divu	$0, $16, $16

	extend	0x123				# DMULT
	dmult	$16, $16

	extend	0x123				# DMULTU
	dmultu	$16, $16

	extend	0x123				# DDIV
	ddiv	$0, $16, $16

	extend	0x123				# DDIVU
	ddivu	$0, $16, $16

	extend	0x123		# EXTEND
	extend	0

	extend	0x123		# I64		# LDSP
	ld	$16, 0($29)

	extend	0x123				# SDSP
	sd	$16, 0($29)

	extend	0x123				# SDRASP
	sd	$31, 0($29)
	extend	0x123
	sd	$31, 256($29)
	extend	0x123
	sd	$31, 512($29)
	extend	0x123
	sd	$31, 768($29)
	extend	0x123
	sd	$31, 1024($29)
	extend	0x123
	sd	$31, 1280($29)
	extend	0x123
	sd	$31, 1536($29)
	extend	0x123
	sd	$31, 1792($29)

	extend	0x123				# DADJSP
	daddiu	$29, 0
	extend	0x123
	daddiu	$29, 256
	extend	0x123
	daddiu	$29, 512
	extend	0x123
	daddiu	$29, 768
	extend	0x123
	daddiu	$29, -1024
	extend	0x123
	daddiu	$29, -768
	extend	0x123
	daddiu	$29, -512
	extend	0x123
	daddiu	$29, -256

	extend	0x123				# LDPC
	ld	$16, 0($pc)

	extend	0x123				# DADDIU5
	daddiu	$16, 0

	extend	0x123				# DADDIUPC
	daddiu	$16, $pc, 0

	extend	0x123				# DADDIUSP
	daddiu	$16, $sp, 0

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16
