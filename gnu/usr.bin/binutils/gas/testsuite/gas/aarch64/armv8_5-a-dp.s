	.macro expand, op, vec
	.irp	sz, 32, 64
	.irp	rnd, z, x
	.ifc \vec, 0
		\op\sz\rnd	s1, s2
		\op\sz\rnd	d2, d3
	.else
		\op\sz\rnd	v0.2d, v1.2d
		\op\sz\rnd	v0.2s, v1.2s
		\op\sz\rnd	v0.4s, v1.4s
	.endif
	.endr
	.endr
	.endm
func:
	xaflag
	axflag
	expand frint,0
	expand frint,1
