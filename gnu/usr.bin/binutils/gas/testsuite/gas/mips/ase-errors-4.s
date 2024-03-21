	.set micromips
	.set mips64r2
	.set dsp		# OK
	lbux $4,$5($6)		# OK
	ldx $4,$5($6)		# ERROR: micromips doesn't have 64-bit DSPr1
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled
	.set mips3		# OK (we assume r2 anyway)
	.set nodsp
	lbux $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips64r2
	.set dspr2		# OK
	lbux $4,$5($6)		# OK
	absq_s.qb $3,$4		# OK
	.set mips3		# OK (we assume r2 anyway)
	.set nodspr2
	lbux $4,$5($6)		# ERROR: dsp not enabled
	absq_s.qb $3,$4		# ERROR: dspr2 not enabled

	.set mips64r2
	.set mcu		# OK
	aclr 4,100($4)		# OK
	.set mips3		# OK (we assume r2 anyway)
	.set nomcu
	aclr 4,100($4)		# ERROR: mcu not enabled

	.set mips64r2
	.set mdmx		# ERROR: not supported at all
	add.ob $f4,$f6,$f8	# ERROR: not supported at all
	.set nomdmx

	.set mips64r2
	.set mips3d		# ERROR: not supported at all
	addr.ps $f4,$f6,$f8	# ERROR: not supported at all
	.set nomips3d

	.set mips64r2
	.set mt			# ERROR: not supported at all
	dmt			# ERROR: not supported at all
	.set nomt

	.set mips64
	.set smartmips		# ERROR: not supported at all
	maddp $4,$5		# ERROR: not supported at all
	.set nosmartmips

	.set mips64r2
	.set virt		# OK
	hypcall			# OK
	dmfgc0 $3, $29		# OK
	.set mips3		# OK (we assume r2 anyway)
	.set novirt
	hypcall			# ERROR: virt not enabled
	dmfgc0 $3, $29		# ERROR: virt not enabled

	.set mips64r2
	.set eva		# OK
	lbue $4,16($5)		# OK
	.set mips3		# OK (we assume r2 anyway)
	lbue $4,16($5)		# OK
	.set noeva
	lbue $4,16($5)		# ERROR: eva not enabled

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
