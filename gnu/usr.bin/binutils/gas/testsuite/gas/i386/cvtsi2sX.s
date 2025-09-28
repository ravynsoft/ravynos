	.text
cvtsi2sX:
	.intel_syntax noprefix
	cvtsi2sd	xmm0, [rax]
	cvtsi2ss	xmm0, [rax]

	vcvtsi2sd	xmm0, xmm0, [rax]
	vcvtsi2ss	xmm0, xmm0, [rax]

	vcvtsi2sd	xmm31, xmm31, [rax]
	vcvtsi2ss	xmm31, xmm31, [rax]

	vcvtusi2sd	xmm0, xmm0, [rax]
	vcvtusi2ss	xmm0, xmm0, [rax]
