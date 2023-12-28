	.text
foo:
	cvtsi2sd (%rax), %xmm0
	cvtsi2ss (%rax), %xmm0
	vcvtsi2sd (%rax), %xmm0, %xmm0
	vcvtsi2sd (%rax), %xmm0, %xmm31
	vcvtsi2ss (%rax), %xmm0, %xmm0
	vcvtsi2ss (%rax), %xmm0, %xmm31
	vcvtusi2sd (%rax), %xmm0, %xmm0
	vcvtusi2ss (%rax), %xmm0, %xmm0
	vcvtsi2sh (%rax), %xmm0, %xmm0
	vcvtsi2sh (%rax), %xmm0, %xmm31
	vcvtusi2sh (%rax), %xmm0, %xmm0
