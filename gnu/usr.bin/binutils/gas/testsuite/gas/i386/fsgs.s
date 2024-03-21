# Check FSGSBase new instructions.

	.text
foo:
	.rept 2
	rdfsbase %ebx
	rdgsbase %ebx
	wrfsbase %ebx
	wrgsbase %ebx

	.intel_syntax noprefix
	rdfsbase ebx
	rdgsbase ebx
	wrfsbase ebx
	wrgsbase ebx

	.att_syntax prefix
	.code16
	.endr
