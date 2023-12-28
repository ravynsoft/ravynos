	.arch	armv8.8-a

	msr	allint, #-1
	msr	allint, #2
	msr	allint, #15
	msr	allint, #0x100000000

	msr	icc_nmiar1_el1, x0
