	.text
	# Check that a nop instruction is inserted to prevent a floating
	# point compare directly followed by a floating point branch.
no_fpop2_before_fcmp:
	fadds	%f0, %f0, %f0
	fbe	1f
	 nop
	fcmps	%f0, %f0
	fbe	1f
	 nop
1:	nop
