# Check 64bit AVX512VBMI2 instructions

	.text
_start:
	loadiwkey %xmm2, %xmm6
	encodekey128 %eax, %edx
	encodekey256 %eax, %edx

	aesenc128kl 126(%rdx), %xmm2
	aesenc256kl 126(%rdx), %xmm2
	aesdec128kl 126(%rdx), %xmm2
	aesdec256kl 126(%rdx), %xmm2

	aesencwide128kl	126(%rdx)
	aesencwide256kl	126(%rdx)
	aesdecwide128kl	126(%rdx)
	aesdecwide256kl	126(%rdx)

	.intel_syntax noprefix

	loadiwkey xmm6, xmm2
	encodekey128 edx, eax
	encodekey256 edx, eax

	aesenc128kl xmm2, [rdx+126]
	aesenc256kl xmm2, [rdx+126]
	aesdec128kl xmm2, [rdx+126]
	aesdec256kl xmm2, [rdx+126]

	aesencwide128kl	[rdx+126]
	aesencwide256kl	[rdx+126]
	aesdecwide128kl	[rdx+126]
	aesdecwide256kl	[rdx+126]
