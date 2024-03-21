# Test HPC/VIS3 instructions
	.text
	nop 
	nop 
	ldx	[%g3], %efsr
	nop
1:	nop
	fnadds	%f1, %f2, %f3
	fnaddd	%f2, %f4, %f6
	fnmuls	%f3, %f5, %f7
	fnmuld	%f6, %f8, %f10
	fhadds	%f7, %f9, %f11
	fhaddd	%f8, %f10, %f12
	fhsubs	%f9, %f11, %f13
	fhsubd	%f10, %f12, %f14
	fnhadds	%f11, %f13, %f15
	fnhaddd	%f12, %f14, %f16
	fnsmuld	%f13, %f15, %f16
	fmadds	%f15, %f17, %f19, %f21
	fmaddd	%f14, %f16, %f18, %f20
	fmsubs	%f17, %f19, %f21, %f23
	fmsubd	%f16, %f18, %f20, %f22
	fnmsubs	%f19, %f21, %f23, %f25
	fnmsubd	%f18, %f20, %f22, %f24
	fnmadds	%f21, %f23, %f25, %f27
	fnmaddd	%f20, %f22, %f24, %f26
	fumadds	%f23, %f25, %f27, %f29
	fumaddd	%f22, %f24, %f26, %f28
	fumsubs	%f25, %f27, %f29, %f31
	fumsubd	%f24, %f26, %f28, %f30
	fnumsubs %f1, %f3, %f5, %f7
	fnumsubd %f2, %f4, %f6, %f8
	fnumadds %f3, %f5, %f7, %f9
	fnumaddd %f4, %f6, %f8, %f10
	addxc	%g5, %g6, %g7
	addxccc	%o1, %o2, %o3
	nop
	umulxhi	%o5, %o6, %o7
	lzcnt	%i1, %i2
	cmask8	%i3
	cmask16	%i4
	cmask32	%i5
	fsll16	%f32, %f34, %f36
	fsrl16	%f34, %f36, %f38
	fsll32	%f36, %f38, %f40
	fsrl32	%f38, %f40, %f42
	fslas16	%f40, %f42, %f44
	fsra16	%f42, %f44, %f46
	fslas32	%f44, %f46, %f48
	fsra32	%f46, %f48, %f50
	pdistn	%f48, %f50, %g1
	fmean16	%f50, %f52, %f54
	fpadd64	%f52, %f54, %f56
	fchksm16 %f54, %f56, %f58
	fpsub64	%f56, %f58, %f60
	fpadds16 %f58, %f60, %f62
	fpadds16s %f2, %f4, %f6
	fpadds32 %f4, %f6, %f8
	fpadds32s %f6, %f8, %f10
	fpsubs16 %f8, %f10, %f12
	fpsubs16s %f10, %f12, %f14
	fpsubs32 %f12, %f14, %f16
	fpsubs32s %f14, %f16, %f18
	movdtox	%f20, %g1
	movstouw %f21, %g2
	movstosw %f23, %g3
	movxtod %g4, %f22
	movwtos %g5, %f23
	xmulx	%o1, %o2, %o3
	xmulxhi	%o4, %o5, %o6
	fucmple8 %f16, %f18, %g1
	fucmpne8 %f18, %f20, %g2
	fucmpgt8 %f20, %f22, %g3
	fucmpeq8 %f22, %f24, %g4
	flcmps	%fcc0, %f1, %f3
	flcmps	%fcc1, %f3, %f5
	flcmps	%fcc2, %f5, %f7
	flcmps	%fcc3, %f7, %f9
	flcmpd	%fcc0, %f12, %f14
	flcmpd	%fcc1, %f14, %f16
	flcmpd	%fcc2, %f16, %f18
	flcmpd	%fcc3, %f18, %f20
	lzd	%i1, %i2
