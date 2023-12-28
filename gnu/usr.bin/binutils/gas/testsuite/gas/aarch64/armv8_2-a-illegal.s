
	/* MSR UAO, #imm4.  */
	.irp N,0, 1,2,3,4,5,8,15,19,31
	msr uao, #\N
	.endr
