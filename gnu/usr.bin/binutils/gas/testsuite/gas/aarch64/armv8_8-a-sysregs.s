	msr	allint, x0
	MSR	ALLINT, X15
	msr	allint, x30
	msr	allint, xzr
	mrs	x0, allint
	mrs	X16, ALLINT
	mrs	x30, allint
	msr	allint, #0
	msr	allint, #1
	.inst	0xd501421f

	mrs	x0, icc_nmiar1_el1
