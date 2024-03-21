pseudo_csr_insn:
	# i-ext
	csrr t0, 0x0
	csrw 0x0, t0
	csrs 0x0, t0
	csrc 0x0, t0
	csrwi 0x0, 31
	csrsi 0x0, 31
	csrci 0x0, 31

	rdcycle t0
	rdtime t0
	rdinstret t0

	# rv32i-ext
	rdcycleh t0
	rdtimeh t0
	rdinstreth t0

	# f-ext
	frcsr t0	# frsr
	fscsr t0, t2	# fssr
	fscsr t2	# fssr
	frrm t0
	fsrm t0, t1
	fsrm t1
	fsrmi t0, 31
	fsrmi 31
	frflags t0
	fsflags t0, t1
	fsflags t1
	fsflagsi t0, 31
	fsflagsi 31
