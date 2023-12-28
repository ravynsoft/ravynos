# Check F16C new instructions.

	.text
foo:
	vcvtph2ps %xmm4,%ymm4
	vcvtph2ps (%ecx),%ymm4
	vcvtph2ps %xmm4,%xmm6
	vcvtph2ps (%ecx),%xmm4
	vcvtps2ph $0x2,%ymm4,%xmm4
	vcvtps2ph $0x2,%ymm4,(%ecx)
	vcvtps2ph $0x2,%xmm4,%xmm4
	vcvtps2ph $0x2,%xmm4,(%ecx)

	.intel_syntax noprefix
	vcvtph2ps ymm4,xmm4
	vcvtph2ps ymm4,XMMWORD PTR [ecx]
	vcvtph2ps ymm4,[ecx]
	vcvtph2ps xmm6,xmm4
	vcvtph2ps xmm4,QWORD PTR [ecx]
	vcvtph2ps xmm4,[ecx]
	vcvtps2ph xmm4,ymm4,0x2
	vcvtps2ph XMMWORD PTR [ecx],ymm4,0x2
	vcvtps2ph [ecx],ymm4,0x2
	vcvtps2ph xmm4,xmm4,0x2
	vcvtps2ph QWORD PTR [ecx],xmm4,0x2
	vcvtps2ph [ecx],xmm4,0x2
