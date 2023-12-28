# Check illegal AVX instructions
	.text
_start:
	vcvtpd2dq (%ecx),%xmm2
	vcvtpd2ps (%ecx),%xmm2
	vcvttpd2dq (%ecx),%xmm2

	vmovss %xmm0, (%esp), %xmm2
	vmovss %xmm0, $4, %xmm2
	vmovss %xmm0, %cr0, %xmm2
	vmovss %xmm0, %ymm4, %xmm2
	vmovss %xmm0, %mm4, %xmm2

	.intel_syntax noprefix
	vcvtpd2dq xmm2,[ecx]
	vcvtpd2ps xmm2,[ecx]
	vcvttpd2dq xmm2,[ecx]
