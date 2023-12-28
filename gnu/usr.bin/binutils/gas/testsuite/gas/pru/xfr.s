# Source file used to test the XFR-class of instructions.

foo:
	# register clear and fill
	zero	r2, 1
	zero	r23.b2, 4
	zero	r0, 124
	fill	r2, 1
	fill	r23.b1, 4
	fill	r0, 124

	# XIN
	xin	0, r10, 1
	xin	0, r10.b1, 124
	xin	253, r10.b3, 1
	xin	253, r10.b2, 124
	xin	85, r12.b0, 85

	# XOUT
	xout	0, r10, 1
	xout	0, r10.b1, 124
	xout	253, r10.b3, 1
	xout	253, r10.b2, 124
	xout	85, r12.b0, 85

	# XCHG
	xchg	0, r10, 1
	xchg	0, r10.b1, 124
	xchg	253, r10.b3, 1
	xchg	253, r10.b2, 124
	xchg	85, r12.b0, 85

	# SXIN
	sxin	0, r10, 1
	sxin	0, r10.b1, 124
	sxin	253, r10.b3, 1
	sxin	253, r10.b2, 124
	sxin	85, r12.b0, 85

	# SXOUT
	sxout	0, r10, 1
	sxout	0, r10.b1, 124
	sxout	253, r10.b3, 1
	sxout	253, r10.b2, 124
	sxout	85, r12.b0, 85

	# XCHG
	sxchg	0, r10, 1
	sxchg	0, r10.b1, 124
	sxchg	253, r10.b3, 1
	sxchg	253, r10.b2, 124
	sxchg	85, r12.b0, 85
