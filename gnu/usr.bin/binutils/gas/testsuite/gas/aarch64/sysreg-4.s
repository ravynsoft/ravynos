/* sysreg-4.s Test file for ARMv8.5 system registers.  */
func:
	cfp rctx, x1
	dvp rctx, x2
	cpp rctx, x3
	dc cvadp, x4
	mrs x5, rndr
	mrs x6, rndrrs
	mrs x7, scxtnum_el0
	mrs x7, scxtnum_el1
	mrs x7, scxtnum_el2
	mrs x7, scxtnum_el3
	mrs x7, scxtnum_el12
	mrs x8, id_pfr2_el1

	# ARMv8.5-a+memtag
	# MRS (register)
	mrs x1, tco
	mrs x2, TCO
	mrs x1, tfsre0_el1
	mrs x1, TFSR_EL1
	mrs x2, TFSR_EL2
	mrs x3, TFSR_EL3
	mrs x12, TFSR_EL12
	mrs x1, rgsr_el1
	mrs x3, gcr_el1
	mrs x4, gmid_el1

	# MSR (register)
	msr tco, x1
	msr TCO, x2
	msr tfsre0_el1, x1
	msr TFSR_EL1, x1
	msr TFSR_EL2, x2
	msr TFSR_EL3, x3
	msr TFSR_EL12, x12
	msr rgsr_el1, x1
	msr gcr_el1, x3

	# MSR (immediate)
	msr TCO, #1

	# Data cache
	dc igvac, x1
	dc igsw, x2
	dc cgsw, x3
	dc cigsw, x4
	dc cgvac, x5
	dc cgvap, x6
	dc cgvadp, x7
	dc cigvac, x8

	dc gva, x9

	dc igdvac, x10
	dc igdsw, x11
	dc cgdsw, x12
	dc cigdsw, x13
	dc cgdvac, x14
	dc cgdvap, x15
	dc cgdvadp, x16
	dc cigdvac, x17

	dc gzva, x18
