# Check 64bit F16C new instructions.

	.text
foo:
	vcvtph2ps %xmm4,%ymm4
	vcvtph2ps (%r8),%ymm8
	vcvtph2ps %xmm4,%xmm6
	vcvtph2ps (%rcx),%xmm4
	vcvtps2ph $0x2,%ymm4,%xmm4
	vcvtps2ph $0x2,%ymm8,(%r8)
	vcvtps2ph $0x2,%xmm4,%xmm4
	vcvtps2ph $0x2,%xmm4,(%rcx)

	.intel_syntax noprefix
	vcvtph2ps ymm4,xmm4
	vcvtph2ps ymm8,XMMWORD PTR [r8]
	vcvtph2ps ymm4,[rcx]
	vcvtph2ps xmm6,xmm4
	vcvtph2ps xmm4,QWORD PTR [rcx]
	vcvtph2ps xmm4,[rcx]
	vcvtps2ph xmm4,ymm4,0x2
	vcvtps2ph XMMWORD PTR [rcx],ymm4,0x2
	vcvtps2ph [rcx],ymm4,0x2
	vcvtps2ph xmm4,xmm4,0x2
	vcvtps2ph QWORD PTR [r8],xmm8,0x2
	vcvtps2ph [rcx],xmm4,0x2
