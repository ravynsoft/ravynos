	.macro	roreg, name
	mrs	x0, \name
	.endm

	.macro	woreg, name
	msr	\name, x1
	.endm

	.macro	rwreg, name
	mrs	x2, \name
	msr	\name, x3
	.endm

	roreg	lorid_el1

	.arch	armv8.2-a

	roreg	ccsidr2_el1

	.arch	armv8.3-a

	rwreg	trfcr_el1
	roreg	pmmir_el1
	rwreg	trfcr_el2

	rwreg	trfcr_el12

	rwreg	amcr_el0
	roreg	amcfgr_el0
	roreg	amcgcr_el0
	rwreg	amuserenr_el0
	rwreg	amcntenclr0_el0
	rwreg	amcntenset0_el0
	rwreg	amcntenclr1_el0
	rwreg	amcntenset1_el0
	rwreg	amevcntr00_el0
	rwreg	amevcntr01_el0
	rwreg	amevcntr02_el0
	rwreg	amevcntr03_el0
	roreg	amevtyper00_el0
	roreg	amevtyper01_el0
	roreg	amevtyper02_el0
	roreg	amevtyper03_el0
	rwreg	amevcntr10_el0
	rwreg	amevcntr11_el0
	rwreg	amevcntr12_el0
	rwreg	amevcntr13_el0
	rwreg	amevcntr14_el0
	rwreg	amevcntr15_el0
	rwreg	amevcntr16_el0
	rwreg	amevcntr17_el0
	rwreg	amevcntr18_el0
	rwreg	amevcntr19_el0
	rwreg	amevcntr110_el0
	rwreg	amevcntr111_el0
	rwreg	amevcntr112_el0
	rwreg	amevcntr113_el0
	rwreg	amevcntr114_el0
	rwreg	amevcntr115_el0
	rwreg	amevtyper10_el0
	rwreg	amevtyper11_el0
	rwreg	amevtyper12_el0
	rwreg	amevtyper13_el0
	rwreg	amevtyper14_el0
	rwreg	amevtyper15_el0
	rwreg	amevtyper16_el0
	rwreg	amevtyper17_el0
	rwreg	amevtyper18_el0
	rwreg	amevtyper19_el0
	rwreg	amevtyper110_el0
	rwreg	amevtyper111_el0
	rwreg	amevtyper112_el0
	rwreg	amevtyper113_el0
	rwreg	amevtyper114_el0
	rwreg	amevtyper115_el0

	.arch	armv8.5-a

	roreg	amcg1idr_el0
	roreg	cntpctss_el0
	roreg	cntvctss_el0
	rwreg	hfgrtr_el2
	rwreg	hfgwtr_el2
	rwreg	hfgitr_el2
	rwreg	hdfgrtr_el2
	rwreg	hdfgwtr_el2
	rwreg	hafgrtr_el2
	rwreg	amevcntvoff00_el2
	rwreg	amevcntvoff01_el2
	rwreg	amevcntvoff02_el2
	rwreg	amevcntvoff03_el2
	rwreg	amevcntvoff04_el2
	rwreg	amevcntvoff05_el2
	rwreg	amevcntvoff06_el2
	rwreg	amevcntvoff07_el2
	rwreg	amevcntvoff08_el2
	rwreg	amevcntvoff09_el2
	rwreg	amevcntvoff010_el2
	rwreg	amevcntvoff011_el2
	rwreg	amevcntvoff012_el2
	rwreg	amevcntvoff013_el2
	rwreg	amevcntvoff014_el2
	rwreg	amevcntvoff015_el2
	rwreg	amevcntvoff10_el2
	rwreg	amevcntvoff11_el2
	rwreg	amevcntvoff12_el2
	rwreg	amevcntvoff13_el2
	rwreg	amevcntvoff14_el2
	rwreg	amevcntvoff15_el2
	rwreg	amevcntvoff16_el2
	rwreg	amevcntvoff17_el2
	rwreg	amevcntvoff18_el2
	rwreg	amevcntvoff19_el2
	rwreg	amevcntvoff110_el2
	rwreg	amevcntvoff111_el2
	rwreg	amevcntvoff112_el2
	rwreg	amevcntvoff113_el2
	rwreg	amevcntvoff114_el2
	rwreg	amevcntvoff115_el2
	rwreg	cntpoff_el2

	.arch	armv8.6-a

	rwreg	pmsnevfr_el1
	rwreg	hcrx_el2
