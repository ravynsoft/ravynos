	.set nomicromips
	.set mips64r2
	.set dsp		# OK
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# OK
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled
	.set mips64		# ERROR: too low
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# OK
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled
	.set nodsp
	lbux $4,$5($6)		# ERROR: dsp not enabled
	ldx $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips64r2
	.set dspr2		# OK
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# OK
	absq_s.qb $3,$4		# OK
	.set mips64		# ERROR: too low
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# OK
	absq_s.qb $3,$4		# OK
	.set nodspr2
	lbux $4,$5($6)		# ERROR: dsp not enabled
	ldx $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips64r2
	.set mcu		# OK
	aclr 4,100($4)		# OK
	.set mips64		# ERROR: too low
	aclr 4,100($4)		# OK
	.set nomcu
	aclr 4,100($4)		# ERROR: mcu not enabled

	.set mips64
	.set mdmx		# OK
	add.ob $f4,$f6,$f8	# OK
	.set mips4		# ERROR: too low
	add.ob $f4,$f6,$f8	# OK
	.set nomdmx
	add.ob $f4,$f6,$f8	# ERROR: mdmx not enabled

	.set mips64
	.set mips3d		# OK
	addr.ps $f4,$f6,$f8	# OK
	.set mips4		# ERROR: too low
	addr.ps $f4,$f6,$f8	# OK
	.set nomips3d
	addr.ps $f4,$f6,$f8	# ERROR: mips3d not enabled

	.set mips64r2
	.set mt			# OK
	dmt			# OK
	.set mips64		# ERROR: too low
	dmt			# OK
	.set nomt
	dmt			# ERROR: mt not enabled

	.set mips64
	.set smartmips		# OK
	maddp $4,$5		# OK
	.set mips4		# ERROR: too low
	maddp $4,$5		# OK
	.set nosmartmips
	maddp $4,$5		# ERROR: smartmips not enabled

	.set mips64r2
	.set virt		# OK
	hypcall			# OK
	dmfgc0 $3, $29		# OK
	.set mips64		# ERROR: too low
	hypcall			# OK
	dmfgc0 $3, $29		# OK
	.set novirt
	hypcall			# ERROR: virt not enabled
	dmfgc0 $3, $29		# ERROR: virt not enabled

	.set mips64r2
	.set eva		# OK
	lbue $4,16($5)		# OK
	.set mips64		# ERROR: too low
	lbue $4,16($5)		# OK
	.set noeva
	lbue $4,16($5)		# ERROR: eva not enabled

	.set mips64r6
	.set crc		# OK
	crc32b $4,$7,$4		# OK
	crc32d $4,$7,$4		# OK
	.set mips64r5		# ERROR: too low
	crc32b $4,$7,$4		# OK
	crc32d $4,$7,$4		# OK
	.set nocrc
	crc32b $4,$7,$4		# ERROR: crc not enabled
	crc32d $4,$7,$4		# ERROR: crc not enabled

	.set mips64r6
	.set ginv		# OK
	ginvi $a0		# OK
	.set mips64r5		# ERROR: too low
	ginvt $a0,1		# OK
	.set noginv
	ginvi $a0		# ERROR: ginv not enabled

	# There should be no errors after this.
	.set fp=32
	.set mips4
	.set dsp
	.set dspr2
	.set mcu
	.set mdmx
	.set mips3d
	.set mt
	.set smartmips
	.set eva
