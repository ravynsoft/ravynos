# Check illegal 64bit AVX instructions
	.text
_start:
	vcvtpd2dq (%rcx),%xmm2
	vcvtpd2ps (%rcx),%xmm2
	vcvttpd2dq (%rcx),%xmm2

	.intel_syntax noprefix
	vcvtpd2dq xmm2,[rcx]
	vcvtpd2ps xmm2,[rcx]
	vcvttpd2dq xmm2,[rcx]
