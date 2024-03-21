# Check AVX512VL instructions with ambiguous operands

	.text
	.intel_syntax noprefix
_start:
	vcvtneps2bf16 xmm0, [ecx]
	vcvtpd2dq xmm0{k1}, [ecx]
	vcvtpd2ps xmm0{k1}, [ecx]
	vcvtpd2udq xmm0, [ecx]
	vcvttpd2dq xmm0{k1}, [ecx]
	vcvttpd2udq xmm0, [ecx]
