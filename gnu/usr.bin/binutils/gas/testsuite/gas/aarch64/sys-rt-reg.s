// sys-rt-reg.s Test file for AArch64 instructions take SYS_Rt register.

	.text

	.irp	ic_op, ialluis, iallu
	ic	\ic_op
	.endr

	.irp	rt, x0, x3, x15, x26, xzr
	ic	ivau, \rt
	.endr

	.irp	tlbi_op, vmalle1, vmalle1is, vmalls12e1, vmalls12e1is, alle2, alle2is, alle1, alle1is, alle3, alle3is
	tlbi	\tlbi_op
	.endr

	.irp	tlbi_op, vae1, aside1, vaae1, vae1is, aside1is, vaae1is, ipas2e1is, ipas2le1is, ipas2e1, ipas2le1, vae2, vae2is, vae3, vae3is, vale1is, vale2is, vale3is, vaale1is, vale1, vale2, vale3, vaale1
	.irp    rt, x0, x3, x15, x26, xzr
	tlbi	\tlbi_op, \rt
	.endr
	.endr
