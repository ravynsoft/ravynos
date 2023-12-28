# Check 32bit PTWRITE instructions

	.text
_start:
	.rept 2
	ptwrite %ecx
	ptwritel %ecx
	ptwrite (%ecx)
	ptwritel (%ecx)

	.intel_syntax noprefix
	ptwrite ecx
	ptwrite [ecx]
	ptwrite DWORD PTR [ecx]

	.att_syntax prefix
	.code16
	.endr
