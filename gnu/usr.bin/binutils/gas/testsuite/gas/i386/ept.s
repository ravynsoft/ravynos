# Check EPT instructions
	.text
_start:
	.rept 2

	invept	(%ecx), %ebx
	invvpid	(%ecx), %ebx

	.intel_syntax noprefix
	invept ebx, oword ptr [ecx]
	invvpid ebx, oword ptr [ecx]

	.att_syntax prefix
	.code16
	.endr
