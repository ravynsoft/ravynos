#source: sysreg-4.s
#as: -march=armv8.5-a+rng+memtag
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d50b7381 	cfp	rctx, x1
.*:	d50b73a2 	dvp	rctx, x2
.*:	d50b73e3 	cpp	rctx, x3
.*:	d50b7d24 	dc	cvadp, x4
.*:	d53b2405 	mrs	x5, rndr
.*:	d53b2426 	mrs	x6, rndrrs
.*:	d53bd0e7 	mrs	x7, scxtnum_el0
.*:	d538d0e7 	mrs	x7, scxtnum_el1
.*:	d53cd0e7 	mrs	x7, scxtnum_el2
.*:	d53ed0e7 	mrs	x7, scxtnum_el3
.*:	d53dd0e7 	mrs	x7, scxtnum_el12
.*:	d5380388 	mrs	x8, id_pfr2_el1
.*:	d53b42e1 	mrs	x1, tco
.*:	d53b42e2 	mrs	x2, tco
.*:	d5385621 	mrs	x1, tfsre0_el1
.*:	d5385601 	mrs	x1, tfsr_el1
.*:	d53c5602 	mrs	x2, tfsr_el2
.*:	d53e5603 	mrs	x3, tfsr_el3
.*:	d53d560c 	mrs	x12, tfsr_el12
.*:	d53810a1 	mrs	x1, rgsr_el1
.*:	d53810c3 	mrs	x3, gcr_el1
.*:	d5390084 	mrs	x4, gmid_el1
.*:	d51b42e1 	msr	tco, x1
.*:	d51b42e2 	msr	tco, x2
.*:	d5185621 	msr	tfsre0_el1, x1
.*:	d5185601 	msr	tfsr_el1, x1
.*:	d51c5602 	msr	tfsr_el2, x2
.*:	d51e5603 	msr	tfsr_el3, x3
.*:	d51d560c 	msr	tfsr_el12, x12
.*:	d51810a1 	msr	rgsr_el1, x1
.*:	d51810c3 	msr	gcr_el1, x3
.*:	d503419f 	msr	tco, #0x1
.*:	d5087661 	dc	igvac, x1
.*:	d5087682 	dc	igsw, x2
.*:	d5087a83 	dc	cgsw, x3
.*:	d5087e84 	dc	cigsw, x4
.*:	d50b7a65 	dc	cgvac, x5
.*:	d50b7c66 	dc	cgvap, x6
.*:	d50b7d67 	dc	cgvadp, x7
.*:	d50b7e68 	dc	cigvac, x8
.*:	d50b7469 	dc	gva, x9
.*:	d50876aa 	dc	igdvac, x10
.*:	d50876cb 	dc	igdsw, x11
.*:	d5087acc 	dc	cgdsw, x12
.*:	d5087ecd 	dc	cigdsw, x13
.*:	d50b7aae 	dc	cgdvac, x14
.*:	d50b7caf 	dc	cgdvap, x15
.*:	d50b7db0 	dc	cgdvadp, x16
.*:	d50b7eb1 	dc	cigdvac, x17
.*:	d50b7492 	dc	gzva, x18
