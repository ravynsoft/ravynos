	vmsge.vx v4, v8, a1		# unmasked va >= x
	vmsge.vx v8, v12, a2, v0.t	# masked va >= x, vd != v0
	vmsge.vx v0, v8, a1, v0.t, v12	# masked va >= x, vd == v0
	vmsge.vx v4, v8, a1, v0.t, v12	# masked va >= x, any vd

	vmsgeu.vx v4, v8, a1		# unmasked va >= x
	vmsgeu.vx v8, v12, a2, v0.t	# masked va >= x, vd != v0
	vmsgeu.vx v0, v8, a1, v0.t, v12	# masked va >= x, vd == v0
	vmsgeu.vx v4, v8, a1, v0.t, v12	# masked va >= x, any vd
