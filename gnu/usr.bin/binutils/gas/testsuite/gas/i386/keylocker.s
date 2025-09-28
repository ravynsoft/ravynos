# Check 32bit KEYLOCKER instructions

	.text
_start:
	loadiwkey %xmm2, %xmm6
	encodekey128 %eax, %edx
	encodekey256 %eax, %edx

	aesenc128kl 126(%edx), %xmm2
	aesenc256kl 126(%edx), %xmm2
	aesdec128kl 126(%edx), %xmm2
	aesdec256kl 126(%edx), %xmm2

	aesencwide128kl	126(%edx)
	aesencwide256kl	126(%edx)
	aesdecwide128kl	126(%edx)
	aesdecwide256kl	126(%edx)

	.intel_syntax noprefix

	loadiwkey xmm6, xmm2
	encodekey128 edx, eax
	encodekey256 edx, eax

	aesenc128kl xmm2, [edx+126]
	aesenc256kl xmm2, [edx+126]
	aesdec128kl xmm2, [edx+126]
	aesdec256kl xmm2, [edx+126]

	aesencwide128kl	[edx+126]
	aesencwide256kl	[edx+126]
	aesdecwide128kl	[edx+126]
	aesdecwide256kl	[edx+126]
