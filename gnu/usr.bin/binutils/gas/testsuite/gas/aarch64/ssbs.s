/* Test SSBS.  */
func:
	.ifdef SUCCESS
	msr ssbs, #1
	msr ssbs, #0

	.irp N,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	msr ssbs, x\N
	mrs x\N, ssbs
	.endr

	.irp N,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
	msr ssbs, x\N
	mrs x\N, ssbs
	.endr
	.endif

	/* Constant >1 Failure.  */
	.ifdef ERROR1
	.irp N,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	msr ssbs, #\N
	.endr
	.endif

	/* No +ssbs Failure.  */
	.ifdef ERROR2
	msr ssbs, #0
	msr ssbs, #1
	msr ssbs, x6
	mrs x25, ssbs
	.endif
