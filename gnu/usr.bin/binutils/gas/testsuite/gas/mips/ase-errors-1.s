	.set nomicromips
	.set mips32r2
	.set fp=64
	.set dsp		# OK
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# ERROR: 64-bit only
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# ERROR: 64-bit only
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled
	.set nodsp
	lbux $4,$5($6)		# ERROR: dsp not enabled
	ldx $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips32r2
	.set fp=64
	.set dspr2		# OK
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# ERROR: 64-bit only
	absq_s.qb $3,$4		# OK
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# ERROR: 64-bit only
	absq_s.qb $3,$4		# OK
	.set nodspr2
	lbux $4,$5($6)		# ERROR: dsp not enabled
	ldx $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips32r2
	.set fp=64
	.set mcu		# OK
	aclr 4,100($4)		# OK
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	aclr 4,100($4)		# OK
	.set nomcu
	aclr 4,100($4)		# ERROR: mcu not enabled

	.set mips32r2
	.set fp=64
	.set mdmx		# ERROR: needs a 64-bit target
	add.ob $f4,$f6,$f8	# OK
	.set fp=32		# ERROR: needs fp=64
	add.ob $f4,$f6,$f8	# OK
	.set nomdmx
	add.ob $f4,$f6,$f8	# ERROR: mdmx not enabled

	.set mips32r2
	.set fp=64
	.set mips3d		# OK
	addr.ps $f4,$f6,$f8	# OK
	.set fp=32		# ERROR: needs fp=64
	.set mips32		# ERROR: too low
	addr.ps $f4,$f6,$f8	# OK
	.set nomips3d
	addr.ps $f4,$f6,$f8	# ERROR: mips3d not enabled

	.set mips32r2
	.set fp=64
	.set mt			# OK
	dmt			# OK
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	dmt			# OK
	.set nomt
	dmt			# ERROR: mt not enabled

	.set fp=32
	.set mips32
	.set smartmips		# OK
	maddp $4,$5		# OK
	.set mips2		# ERROR: too low
	maddp $4,$5		# OK
	.set nosmartmips
	maddp $4,$5		# ERROR: smartmips not enabled

	.set mips32r2
	.set fp=64
	.set virt		# OK
	hypcall			# OK
	dmfgc0 $3, $29		# ERROR: 64-bit only
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	hypcall			# OK
	dmfgc0 $3, $29		# ERROR: 64-bit only
	.set novirt
	hypcall			# ERROR: virt not enabled
	dmfgc0 $3, $29		# ERROR: virt not enabled

	.set mips32r2
	.set fp=64
	.set eva		# OK
	lbue $4,16($5)		# OK
	.set fp=32		# OK
	.set mips32		# ERROR: too low
	lbue $4,16($5)		# OK
	.set noeva
	lbue $4,16($5)		# ERROR: eva not enabled

	.set mips32r6
	.set crc		# OK
	crc32b $4,$7,$4		# OK
	crc32d $4,$7,$4		# ERROR: 64-bit only
	.set mips32r5		# ERROR: too low
	crc32b $4,$7,$4		# OK
	.set nocrc
	crc32b $4,$7,$4		# ERROR: crc not enabled

	.set mips32r6
	.set ginv		# OK
	ginvi $a0		# OK
	.set mips32r5		# ERROR: too low
	ginvt $a0, 1		# OK
	.set noginv
	ginvi $a0		# ERROR: ginv not enabled

	.set mips32r6
	.set eva
	llwpe	$2, $3, $4	# OK
	scwpe	$2, $3, $4	# OK
	.set noeva
	llwpe	$2, $3, $4	# ERROR: eva not enabled
	scwpe	$2, $3, $4	# ERROR: eva not enabled
	.set mips32r5
	.set eva
	llwpe	$2, $3, $4	# ERROR: only avaialable on R6
	scwpe	$2, $3, $4	# ERROR: only avaialable on R6

	# There should be no errors after this.
	.set fp=32
	.set mips1
	.set dsp
	.set dspr2
	.set mcu
	.set mdmx
	.set mips3d
	.set mt
	.set smartmips
	.set eva
