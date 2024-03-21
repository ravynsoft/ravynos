	.macro	roreg, name
	msr	\name, x1
	.endm

	.macro	woreg, name
	mrs	x0, \name
	.endm

	roreg	id_dfr1_el1
	roreg	id_mmfr5_el1
	roreg	id_isar6_el1

	roreg	icc_iar0_el1
	woreg	icc_eoir0_el1
	roreg	icc_hppir0_el1
	woreg	icc_dir_el1
	roreg	icc_rpr_el1
	woreg	icc_sgi1r_el1
	woreg	icc_asgi1r_el1
	woreg	icc_sgi0r_el1
	roreg	icc_iar1_el1
	woreg	icc_eoir1_el1
	roreg	icc_hppir1_el1
	roreg	ich_misr_el2
	roreg	ich_eisr_el2
	roreg	ich_elrsr_el2

	.arch	armv8.1-a

	roreg	lorid_el1

	.arch	armv8.3-a

	roreg	ccsidr2_el1

	.arch	armv8.4-a

	roreg	pmmir_el1

	roreg	amcfgr_el0
	roreg	amcgcr_el0
	roreg	amevtyper00_el0
	roreg	amevtyper01_el0
	roreg	amevtyper02_el0
	roreg	amevtyper03_el0

	.arch	armv8.6-a

	roreg	amcg1idr_el0
	roreg	cntpctss_el0
	roreg	cntvctss_el0
