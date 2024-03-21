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

	roreg	id_dfr1_el1
	roreg	id_mmfr5_el1
	roreg	id_isar6_el1

	rwreg	icc_pmr_el1
	roreg	icc_iar0_el1
	woreg	icc_eoir0_el1
	roreg	icc_hppir0_el1
	rwreg	icc_bpr0_el1
	rwreg	icc_ap0r0_el1
	rwreg	icc_ap0r1_el1
	rwreg	icc_ap0r2_el1
	rwreg	icc_ap0r3_el1
	rwreg	icc_ap1r0_el1
	rwreg	icc_ap1r1_el1
	rwreg	icc_ap1r2_el1
	rwreg	icc_ap1r3_el1
	woreg	icc_dir_el1
	roreg	icc_rpr_el1
	woreg	icc_sgi1r_el1
	woreg	icc_asgi1r_el1
	woreg	icc_sgi0r_el1
	roreg	icc_iar1_el1
	woreg	icc_eoir1_el1
	roreg	icc_hppir1_el1
	rwreg	icc_bpr1_el1
	rwreg	icc_ctlr_el1
	rwreg	icc_igrpen0_el1
	rwreg	icc_igrpen1_el1
	rwreg	ich_ap0r0_el2
	rwreg	ich_ap0r1_el2
	rwreg	ich_ap0r2_el2
	rwreg	ich_ap0r3_el2
	rwreg	ich_ap1r0_el2
	rwreg	ich_ap1r1_el2
	rwreg	ich_ap1r2_el2
	rwreg	ich_ap1r3_el2
	rwreg	ich_hcr_el2
	roreg	ich_misr_el2
	roreg	ich_eisr_el2
	roreg	ich_elrsr_el2
	rwreg	ich_vmcr_el2
	rwreg	ich_lr0_el2
	rwreg	ich_lr1_el2
	rwreg	ich_lr2_el2
	rwreg	ich_lr3_el2
	rwreg	ich_lr4_el2
	rwreg	ich_lr5_el2
	rwreg	ich_lr6_el2
	rwreg	ich_lr7_el2
	rwreg	ich_lr8_el2
	rwreg	ich_lr9_el2
	rwreg	ich_lr10_el2
	rwreg	ich_lr11_el2
	rwreg	ich_lr12_el2
	rwreg	ich_lr13_el2
	rwreg	ich_lr14_el2
	rwreg	ich_lr15_el2
	rwreg	icc_igrpen1_el3

	.arch	armv8.1-a

	roreg	lorid_el1

	.arch	armv8.3-a

	roreg	ccsidr2_el1

	.arch	armv8.4-a

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

	.arch	armv8.6-a

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

	.arch	armv8.7-a

	rwreg	pmsnevfr_el1
	rwreg	hcrx_el2
