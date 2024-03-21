	.text
prefix:
	addr32 loop	.Lrep_ret
	addr32 jrcxz	.Ldata16

	repe addss	%xmm0, %xmm0
	repne addss	%xmm0, %xmm0
	repe vaddss	%xmm0, %xmm0, %xmm0
	repne vaddss	%xmm0, %xmm0, %xmm0

.Lrep_ret:
	bnd ret
	rep ret
	bnd rep ret
	rep bnd ret

.Ldata16:
	data16 addps	%xmm0, %xmm0
	data16 addpd	%xmm0, %xmm0
	data16 vaddps	%xmm0, %xmm0, %xmm0
	data16 vaddpd	%xmm0, %xmm0, %xmm0

	.p2align	4,0
